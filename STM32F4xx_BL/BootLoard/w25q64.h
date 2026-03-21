#ifndef __W25Q64_H__
#define __W25Q64_H__
#include "main.h"
#include <stdio.h>
#include <string.h>

#define W25Q64_WRITE_ENABLE							0x06
#define W25Q64_WRITE_DISABLE						0x04
#define W25Q64_READ_STATUS_REGISTER_1				0x05
#define W25Q64_READ_STATUS_REGISTER_2				0x35
#define W25Q64_WRITE_STATUS_REGISTER				0x01
#define W25Q64_PAGE_PROGRAM							0x02
#define W25Q64_QUAD_PAGE_PROGRAM					0x32
#define W25Q64_BLOCK_ERASE_64KB						0xD8
#define W25Q64_BLOCK_ERASE_32KB						0x52
#define W25Q64_SECTOR_ERASE_4KB						0x20
#define W25Q64_CHIP_ERASE							0xC7
#define W25Q64_ERASE_SUSPEND						0x75
#define W25Q64_ERASE_RESUME							0x7A
#define W25Q64_POWER_DOWN							0xB9
#define W25Q64_HIGH_PERFORMANCE_MODE				0xA3
#define W25Q64_CONTINUOUS_READ_MODE_RESET			0xFF
#define W25Q64_RELEASE_POWER_DOWN_HPM_DEVICE_ID		0xAB
#define W25Q64_MANUFACTURER_DEVICE_ID				0x90
#define W25Q64_READ_UNIQUE_ID						0x4B
#define W25Q64_JEDEC_ID								0x9F
#define W25Q64_READ_DATA							0x03
#define W25Q64_FAST_READ							0x0B
#define W25Q64_FAST_READ_DUAL_OUTPUT				0x3B
#define W25Q64_FAST_READ_DUAL_IO					0xBB
#define W25Q64_FAST_READ_QUAD_OUTPUT				0x6B
#define W25Q64_FAST_READ_QUAD_IO					0xEB
#define W25Q64_OCTAL_WORD_READ_QUAD_IO				0xE3

#define W25Q64_DUMMY_BYTE							0xFF



void W25Q64_ReadID(uint8_t *MID, uint16_t *DID);
void W25Q64_PageProgram(uint32_t addr, uint8_t *buf, uint16_t len);
void W25Q64_SectorErase(uint32_t addr);
void W25Q64_BlockErase(uint32_t addr);
void W25Q64_ChipErase_NoCheck(void);
void W25Q64_WriteData(uint32_t addr,uint8_t *buf,uint32_t len);
void W25Q64_ReadData(uint32_t addr, uint8_t *buf, uint32_t len);





#define FLAG_SECTOR_ADDR    0x7FF000        
#define FLAG_SECTOR_SIZE    ( 4 * 1024 )                            // 4KB
#define FLAG_SLOT_SIZE          32
#define FLAG_SLOT_COUNT     (FLAG_SECTOR_SIZE / FLAG_SLOT_SIZE)     // 128
#define EMPTY_FLAG          (0xFFFFFFFF)
#define MAGIC_NUM           (0xa5a5a5a5)

// 槽位地址计算
#define FLAG_SLOT_ADDR(num)      (FLAG_SECTOR_ADDR + (num) * FLAG_SLOT_SIZE)


typedef struct{
    uint32_t magic;               

    uint8_t  upgrade_state;        
    uint8_t  reserved[3];  

    uint32_t download_addr;         // 下载区地址
    uint32_t download_crc;             // 固件CRC

    uint32_t copy_addr;             // 拷贝区地址
    uint32_t copy_crc;               // 拷贝区CRC

    uint16_t downloard_size_kb;
    uint16_t copy_size_kb;

    uint32_t crc;                   // 结构CRC（不含magic）
}UpgradeSlot_t;

typedef enum {
    UPGRADE_STATE_NONE = 0,           // 无升级
    UPGRADE_STATE_AVAILABLE,          // 有升级可用（待升级）
    UPGRADE_STATE_DOWNLOADING,        // 正在下载升级包
    UPGRADE_STATE_DOWNLOADED,         // 下载完成，待安装
    UPGRADE_STATE_INSTALLING,         // 正在安装/升级中
    UPGRADE_STATE_SUCCESS,            // 升级成功
    UPGRADE_STATE_FAILED,             // 升级失败
    UPGRADE_STATE_ROLLBACK,           // 正在回滚
    UPGRADE_STATE_PAUSED,             // 升级暂停
    UPGRADE_STATE_CANCELLED,          // 升级取消
    UPGRADE_STATE_CHECKING            // 正在检查更新
} upgrade_state_t;


void W25Q64_EraseSlotRegion(void);
void W25Q64_WriteUpgradeSlot(UpgradeSlot_t *slot);
uint8_t W25Q64_ReadLatestUpgradeSlot(UpgradeSlot_t *slot);

void W25Q64_EraseBackupRegion(void);
void W25Q64_EraseDownloadRegion(void);


HAL_StatusTypeDef W25Q64_WriteFlash(uint8_t *buf);
#endif
