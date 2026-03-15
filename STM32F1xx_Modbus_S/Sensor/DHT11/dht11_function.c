#include "dht11.h"
#include "main.h"
#include "FreeRTOS.h"
#include "task.h"
extern TIM_HandleTypeDef htim3;
void DHT11_OutputMode(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;    
	GPIO_InitStructure.Pin=DHT11_Pin;      
	GPIO_InitStructure.Mode=GPIO_MODE_OUTPUT_PP;
	GPIO_InitStructure.Speed=GPIO_SPEED_FREQ_HIGH;
	HAL_GPIO_Init(DHT11_GPIO_Port,&GPIO_InitStructure);
}


void DHT11_OutputLow(void)
{
    HAL_GPIO_WritePin(DHT11_GPIO_Port,DHT11_Pin,GPIO_PIN_RESET);
}

void DHT11_OutputHigh(void)
{
	HAL_GPIO_WritePin(DHT11_GPIO_Port,DHT11_Pin,GPIO_PIN_SET);
}

void DHT11_InputMode(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	GPIO_InitStructure.Pin=DHT11_Pin;
	GPIO_InitStructure.Mode=GPIO_MODE_INPUT;
	GPIO_InitStructure.Speed=GPIO_SPEED_FREQ_HIGH;
	HAL_GPIO_Init(DHT11_GPIO_Port,&GPIO_InitStructure);
}

bool DHT11_GetStatus(void)
{
	bool status;
	status=HAL_GPIO_ReadPin(DHT11_GPIO_Port,DHT11_Pin) == GPIO_PIN_SET ? true : false;
	return status;
}

void DWT_Init(void)
{
    HAL_TIM_Base_Start(&htim3);
}

void DHT11_DelayUs(uint32_t us)
{
	__HAL_TIM_SET_COUNTER(&htim3,0);
    while(__HAL_TIM_GET_COUNTER(&htim3) < us);
}
void DHT11_DelayMs(uint32_t ms)
{
	vTaskDelay(pdMS_TO_TICKS(ms));
}
