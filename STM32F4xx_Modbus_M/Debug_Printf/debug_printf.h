#ifndef __DEBUG_PRINTF_H__
#define __DEBUG_PRINTF_H__
#include "stm32f4xx_hal.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <stdbool.h>

extern UART_HandleTypeDef huart1;
/* 调试任务相关定义 */
#define DEBUG_UART            &huart1           /* 使用串口1 */
#define DEBUG_QUEUE_LENGTH    20                /* 队列长度 */
#define DEBUG_MSG_MAX_LEN     128               /* 单条消息最大长度 */

/* 调试消息结构 */
typedef struct
{
  uint8_t data[DEBUG_MSG_MAX_LEN];
  uint16_t len;
} DebugMsg_t;

void vDebugTaskInit(void);
void vDebugPrint(const char *format, ...);

#endif
