#ifndef __QS_MODBUS_APP_H__
#define __QS_MODBUS_APP_H__
#include "port.h"
#include "mb.h"



/* ----------------------- Modbus Task ----------------------------------*/
void eQSModbusSlaveTaskInit(void);
void eQSModbusSlaveTask(void *pvParameters);



#endif
