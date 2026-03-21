#include "w25q64.h"
#include "flash_if.h"
/*

    W25Q64 分区 (总 8 MB = 0x800000 字节)

    [下载区]          ; 2 MB
    起始: 0x000000
    结束: 0x1FFFFF

    [备份区]          ; 2 MB
    起始: 0x200000
    结束: 0x3FFFFF

    [正常数据区]      ; 4 MB - 4 KB = 4092 KB
    起始: 0x400000
    结束: 0x7FEFFF

    [标志区]          ; 4 KB
    起始: 0x7FF000
    结束: 0x7FFFFF

*/



/* ----------------------- static ----------------------------------*/

static uint32_t CalcSlotCRC(UpgradeSlot_t *slot);
static uint32_t W25Q64_ReadMagic(uint16_t num);
static uint16_t FindFirstEmptySlot(void);

UpgradeSlot_t slot = {
    .magic = MAGIC_NUM,
    .upgrade_state = UPGRADE_STATE_NONE,
    .download_addr = 0x000000,
    .copy_addr = 0x200000,
    .copy_size_kb = 0
};

void W25Q64_EraseSlotRegion(void)
{
   W25Q64_SectorErase(FLAG_SECTOR_ADDR);
}

void W25Q64_WriteUpgradeSlot(UpgradeSlot_t *slot)
{
    uint16_t first_empty;
    UpgradeSlot_t temp;
    slot->magic = MAGIC_NUM;
    slot->crc = CalcSlotCRC(slot);
    first_empty = FindFirstEmptySlot();
    if (first_empty < FLAG_SLOT_COUNT) {
        W25Q64_WriteData( FLAG_SLOT_ADDR(first_empty), (uint8_t *)slot, sizeof(UpgradeSlot_t) );
    }
    else{
        W25Q64_ReadData( FLAG_SLOT_ADDR(FLAG_SLOT_COUNT - 1), (uint8_t *)&temp, sizeof(UpgradeSlot_t) );
        W25Q64_EraseSlotRegion();
        W25Q64_WriteData( FLAG_SLOT_ADDR(1), (uint8_t *)slot, sizeof(UpgradeSlot_t) );
        W25Q64_WriteData( FLAG_SLOT_ADDR(0), (uint8_t *)&temp, sizeof(UpgradeSlot_t) );
    }
    
}

uint8_t W25Q64_ReadLatestUpgradeSlot(UpgradeSlot_t *slot)
{
    uint16_t first_empty;
    
    // 1. 找空槽位置
    first_empty = FindFirstEmptySlot();
    
    // 2. 没有数据
    if (first_empty == 0) {
        return 0;
    }
    
    // 3. 读取最新槽
    if (first_empty >= FLAG_SLOT_COUNT) {
        W25Q64_ReadData( FLAG_SLOT_ADDR(FLAG_SLOT_COUNT - 1), (uint8_t *)slot, sizeof(UpgradeSlot_t) );
    } else {
        W25Q64_ReadData( FLAG_SLOT_ADDR(first_empty - 1), (uint8_t *)slot, sizeof(UpgradeSlot_t) );
    }
    
    // 4. 多重有效性检查
    if (slot->magic != MAGIC_NUM) {
        return 0;  // 魔数不对
    }

    if (slot->crc != CalcSlotCRC(slot)) {
        return 0;  // CRC校验失败
    }
    return 1;  // 成功
}


static uint32_t CalcSlotCRC(UpgradeSlot_t *slot)
{
    uint32_t crc = 0;
    uint8_t *p = (uint8_t *)&slot->upgrade_state;
    uint32_t len = sizeof(UpgradeSlot_t) - 8;  // magic(4) + crc(4)
    
    for (uint32_t i = 0; i < len; i++) {
        crc += p[i];
    }
    return crc;
}

static uint32_t W25Q64_ReadMagic(uint16_t num)
{
    uint32_t magic = 0;
    W25Q64_ReadData( FLAG_SLOT_ADDR(num), (uint8_t *)&magic, sizeof(uint32_t) );
    return magic;
}

static uint16_t FindFirstEmptySlot(void)
{
    uint16_t left = 0;
    uint16_t right = FLAG_SLOT_COUNT - 1;
    uint16_t mid;

    if (W25Q64_ReadMagic(0) != 0xa5a5a5a5)
        return 0;

    if(W25Q64_ReadMagic(FLAG_SLOT_COUNT - 1) == 0xa5a5a5a5)
        return FLAG_SLOT_COUNT;

    while (left <= right)
    {
        mid = (left + right) / 2;

        if (W25Q64_ReadMagic(mid) != 0xa5a5a5a5)
        {
            // 空 → 往左找
            right = mid - 1;
        }
        else
        {
            // 有效 → 往右找
            left = mid + 1;
        }
    }

    return left;  // 第一个空槽
}

