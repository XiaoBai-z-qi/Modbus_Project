/*
 * FreeModbus Libary: BARE Demo Application
 * Copyright (C) 2006 Christian Walter <wolti@sil.at>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 *
 * File: $Id$
 */

/* ----------------------- Modbus includes ----------------------------------*/
#include "mb.h"
#include "mbport.h"

/* ----------------------- Input Defines And Static variables------------------------------------------*/
#define REG_INPUT_START 1000
#define REG_INPUT_NREGS 4
static USHORT   usRegInputStart = REG_INPUT_START;
static USHORT   usRegInputBuf[REG_INPUT_NREGS] = {2, 5, 7, 10};

/* ----------------------- Holding Defines And Static variables------------------------------------------*/
#define REG_HOLDING_START 1000
#define REG_HOLDING_NREGS 4
static USHORT   usRegHoldingStart = REG_HOLDING_START;
static USHORT   usRegHoldingBuf[REG_HOLDING_NREGS] = {7, 3, 2, 1};

/* ----------------------- Holding Defines And Static variables------------------------------------------*/
#define REG_COILS_START 1000
#define REG_COILS_NCOILS  16
static USHORT   usRegCoilsStart = REG_COILS_START;
#define REG_COILS_BYTES      ((REG_COILS_NCOILS + 7) / 8)
static UCHAR    ucRegCoilsBuf[REG_COILS_BYTES] = {0xb1, 0xc3};



/* ----------------------- Start implementation -----------------------------*/

eMBErrorCode
eMBRegInputCB( UCHAR * pucRegBuffer, USHORT usAddress, USHORT usNRegs )
{
    eMBErrorCode    eStatus = MB_ENOERR;
    int             iRegIndex;

    usAddress--;
    if( ( usAddress >= REG_INPUT_START )
        && ( usAddress + usNRegs <= REG_INPUT_START + REG_INPUT_NREGS ) )
    {
        iRegIndex = ( int )( usAddress - usRegInputStart );
        while( usNRegs > 0 )
        {
            *pucRegBuffer++ =
                ( unsigned char )( usRegInputBuf[iRegIndex] >> 8 );
            *pucRegBuffer++ =
                ( unsigned char )( usRegInputBuf[iRegIndex] & 0xFF );
            iRegIndex++;
            usNRegs--;
        }
    }
    else
    {
        eStatus = MB_ENOREG;
    }

    return eStatus;
}

eMBErrorCode
eMBRegHoldingCB( UCHAR * pucRegBuffer, USHORT usAddress, USHORT usNRegs,
                 eMBRegisterMode eMode )
{
    eMBErrorCode    eStatus = MB_ENOERR;
    int             iRegIndex;

    usAddress--;
    if( ( usAddress >= REG_HOLDING_START )
        && ( usAddress + usNRegs <= REG_HOLDING_START + REG_HOLDING_NREGS ) )
    {
        iRegIndex = ( int )( usAddress - usRegHoldingStart );
        if(eMode == MB_REG_READ)
        {
                while( usNRegs > 0 )
                {
                    *pucRegBuffer++ =
                        ( unsigned char )( usRegHoldingBuf[iRegIndex] >> 8 ); 
                    *pucRegBuffer++ =
                        ( unsigned char )( usRegHoldingBuf[iRegIndex] & 0xFF );
                    iRegIndex++;
                    usNRegs--;
                }
        }else if(eMode == MB_REG_WRITE)
        {
                while( usNRegs > 0 )
                {
                    usRegHoldingBuf[iRegIndex] = *pucRegBuffer++ << 8;
                    usRegHoldingBuf[iRegIndex] |= *pucRegBuffer++;
                    iRegIndex++;
                    usNRegs--;
                } 
        }
        
    }
    else
    {
        eStatus = MB_ENOREG;
    }

    return eStatus;
}


eMBErrorCode
eMBRegCoilsCB( UCHAR * pucRegBuffer, USHORT usAddress, USHORT usNCoils,
               eMBRegisterMode eMode )
{
    eMBErrorCode eStatus = MB_ENOERR;
    USHORT iRegIndex;       //线圈数组中的字节索引
    USHORT iBitIndex;       //字节中的bit索引
    USHORT iCoilIndex;      //当前线圈编号
    
    usAddress--;
    if ( ( usAddress >= REG_COILS_START ) 
         && ( usAddress + usNCoils <= REG_COILS_START + REG_COILS_NCOILS ) )
    {
        iCoilIndex = usAddress - usRegCoilsStart;   //计算线圈在数组中的起始位置
        if (eMode == MB_REG_READ)
        {
            UCHAR ucByte = 0;                       //临时存储发送的字节
            UCHAR ucBitCount = 0;                   //当前字节中bit计数

            while(usNCoils > 0)
            {
                iRegIndex = iCoilIndex / 8;         //计算当前线圈字节索引
                iBitIndex = iCoilIndex % 8;         //计算当前线圈在字节中的位置

                //如果线圈为1, 则把对应bit写入Byte
                if(ucRegCoilsBuf[iRegIndex] & (1 << iBitIndex))
                {
                    ucByte |= (1 << ucBitCount);
                }
                ucBitCount++;
                iCoilIndex++;
                usNCoils--;

                if(ucBitCount == 8 || usNCoils == 0)    //组成一个字节后放入buff，清空Byte和计数器
                {
                    *pucRegBuffer++ = ucByte;
                    ucByte = 0;
                    ucBitCount = 0;
                }
            }
        }
        else if (eMode == MB_REG_WRITE)
        {
            UCHAR ucByte;           // 当前接收到的字节
            UCHAR ucBitCount = 0;   // 当前字节的bit位置

            while (usNCoils > 0)
            {
                /* 每处理8个线圈，从buffer取一个byte */
                if (ucBitCount == 0)
                {
                    ucByte = *pucRegBuffer++;
                }

                /* 计算线圈在数组中的位置 */
                iRegIndex = iCoilIndex / 8;
                iBitIndex = iCoilIndex % 8;

                /* 判断当前bit是否为1 */
                if (ucByte & (1 << ucBitCount))
                {
                    ucRegCoilsBuf[iRegIndex] |= (1 << iBitIndex);  // 置1
                }
                else
                {
                    ucRegCoilsBuf[iRegIndex] &= ~(1 << iBitIndex); // 置0
                }

                ucBitCount++;
                iCoilIndex++;
                usNCoils--;

                /* 8bit处理完，准备读取下一个字节 */
                if (ucBitCount == 8)
                {
                    ucBitCount = 0;
                }
            }
        }
    }
    else
    {
        eStatus = MB_ENOREG;
    }
    
    return eStatus;
}

eMBErrorCode
eMBRegDiscreteCB( UCHAR * pucRegBuffer, USHORT usAddress, USHORT usNDiscrete )
{
    return MB_ENOREG;
}

// int
// modbus_test( void )
// {
//     eMBErrorCode    eStatus;

//     eStatus = eMBInit( MB_RTU, 0x0A, 0, 38400, MB_PAR_EVEN );

//     /* Enable the Modbus Protocol Stack. */
//     eStatus = eMBEnable(  );

//     for( ;; )
//     {
//         ( void )eMBPoll(  );

//         /* Here we simply count the number of poll cycles. */
//         usRegInputBuf[0]++;
//     }
// }
