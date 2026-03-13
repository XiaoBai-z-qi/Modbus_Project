#include "qs_modbus_app.h"
#include "mb.h"
#include "mbport.h"

TaskHandle_t xModbusSlaveTaskHandle = NULL;
void eQSModbusSlaveTaskInit(void)
{
    //创建从机任务
    xTaskCreate(eQSModbusSlaveTask, 
                "Modbus Slave",
                256,
                NULL,
                2,
                &xModbusSlaveTaskHandle);
}

void eQSModbusSlaveTask(void *pvParameters)
{
    eMBErrorCode    eStatus;
    eMBSetRegInput(1004, 91);
    eMBSetRegHolding(1005, 100);
    eMBSetRegCoils(1003, 1);
    eStatus = eMBInit( MB_RTU, 0x01, 0, 9600, MB_PAR_NONE );
    eStatus = eMBEnable(  );
	if(eStatus != MB_ENOERR)
    {
        vTaskDelete(NULL);
    }
    while(1)
    {
        eMBPoll(  );
        vTaskDelay(pdMS_TO_TICKS(10));
    }
}
