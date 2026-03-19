#include "w25q64.h"

/*
┌───────────────────────────────────────────────────┐ 0x7FFFFF (8MB 末尾)
│                                                   │
│             正常数据区 (APP 读写)                  │
│             大小：~4MB（4092KB）                  │
│           起始：0x401000  结束：0x7FFFFF           │
│                                                   │
├───────────────────────────────────────────────────┤ 0x400FFF
│                                                   │
│                  备份区 (BOOT)                    │
│             大小：2MB (2048KB)                    │
│           起始：0x201000  结束：0x400FFF           │
│                                                   │
├───────────────────────────────────────────────────┤ 0x200FFF
│                                                   │
│                  下载区 (BOOT)                    │
│             大小：2MB (2048KB)                    │
│           起始：0x001000  结束：0x200FFF           │
│                                                   │
├───────────────────────────────────────────────────┤ 0x000FFF
│                                                   │
│                标志位区 (BOOT)                    │
│              大小：4KB (0x1000 字节)              │
│           起始：0x000000  结束：0x000FFF           │
│                                                   │
└───────────────────────────────────────────────────┘ 0x000000 (8MB 起始)
*/



/* ----------------------- static ----------------------------------*/

static uint32_t CalcSlotCRC(UpgradeSlot_t *slot);
static uint32_t W25Q64_ReadMagic(uint16_t num);
static uint16_t FindFirstEmptySlot(void);



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
    if (first_empty < SLOT_COUNT) {
        W25Q64_Write( SLOT_ADDR(first_empty), (uint8_t *)slot, sizeof(UpgradeSlot_t) );
    }
    else{
        W25Q64_Read( SLOT_ADDR(SLOT_COUNT - 1), (uint8_t *)&temp, sizeof(UpgradeSlot_t) );
        W25Q64_EraseSlotRegion();
        W25Q64_Write( SLOT_ADDR(1), (uint8_t *)slot, sizeof(UpgradeSlot_t) );
        W25Q64_Write( SLOT_ADDR(0), (uint8_t *)&temp, sizeof(UpgradeSlot_t) );
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
    if (first_empty >= SLOT_COUNT) {
        W25Q64_Read( SLOT_ADDR(SLOT_COUNT - 1), (uint8_t *)slot, sizeof(UpgradeSlot_t) );
    } else {
        W25Q64_Read( SLOT_ADDR(first_empty - 1), (uint8_t *)slot, sizeof(UpgradeSlot_t) );
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
    uint8_t *p = (uint8_t *)&slot->reserved1;
    uint32_t len = sizeof(UpgradeSlot_t) - 8;  // magic(4) + crc(4)
    
    for (uint32_t i = 0; i < len; i++) {
        crc += p[i];
    }
    return crc;
}

static uint32_t W25Q64_ReadMagic(uint16_t num)
{
    uint32_t magic = 0;
    W25Q64_Read( SLOT_ADDR(num), (uint8_t *)&magic, sizeof(uint32_t) );
    return magic;
}

static uint16_t FindFirstEmptySlot(void)
{
    uint16_t left = 0;
    uint16_t right = SLOT_COUNT - 1;
    uint16_t mid;

    if (W25Q64_ReadMagic(0) != 0xa5a5a5a5)
        return 0;

    if(W25Q64_ReadMagic(SLOT_COUNT - 1) == 0xa5a5a5a5)
        return SLOT_COUNT;

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
