#include "ymodem.h"
#include "flash_if.h"
#include "main.h"
#include <stdio.h>
#include <string.h>
uint8_t FileName[FILE_NAME_LENGTH];
uint8_t debug_count = 0;
extern UART_HandleTypeDef huart1;

/* ----------------------- ymodem static function ----------------------------------*/


static int8_t Ymodem_ReceiveByte(uint8_t *byte, uint32_t timeout);
static uint8_t Str2Int(uint8_t *inputstr, int32_t *intnum);

static int32_t Ymodem_ReceivePacket(uint8_t *data, int *length, uint32_t timeout)
{
    uint8_t header;
    uint16_t i, packet_size;
    *length = 0;
    if (Ymodem_ReceiveByte(&header, timeout) != 0)
        return -1;

    switch (header)
    {
    case SOH: /* 包的大小为128字节 */
        packet_size = PACKET_SIZE;
        break;
    case STX: /* 包的大小为1024字节 */
        packet_size = PACKET_1K_SIZE;
        break;
    case EOT: /* 传输结束 */
        return 0;
    case CAN: /* PC取消传输 */
        if (Ymodem_ReceiveByte(&header, timeout) == 0 && (header == CAN))
        {
            *length = -1;
            return 0;
        }
        else
        {
            return -1;
        }

    case ABORT1: /* 用户主动终止 */
    case ABORT2: /* 用户主动终止 */
        return 1;
    default: /* 收到非法字节 */
        return -1;
    }

    *data = header;
    for (i = 1; i < (packet_size + PACKET_OVERHEAD); i++)
    {
        if (Ymodem_ReceiveByte(data + i, timeout) != 0)
            return -1;
    }

    if (data[PACKET_SEQNO_INDEX] != ((data[PACKET_SEQNO_COMP_INDEX] ^ 0xff) & 0xff))
        return -1;

    *length = packet_size;
    return 0;
}

int32_t Ymodem_Receive(uint8_t *buf)
{
    uint8_t packet_data[PACKET_1K_SIZE + PACKET_OVERHEAD], file_size[FILE_SIZE_LENGTH], *file_ptr, *buf_ptr;
    int32_t i, packet_length, session_done, file_done, packets_received, errors, session_begin, size = 0;
    uint32_t flashdestination, ramsource;

    /* Initialize flashdestination variable */
    flashdestination = APPLICATION_ADDRESS;

    for (session_done = 0, errors = 0, session_begin = 0;;)
    {
        for (packets_received = 0, file_done = 0, buf_ptr = buf;;)
        {
            switch (Ymodem_ReceivePacket(packet_data, &packet_length, NAK_TIMEOUT))
            {
                case 1:
                    Ymodem_SendByte(CAN);
                    Ymodem_SendByte(CAN);
                    return -3;


                case 0:
                    errors = 0;
                    switch(packet_length)
                    {
                        case -1:
                            Ymodem_SendByte(ACK);
                            return 0;

                        case 0:
                            Ymodem_SendByte(ACK);
                            file_done = 1;        /* 一包传输结束 */
                        break;

                        default:
                            if ((packet_data[PACKET_SEQNO_INDEX] & 0xff) != (packets_received & 0xff))
                            {
                                Ymodem_SendByte(NAK);
                            }
                            else
                            {
                                if (packets_received == 0)
                                {
                                    if (packet_data[PACKET_HEADER] != 0)
                                    {
                                        for (i = 0, file_ptr = packet_data + PACKET_HEADER; (*file_ptr != 0) && (i < FILE_NAME_LENGTH);)
                                        {
                                            FileName[i++] = *file_ptr++;
                                        }
                                            FileName[i++] = '\0';
                                        for (i = 0, file_ptr ++; (*file_ptr != ' ') && (i < FILE_SIZE_LENGTH);)
                                        {
                                            file_size[i++] = *file_ptr++;
                                        }
                                            file_size[i++] = '\0';
                                        Str2Int(file_size, &size);

                                        if (size > (USER_FLASH_SIZE + 1))
                                        {
                                            Ymodem_SendByte(CAN);
                                            Ymodem_SendByte(CAN);
                                            return -1;
                                        }
                                        else
                                        {
                                            /* 准备Flash 写入 */
                                            //FLASH_If_Erase(APPLICATION_ADDRESS);
                                            //printf("***this file is %s, size is %d bytes***\r\n", FileName, size);
                                            Ymodem_SendByte(ACK);
                                            Ymodem_SendByte(CRC16);
                                        }
                                    }
                                    else
                                    {
                                        Ymodem_SendByte(ACK);
                                        file_done = 1;
                                        session_done = 1;        
                                    }
                                }
                                else
                                {
                                    memcpy(buf_ptr, packet_data + PACKET_HEADER, packet_length);
                                    ramsource = (uint32_t)buf;      // 缓冲区地址
									debug_count++;
                                    //printf("===yes I recv data===\r\n");

                                    /* Write received data in Flash */
                                    // if (FLASH_If_Write(&flashdestination, (uint32_t*) ramsource, (uint16_t) packet_length/4)  == 0)
                                    // {
                                         Ymodem_SendByte(ACK);
                                    // }
                                    // else /* An error occurred while writing to Flash memory */
                                    // {
                                    //     /* End session */
                                    //     Ymodem_SendByte(CAN);
                                    //     Ymodem_SendByte(CAN);
                                    //     return -2;
                                    // }
                                }
                                packets_received++;
                                session_begin = 1;          // 会话开始
                            }
                        break;
                    }
                break;

                default:
                    if (session_begin > 0)
                    {
                        errors ++;
                    }
                    if (errors > MAX_ERRORS)
                    {
                        Ymodem_SendByte(CAN);
                        Ymodem_SendByte(CAN);
                        return 0;
                    }
                    Ymodem_SendByte(CRC16);  
                break;

            }
            if(file_done != 0)
                break;
        }
        if(session_done != 0)
            break;
    }
	return (int32_t)size;
}


/* ----------------------- 功能函数 ----------------------------------*/

/*
    * 将字符串转换为整数
    *
    *      "12345" → 12345         （支持）
    *      "0x1A3F" → 6719         （不支持 未来可扩展）
    * 功能说明：
    *   该函数将输入的数字字符串转换为32位有符号整数，支持处理空格分隔的字符串
    *   并包含溢出检查机制
    *
    */
static uint8_t Str2Int(uint8_t *inputstr, int32_t *intnum)
{
    uint32_t result = 0;

    if (!inputstr || !intnum)
        return 0;

    while (*inputstr)
    {
        if (*inputstr == ' ')
            break;

        if (*inputstr < '0' || *inputstr > '9')
            return 0;

        /* 防止溢出 */
        if (result > 0xFFFFFFFF / 10)
            return 0;

        result = result * 10 + (*inputstr - '0');
        inputstr++;
    }

    *intnum = result;
    return 1;
}

/* ----------------------- 用户自定义 ----------------------------------*/
void Ymodem_SendByte(uint8_t byte)
{
    HAL_UART_Transmit(&huart1, &byte, 1, 100);
}

static int8_t Ymodem_ReceiveByte(uint8_t *byte, uint32_t timeout)
{
    HAL_StatusTypeDef status;
    status = HAL_UART_Receive(&huart1, byte, 1, timeout);
    return (status == HAL_OK ? 0 : -1);
}
