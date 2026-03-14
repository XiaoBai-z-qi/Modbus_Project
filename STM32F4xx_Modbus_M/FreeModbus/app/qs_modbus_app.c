#include "qs_modbus_app.h"
#include "mb_master_reg.h"
#include "debug_printf.h"
extern USHORT   usMRegInBuf[MB_MASTER_TOTAL_SLAVE_NUM][M_REG_INPUT_NREGS];
void eQSModbusSlave1(void)
{
    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, GPIO_PIN_SET);
    eMBMasterReqReadInputRegister(QS_SLAVE1_ADDR, 0, 3, 100);
    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, GPIO_PIN_RESET);
    
    
}


void eQSModbusSlave2(void)
{
    return;
}
