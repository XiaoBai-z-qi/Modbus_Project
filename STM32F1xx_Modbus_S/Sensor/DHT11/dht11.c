#include "dht11.h"

static volatile uint8_t DHT11Buff[5] = {0};

/* ----------------------- static functions ---------------------------------*/
static bool DHT11_GetByte(uint8_t *byte);
static bool DHT11_GetBuff(void);

/* ----------------------- Start implementation -----------------------------*/
void DHT11_GetData(uint8_t *temperature, uint8_t *humidity)
{
    if(DHT11_GetBuff() == true)
    {
        *temperature = DHT11Buff[2];
        *humidity = DHT11Buff[0];
    }
    else
    {
        *temperature = 0;
        *humidity = 0;
    }
}


static bool DHT11_GetByte(uint8_t *byte)
{
    uint8_t bits, temp;
    uint8_t getbyte = 0;
    
    for(bits = 0; bits < 8; bits++)
    {
        uint8_t count=0;
        while(!DHT11_GetStatus() && count < 100)
        {
            DHT11_DelayUs(1);
            count++;
        }
        count = 0;

        DHT11_DelayUs(40);
        if(DHT11_GetStatus())
            temp = 1;
        else
            temp = 0;

        while(DHT11_GetStatus() && count < 100)
        {
            DHT11_DelayUs(1);
            count++;
        }
        count = 0;

        getbyte<<=1;
        getbyte |= temp;
    }
    *byte = getbyte;
    return true;
}

static bool DHT11_GetBuff(void)
{
    uint8_t i,temp;
    DHT11_OutputMode();

    DHT11_OutputLow();
    DHT11_DelayMs(18);
    DHT11_OutputHigh();
    DHT11_DelayUs(20);

    DHT11_InputMode();
    DHT11_DelayUs(20);
    if(!DHT11_GetStatus())
    {
        uint8_t count = 0;
        while(!DHT11_GetStatus() && count < 100)
        {
            DHT11_DelayUs(1);
            count++;
        }
        count = 0;

        while(DHT11_GetStatus() && (count < 100))
        {
            DHT11_DelayUs(1);
            count++;
        }
        count = 0;

        for(i=0; i<5; i++)
        {
            DHT11_GetByte(&temp);
            DHT11Buff[i] = temp;
        }
        DHT11_DelayUs(50);
    }

    uint32_t sum = DHT11Buff[0]+DHT11Buff[1]+DHT11Buff[2]+DHT11Buff[3];
	if(sum == DHT11Buff[4])    
        return true;  
	else   
        return false;
}



