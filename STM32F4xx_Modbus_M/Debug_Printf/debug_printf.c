#include "debug_printf.h"
/* 全局句柄 */
static QueueHandle_t xDebugQueue;
static TaskHandle_t xDebugTaskHandle;
static DebugMsg_t xCurrentMsg;                   /* 当前正在发送的消息 */
static volatile bool xDmaBusy = false;           /* DMA发送忙标志 */

/* 调试任务函数 */
static void vDebugTask(void *pvParameters)
{
  DebugMsg_t msg;
  
  while(1)
  {
    /* 如果DMA空闲，尝试发送下一条消息 */
    if(!xDmaBusy)
    {
      if(xQueueReceive(xDebugQueue, &msg, 0) == pdPASS)  /* 非阻塞接收 */
      {
        /* 保存当前消息（防止msg被覆盖） */
        memcpy(&xCurrentMsg, &msg, sizeof(DebugMsg_t));
        
        /* 标记DMA忙，启动发送 */
        xDmaBusy = true;
        HAL_UART_Transmit_DMA(DEBUG_UART, xCurrentMsg.data, xCurrentMsg.len);
      }
      else
      {
        /* 队列空，任务休眠等待信号量或直接延时 */
        vTaskDelay(pdMS_TO_TICKS(5));  /* 简单延时，避免空转 */
      }
    }
    else
    {
      /* DMA正忙，稍等再检查 */
      vTaskDelay(pdMS_TO_TICKS(1));
    }
  }
}

/* 调试任务初始化 */
void vDebugTaskInit(void)
{
  /* 创建消息队列 */
  xDebugQueue = xQueueCreate(DEBUG_QUEUE_LENGTH, sizeof(DebugMsg_t));
  
  /* 创建调试任务 */
  xTaskCreate(vDebugTask, "Debug", 1280, NULL, 3, &xDebugTaskHandle);
}

/* 调试打印函数（供其他任务调用） */
void vDebugPrint(const char *format, ...)
{
  DebugMsg_t msg;
  va_list args;
  
  /* 格式化字符串 */
  va_start(args, format);
  msg.len = vsnprintf((char*)msg.data, DEBUG_MSG_MAX_LEN, format, args);
  va_end(args);
  
  /* 发送到队列，非阻塞 */
  if(xQueueSend(xDebugQueue, &msg, 0) != pdPASS)
  {
    /* 队列满，丢弃消息（可添加计数统计） */
  }
}

/* DMA发送完成回调（在stm32f4xx_it.c或usart.c中调用） */
void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart)
{
  if(huart->Instance == USART1)
  {
    xDmaBusy = false;  /* 标记DMA空闲 */
  }
}
