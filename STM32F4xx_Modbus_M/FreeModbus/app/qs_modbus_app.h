#ifndef __QS_MODBUS_APP_H__
#define __QS_MODBUS_APP_H__
#include "port.h"
#include "mb.h"



/* ----------------------- Modbus Task ----------------------------------*/
void eQSModbusMasterTaskInit(void);
void eQSModbusMasterTask(void *pvParameters);



#endif