/* ----------------------- 备份区和下载区 ----------------------------------*/
void W25Q64_EraseBackupRegion(void)
{
    uint16_t i;
    for(i=0;i<32;i++)
        W25Q64_BlockErase(0x200000 + i*0x10000);
}

void W25Q64_EraseDownloadRegion(void)
{
    uint16_t i;
    for(i=0;i<32;i++)
        W25Q64_BlockErase(0x000000 + i*0x10000);
}

HAL_StatusTypeDef EraseFlashSectors(void)
{
    FLASH_EraseInitTypeDef erase_init;
    uint32_t sector_error = 0;
    
    /* 计算起始扇区号 */
    uint32_t start_sector = 4;
    
    /* 需要擦除的扇区数量 */
    uint32_t sector_count = 4;
    
    /* 配置擦除参数 */
    erase_init.TypeErase   = FLASH_TYPEERASE_SECTORS;  // 扇区擦除
    erase_init.Sector      = start_sector;              // 起始扇区
    erase_init.NbSectors   = sector_count;              // 扇区数量
    erase_init.VoltageRange = FLASH_VOLTAGE_RANGE_3;    // 电压范围（3.3V）
    
    /* 执行擦除 */
    if(HAL_FLASHEx_Erase(&erase_init, &sector_error) != HAL_OK)
    {
        /* 擦除失败，sector_error记录出错的扇区 */
        return HAL_ERROR;
    }
    
    return HAL_OK;
}

HAL_StatusTypeDef WriteDataToFlash(uint32_t flash_addr, uint8_t *buf, uint32_t len)
{
    uint32_t offset = 0;
    uint32_t data_word;
    
    /* 确保地址是字对齐的（4字节对齐） */
    if((flash_addr & 0x03) != 0)
    {
        return HAL_ERROR;  // 地址未对齐
    }
    
    /* 按字（4字节）写入 */
    while(offset < len)
    {
        /* 如果剩余数据不足4字节，按字节写入 */
        if(len - offset < 4)
        {
            /* 处理剩余的1-3字节 */
            for(uint32_t i = 0; i < (len - offset); i++)
            {
                if(HAL_FLASH_Program(FLASH_TYPEPROGRAM_BYTE, 
                                     flash_addr + offset + i, 
                                     buf[offset + i]) != HAL_OK)
                {
                    return HAL_ERROR;
                }
            }
            break;
        }
        
        /* 组装一个字（4字节） */
        data_word = buf[offset] | 
                   (buf[offset + 1] << 8) | 
                   (buf[offset + 2] << 16) | 
                   (buf[offset + 3] << 24);
        
        /* 写入一个字 */
        if(HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, 
                             flash_addr + offset, 
                             data_word) != HAL_OK)
        {
            return HAL_ERROR;
        }
        
        offset += 4;
    }
    
    return HAL_OK;
}

HAL_StatusTypeDef W25Q64_WriteFlash(uint8_t *buf)
{
    uint32_t flash_addr = ADDR_FLASH_SECTOR_4;              // Flash写入起始地址
    uint32_t data_len   = slot.copy_size_kb * 1024;  // 数据长度（字节）
    uint32_t offset     = 0;                     // 当前写入位置偏移
    uint32_t crc = 0;
    
    /* ==================== 第1步：解锁Flash ==================== */
    HAL_FLASH_Unlock();
    
    /* ==================== 第2步：擦除Flash扇区 ==================== */
    if(EraseFlashSectors() != HAL_OK)
    {
        HAL_FLASH_Lock();
        return HAL_ERROR;
    }
    while(offset < data_len)
    {
        W25Q64_ReadData(slot.copy_addr + offset, buf, 1024);
        for(int n = 0; n < 1024; n++)
            crc+=buf[n];
        WriteDataToFlash(flash_addr + offset, (uint8_t *)buf, 1024);
        offset+=1024;
    }
    if(crc != slot.copy_crc)
    {
        while(1);
    }
    /* ==================== 第4步：锁定Flash ==================== */
    HAL_FLASH_Lock();
    
    /* ==================== 第5步：校验数据（可选） ==================== */
    if(crc != slot.copy_crc)
    {
        return HAL_ERROR;
    }
    
    return HAL_OK;
}

