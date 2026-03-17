#include "w25q64.h"
#include <stdio.h>
#include <string.h>

#define W25Q64_BUFF_SIZE 256
static volatile uint8_t dma_busy_flag = 0;
uint8_t W25Q64_Buff_Dummy[W25Q64_BUFF_SIZE];

#define W25Q64_CS_LOW()  HAL_GPIO_WritePin(W25Q64_CS_GPIO_Port, W25Q64_CS_Pin, GPIO_PIN_RESET)
#define W25Q64_CS_HIGH() HAL_GPIO_WritePin(W25Q64_CS_GPIO_Port, W25Q64_CS_Pin, GPIO_PIN_SET)

extern SPI_HandleTypeDef hspi2;

static void W25Q64_WriteEnable(void);
static void W25Q64_PageWrite(uint32_t addr,uint8_t *buf,uint16_t len);
static void W25Q64_WaitBusy(void);
static void W25Q64_SPI_DMA_Transmit(uint8_t *txdata, uint16_t len);
static void W25Q64_SPI_DMA_Receive(uint8_t *rxdata, uint16_t len);
static void W25Q64_SPI_DMA_Transmit_Receive(uint8_t *txdata, uint8_t *rxdata, uint16_t len);

void W25Q64_Init(void)
{
    memset(W25Q64_Buff_Dummy,0xFF,sizeof(W25Q64_Buff_Dummy));
}



void W25Q64_ReadID(uint8_t *MID, uint16_t *DID)
{
    const uint8_t txdata[4] = { W25Q64_JEDEC_ID, 
                                W25Q64_DUMMY_BYTE, 
                                W25Q64_DUMMY_BYTE, 
                                W25Q64_DUMMY_BYTE };
    uint8_t rxdata[4];

    W25Q64_CS_LOW();
    W25Q64_SPI_DMA_Transmit_Receive((uint8_t *)txdata, rxdata, 4);
    W25Q64_CS_HIGH();

    *MID = rxdata[1];
    *DID = (rxdata[2] << 8) | rxdata[3];

}

void W25Q64_Read(uint32_t addr, uint8_t *data, uint16_t len)
{
    uint8_t cmd[4];
    cmd[0] = W25Q64_READ_DATA;
    cmd[1] = (uint8_t)(addr >> 16) & 0xFF;
    cmd[2] = (uint8_t)(addr >> 8) & 0xFF;
    cmd[3] = (uint8_t)(addr & 0xFF);

    W25Q64_CS_LOW();
    W25Q64_SPI_DMA_Transmit(cmd, 4);
    while(len)
    {
        uint16_t chunk = len > W25Q64_BUFF_SIZE? W25Q64_BUFF_SIZE : len;
        W25Q64_SPI_DMA_Transmit_Receive(W25Q64_Buff_Dummy, data, chunk);
        len -= chunk;
        data += chunk;
    }
    W25Q64_CS_HIGH();

}



//×Ô¶¯·ÖÒ³Ð´
void W25Q64_Write(uint32_t addr,uint8_t *buf,uint32_t len)
{
    uint16_t page_remain;

    while(len)
    {
        page_remain = 256 - (addr % 256);

        if(len < page_remain)
            page_remain = len;

        W25Q64_PageWrite(addr,buf,page_remain);

        addr += page_remain;
        buf  += page_remain;
        len  -= page_remain;
    }
}

//ÉÈÇø²Á³ý
void W25Q64_SectorErase(uint32_t addr)
{
    uint8_t cmd[4];

    cmd[0]=W25Q64_SECTOR_ERASE_4KB;
    cmd[1]=addr>>16;
    cmd[2]=addr>>8;
    cmd[3]=addr;

    W25Q64_WriteEnable();
      
    W25Q64_CS_LOW();
    W25Q64_SPI_DMA_Transmit(cmd,4);
    W25Q64_CS_HIGH();

    W25Q64_WaitBusy();
}

//Ð¾Æ¬²Á³ý
void W25Q64_ChipErase(void)
{
    uint8_t cmd=W25Q64_CHIP_ERASE;

    W25Q64_WriteEnable();
      
    W25Q64_CS_LOW();
    W25Q64_SPI_DMA_Transmit(&cmd,1);
    W25Q64_CS_HIGH();

    W25Q64_WaitBusy();
}

static void W25Q64_WriteEnable(void)
{
    const uint8_t cmd = W25Q64_WRITE_ENABLE;

    W25Q64_CS_LOW();

    W25Q64_SPI_DMA_Transmit((uint8_t *)&cmd, 1);

    W25Q64_CS_HIGH();

}

static void W25Q64_PageWrite(uint32_t addr,uint8_t *buf,uint16_t len)
{
    uint8_t cmd[4];

    cmd[0]=W25Q64_PAGE_PROGRAM;
    cmd[1]=addr>>16;
    cmd[2]=addr>>8;
    cmd[3]=addr;

    W25Q64_WriteEnable();
     
    W25Q64_CS_LOW();
    W25Q64_SPI_DMA_Transmit(cmd,4);
    W25Q64_SPI_DMA_Transmit(buf,len);
    W25Q64_CS_HIGH();

    W25Q64_WaitBusy();
}

static void W25Q64_SPI_DMA_Transmit(uint8_t *txdata, uint16_t len)
{  
    dma_busy_flag = 1;
    HAL_SPI_Transmit_DMA(&hspi2, txdata, len);
    while(dma_busy_flag);
    
}

static void W25Q64_SPI_DMA_Receive(uint8_t *rxdata, uint16_t len)
{
    dma_busy_flag = 1;
    HAL_SPI_Receive_DMA(&hspi2, rxdata, len);
    while(dma_busy_flag);
}

static void W25Q64_SPI_DMA_Transmit_Receive(uint8_t *txdata, uint8_t *rxdata, uint16_t len)
{
    dma_busy_flag = 1;
    HAL_SPI_TransmitReceive_DMA(&hspi2, txdata, rxdata, len);
    while(dma_busy_flag);
}

static void W25Q64_WaitBusy(void)
{
    uint8_t cmd[2] = {W25Q64_READ_STATUS_REGISTER_1, W25Q64_DUMMY_BYTE};
    uint8_t rx[2];

    do
    {
        W25Q64_CS_LOW();
        W25Q64_SPI_DMA_Transmit_Receive(cmd, rx, 2);
        W25Q64_CS_HIGH();
    } while(rx[1] & 0x01);  // BUSY bit
}

void HAL_SPI_TxRxCpltCallback(SPI_HandleTypeDef *hspi)
{
    if(hspi == &hspi2)
    {
        dma_busy_flag = 0;
    }
}

void HAL_SPI_TxCpltCallback(SPI_HandleTypeDef *hspi)
{
    if(hspi == &hspi2)
    {
        dma_busy_flag = 0;
    }
}

void HAL_SPI_RxCpltCallback(SPI_HandleTypeDef *hspi)
{
    if(hspi == &hspi2)
    {
        dma_busy_flag = 0;
    }
}
