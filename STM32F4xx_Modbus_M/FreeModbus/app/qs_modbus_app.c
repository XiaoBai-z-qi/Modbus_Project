#include "qs_modbus_app.h"
#include "mb_master_reg.h"
#include "debug_printf.h"
extern USHORT   usMRegInBuf[MB_MASTER_TOTAL_SLAVE_NUM][M_REG_INPUT_NREGS];
void eQSModbusSlave1(void)
{
    //uint32_t tt = xTaskGetTickCount();
    eMBMasterReqReadInputRegister(QS_SLAVE1_ADDR, 0, 3, 100);
    
	//tt = xTaskGetTickCount() - tt;
    //vDebugPrint("%d, %d, %d, %d\r\n", usMRegInBuf[0][0], usMRegInBuf[0][1], usMRegInBuf[0][2], tt);
    HAL_GPIO_TogglePin(LED_RUN_GPIO_Port, LED_RUN_Pin);
}


void eQSModbusSlave2(void)
{
    return;
}
