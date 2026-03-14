#ifndef __QS_MODBUS_APP_H__
#define __QS_MODBUS_APP_H__
#include "port.h"
#include "mb.h"

#define QS_SLAVE1_ADDR 0x01
/* ----------------------- Modbus App ----------------------------------*/
void eQSModbusSlave1(void);
void eQSModbusSlave2(void);
/* ----------------------- Modbus Task ----------------------------------*/
void eQSModbusMasterTaskInit(void);
void eQSModbusMasterTask(void *pvParameters);
void eQSModbusSlavePollTask(void *pvParameters);


#endif
