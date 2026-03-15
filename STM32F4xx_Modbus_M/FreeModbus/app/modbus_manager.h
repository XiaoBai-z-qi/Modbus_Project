#ifndef __MODBUS_MANAGER_H__
#define __MODBUS_MANAGER_H__
#include "stm32f4xx_hal.h"
#include "FreeRTOS.h"
#include "queue.h"
#include "mb_master_reg.h"
#include <string.h>

#define MB_REQ_QUEUE_LEN 20
#define MB_MAX_DATA 16


typedef enum
{
    MB_REQ_READ_INPUT,          //读输入寄存器
    MB_REQ_READ_HOLD,           //读保持寄存器
    MB_REQ_WRITE_HOLD,          //写保持寄存器
    MB_REQ_READ_COIL,           //读线圈
    MB_REQ_WRITE_COIL           //写线圈
}MBReqType_t;




typedef struct
{
    uint8_t slaveAddr;              //从站地址
    MBReqType_t type;               //请求类型
    uint16_t regAddr;               //寄存器地址
    uint16_t regNum;                //寄存器个数
    uint16_t data[MB_MAX_DATA];     //数据
    uint16_t timeout;               //超时时间
}MBRequest_t;


/*----------------------- 轮询表 ----------------------*/

typedef struct
{
    uint8_t slave;                  //从站地址
    MBReqType_t type;               //请求类型
    uint16_t addr;                  //寄存器地址
    uint16_t num;                   //寄存器个数
    uint32_t period;                //轮询周期
    uint32_t lastTick;              //上次轮询时间
}MBPollItem_t;

MBPollItem_t PollTable[] =
{
    {1,MB_REQ_READ_INPUT,0,2,1000,0},
    {1,MB_REQ_READ_INPUT,10,2,2000,0},
    {2,MB_REQ_READ_HOLD,0,2,500,0}
};

#define MB_POLL_NUM (sizeof(PollTable)/sizeof(MBPollItem_t))



#endif

