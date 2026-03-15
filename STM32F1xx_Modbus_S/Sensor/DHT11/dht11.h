#ifndef __DHT11_H__
#define __DHT11_H__
#include "stdint.h"
#include "stdbool.h"

/* ----------------------- dht11.c ---------------------------------*/

void DHT11_GetData(uint8_t *temperature, uint8_t *humidity);

/* ----------------------- dht11_function.c ---------------------------------*/

void DHT11_OutputMode(void);
void DHT11_OutputLow(void);
void DHT11_OutputHigh(void);

void DHT11_InputMode(void);
bool DHT11_GetStatus(void);

void DWT_Init(void);
void DHT11_DelayUs(uint32_t us);
void DHT11_DelayMs(uint32_t ms);


#endif
