#include "qs_modbus_app.h"
#include "mb.h"
#include "mbport.h"
#include "mb_master_reg.h"
TaskHandle_t xModbusMasterTaskHandle = NULL;
void eQSModbusMasterTaskInit(void)
{
    //创建从机任务
    xTaskCreate(eQSModbusMasterTask, 
                "Modbus Master",
                256,
                NULL,
                2,
                &xModbusMasterTaskHandle);
}

void eQSModbusMasterTask(void *pvParameters)
{
    eMBErrorCode    eStatus;
    eStatus = eMBMasterInit(MB_RTU, 2, 9600, MB_PAR_NONE);
    eStatus = eMBMasterEnable(  );
	if(eStatus != MB_ENOERR)
    {
        vTaskDelete(NULL);
    }
    while(1)
    {
        eMBMasterPoll(  );
        vTaskDelay(pdMS_TO_TICKS(10));
    }
}
