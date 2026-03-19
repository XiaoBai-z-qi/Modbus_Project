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




void W25Q64_Init(void);
void W25Q64_ReadID(uint8_t *MID, uint16_t *DID);
void W25Q64_Read(uint32_t addr, uint8_t *data, uint16_t len);
void W25Q64_Write(uint32_t addr,uint8_t *buf,uint32_t len);
void W25Q64_SectorErase(uint32_t addr);
void W25Q64_ChipErase(void);



#define FLAG_SECTOR_ADDR    0x000000        // 你指定的地址
#define FLAG_SECTOR_SIZE    4 * 1024        // 4KB
#define SLOT_SIZE           32
#define SLOT_COUNT          (FLAG_SECTOR_SIZE / SLOT_SIZE)  // 128
#define EMPTY_FLAG          (0xFFFFFFFF)
#define MAGIC_NUM           (0xa5a5a5a5)

// 槽位地址计算
#define SLOT_ADDR(num)      (FLAG_SECTOR_ADDR + (num) * SLOT_SIZE)

typedef struct{
    uint32_t magic;       
    uint32_t reserved1;          

    uint8_t  upgrade_state;        
    uint8_t  reserved[3];  

    uint32_t image_size;   // 4  固件大小
    uint32_t image_crc;    // 4  固件CRC

    uint32_t reserved2;    // 4  预留（以后扩展用）

    uint32_t crc;          // 4  结构CRC（不含magic）
}UpgradeSlot_t;



void W25Q64_EraseSlotRegion(void);
void W25Q64_WriteUpgradeSlot(UpgradeSlot_t *slot);
uint8_t W25Q64_ReadLatestUpgradeSlot(UpgradeSlot_t *slot);

#endif
