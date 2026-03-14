#include "qs_modbus_app.h"
#include "mb.h"
#include "mbport.h"
#include "mb_master_reg.h"
#include "debug_printf.h"
TaskHandle_t xModbusMasterTaskHandle = NULL;
TaskHandle_t xModbusSlavePollTaskHandle = NULL;
void eQSModbusMasterTaskInit(void)
{
    //创建主机任务
    xTaskCreate(eQSModbusMasterTask, 
                "Master",
                256,
                NULL,
                3,
                &xModbusMasterTaskHandle);

    //创建从机轮询任务
    xTaskCreate(eQSModbusSlavePollTask, 
                "Slave Poll",
                256,
                NULL,
                2,
                &xModbusSlavePollTaskHandle);
}

void eQSModbusMasterTask(void *pvParameters)
{
    eMBErrorCode    eStatus;
    eStatus = eMBMasterInit(MB_RTU, 2, 115200, MB_PAR_NONE);
    eStatus = eMBMasterEnable(  );
	if(eStatus != MB_ENOERR)
    {
        vTaskDelete(NULL);
    }
    while(1)
    {
        eMBMasterPoll(  );
		//vTaskDelay(pdMS_TO_TICKS(10));  
    }
}

void eQSModbusSlavePollTask(void *pvParameters)
{
    while(1)
    {
        eQSModbusSlave1();
        eQSModbusSlave2();
        //vTaskDelay(pdMS_TO_TICKS(10));
        
    }
}
