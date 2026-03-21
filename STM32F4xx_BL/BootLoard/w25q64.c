#include "w25q64.h"


extern SPI_HandleTypeDef hspi2;

void SPI_Star(void);
void SPI_Stop(void);


/**
 * @brief   交换发送接收的字节
 * @param   ByteSend  ——  要发送的数据
 * @retval  收到的数据
 */
uint8_t SPI_SwapByte(uint8_t ByteSend)
{
    
    uint8_t ByteRecv;
    HAL_SPI_TransmitReceive(&hspi2, &ByteSend, &ByteRecv, 1,1000);
    return ByteRecv;
}


/**
 * 函    数：W25Q64读取ID号
 * 参    数：MID 工厂ID，使用输出参数的形式返回
 * 参    数：DID 设备ID，使用输出参数的形式返回
 * 返 回 值：无
 */
void W25Q64_ReadID(uint8_t *MID, uint16_t *DID)
{
    SPI_Star();								    //SPI起始
    SPI_SwapByte(W25Q64_JEDEC_ID);			    //交换发送读取ID的指令
    *MID = SPI_SwapByte(W25Q64_DUMMY_BYTE);	    //交换接收MID，通过输出参数返回
    *DID = SPI_SwapByte(W25Q64_DUMMY_BYTE);	    //交换接收DID高8位
    *DID <<= 8;									//高8位移到高位
    *DID |= SPI_SwapByte(W25Q64_DUMMY_BYTE);	//或上交换接收DID的低8位，通过输出参数返回
    SPI_Stop();								    //SPI终止
}

//写使能
void W25Q64_WriteEnable(void)
{
    SPI_Star();								//SPI起始
    SPI_SwapByte(W25Q64_WRITE_ENABLE);		//交换发送写使能指令
    SPI_Stop();								//SPI终止
}

//读状态寄存器——判断芯片是否为忙状态
void W25Q64_WaitBusy(void)
{
    SPI_Star();								                    //SPI起始
    SPI_SwapByte(W25Q64_READ_STATUS_REGISTER_1);	            //交换发送读取状态寄存器指令
    while((SPI_SwapByte(W25Q64_DUMMY_BYTE) & 0x01) == 0x01);	//等待芯片忙状态
    SPI_Stop();								                    //SPI终止
}


//页编程
void W25Q64_PageProgram(uint32_t addr, uint8_t *buf, uint16_t len)
{
    W25Q64_WriteEnable();					//写使能

    SPI_Star();								//SPI起始
    SPI_SwapByte(W25Q64_PAGE_PROGRAM);		//交换发送页编程指令
    SPI_SwapByte(addr >> 16);				//交换发送地址的高8位
    SPI_SwapByte(addr >> 8);				//交换发送地址的中间8位
    SPI_SwapByte(addr);					//交换发送地址的低8位
    for(uint16_t i = 0; i < len; i++)
    {
        SPI_SwapByte(buf[i]);				//交换发送数据
    }
    SPI_Stop();								//SPI终止

    W25Q64_WaitBusy();						//等待芯片忙状态
}

//自动分页写
void W25Q64_WriteData(uint32_t addr,uint8_t *buf,uint32_t len)
{
    uint16_t page_remain;

    while(len)
    {
        page_remain = 256 - (addr % 256);

        if(len < page_remain)
            page_remain = len;

        W25Q64_PageProgram(addr,buf,page_remain);

        addr += page_remain;
        buf  += page_remain;
        len  -= page_remain;
    }
}

//扇区擦除
void W25Q64_SectorErase(uint32_t addr)
{
    W25Q64_WriteEnable();					//写使能

    SPI_Star();								//SPI起始
    SPI_SwapByte(W25Q64_SECTOR_ERASE_4KB);		//交换发送扇区擦除指令
    SPI_SwapByte(addr >> 16);				//交换发送地址的高8位
    SPI_SwapByte(addr >> 8);				//交换发送地址的中间8位
    SPI_SwapByte(addr);					//交换发送地址的低8位
    SPI_Stop();								//SPI终止

    W25Q64_WaitBusy();						//等待芯片忙状态
}


//块擦除
void W25Q64_BlockErase(uint32_t addr)
{
    W25Q64_WriteEnable();					//写使能

    SPI_Star();								//SPI起始
    SPI_SwapByte(W25Q64_BLOCK_ERASE_64KB);		//交换发送块擦除指令
    SPI_SwapByte(addr >> 16);				//交换发送地址的高8位    
    SPI_SwapByte(addr >> 8);				//交换发送地址的中间8位
    SPI_SwapByte(addr);					//交换发送地址的低8位
    SPI_Stop();								//SPI终止

    W25Q64_WaitBusy();						//等待芯片忙状态
}

//芯片擦除
void W25Q64_ChipErase_NoCheck(void)
{
    W25Q64_WriteEnable();					//写使能

    SPI_Star();								//SPI起始
    SPI_SwapByte(W25Q64_CHIP_ERASE);			//交换发送芯片擦除指令
    SPI_Stop();								//SPI终止

    W25Q64_WaitBusy();						//等待芯片忙状态
}

//读取数据
void W25Q64_ReadData(uint32_t addr, uint8_t *buf, uint32_t len)
{
    SPI_Star();								//SPI起始
    SPI_SwapByte(W25Q64_READ_DATA);			//交换发送读取数据指令
    SPI_SwapByte(addr >> 16);				//交换发送地址的高8位
    SPI_SwapByte(addr >> 8);				//交换发送地址的中间8位
    SPI_SwapByte(addr);					//交换发送地址的低8位
    for(uint32_t i = 0; i < len; i++)
    {
        buf[i] = SPI_SwapByte(W25Q64_DUMMY_BYTE);	//交换接收数据
    }
    SPI_Stop();								//SPI终止
}
  






void SPI_Star(void)
{
    HAL_GPIO_WritePin(W25Q64_CS_GPIO_Port, W25Q64_CS_Pin, GPIO_PIN_RESET);
}

void SPI_Stop(void)
{
    HAL_GPIO_WritePin(W25Q64_CS_GPIO_Port, W25Q64_CS_Pin, GPIO_PIN_SET);    
}
