#include "modbus_manager.h"

QueueHandle_t MBReqQueue;

void MB_RequestInit(void)
{
    MBReqQueue = xQueueCreate(MB_REQ_QUEUE_LEN,sizeof(MBRequest_t));
}

//∂¡ ‰»Îºƒ¥Ê∆˜«Î«Û
BaseType_t MB_ReadInputReg(uint8_t slave, uint16_t addr, uint16_t num, uint16_t timeout)
{
    MBRequest_t req;

    req.slaveAddr = slave;
    req.type = MB_REQ_READ_INPUT;

    req.regAddr = addr;
    req.regNum = num;
    req.timeout = timeout;

    return xQueueSend(MBReqQueue,&req,0);
}

//∂¡±£≥÷ºƒ¥Ê∆˜«Î«Û
BaseType_t MB_ReadHoldingReg(uint8_t slave, uint16_t addr, uint16_t num, uint16_t timeout)
{
    MBRequest_t req;

    req.slaveAddr = slave;
    req.type = MB_REQ_READ_HOLD;

    req.regAddr = addr;
    req.regNum = num;
    req.timeout = timeout;

    return xQueueSend(MBReqQueue,&req,0);     
}

//–¥±£≥÷ºƒ¥Ê∆˜«Î«Û
BaseType_t MB_WriteHoldingReg(uint8_t slave, uint16_t addr, uint16_t *data, uint16_t num, uint16_t timeout)
{
    MBRequest_t req;

    req.slaveAddr = slave;
    req.type = MB_REQ_WRITE_HOLD;

    req.regAddr = addr;
    req.regNum = num;
    req.timeout = timeout;

    memcpy(req.data,data,num*2);

    return xQueueSend(MBReqQueue,&req,0);
}

//∂¡œﬂ»¶«Î«Û
BaseType_t MB_ReadCoil(uint8_t slave, uint16_t addr, uint16_t num, uint16_t timeout)
{
    MBRequest_t req;

    req.slaveAddr = slave;
    req.type = MB_REQ_READ_COIL;

    req.regAddr = addr;
    req.regNum = num;
    req.timeout = timeout;

    return xQueueSend(MBReqQueue,&req,0); 
}

//–¥œﬂ»¶«Î«Û
BaseType_t MB_WriteCoil(uint8_t slave, uint16_t addr, uint16_t *data, uint16_t num, uint16_t timeout)
{
    MBRequest_t req;

    req.slaveAddr = slave;
    req.type = MB_REQ_WRITE_COIL;

    req.regAddr = addr;
    req.regNum = num;
    req.timeout = timeout;

    memcpy(req.data,data,num*2);

    return xQueueSend(MBReqQueue,&req,0);
}


void MB_CheckPoll(void)
{
    uint32_t tick = xTaskGetTickCount();

    for(int i=0; i<MB_POLL_NUM; i++)
    {
        if(tick - PollTable[i].lastTick >= PollTable[i].period)
        {
            MBRequest_t req;

            req.slaveAddr = PollTable[i].slave;
            req.type = PollTable[i].type;

            req.regAddr = PollTable[i].addr;
            req.regNum = PollTable[i].num;
            req.timeout = 100;

            xQueueSend(MBReqQueue,&req,0);

            PollTable[i].lastTick = tick;
        }
    }
}

//void ModbusManagerTask(void *pv)
//{
//    MBRequest_t req;

//    while(1)
//    {
//        if(eMBMasterGetState() != STATE_M_IDLE)
//        {
//            vTaskDelay(1);
//            continue;
//        }

//        if(xQueueReceive(MBReqQueue,&req,0) == pdTRUE)
//        {
//            MB_SendRequest(&req);
//        }

//        MB_CheckPoll();

//        vTaskDelay(1);
//    }
//}



