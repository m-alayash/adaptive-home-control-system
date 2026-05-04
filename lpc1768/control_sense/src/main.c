#include <lpc17xx.h>
#include <stdio.h>
#include "ocf_lpc176x_lib.h"

#define DATA_PIN (1 << 0)
#define PIR_PIN (1 << 5)   // P2.5


// Function prototypes
void DHT11_Start(void);
int  DHT11_CheckResponse(void);
int  DHT11_ReadBit(void);
int  DHT11_ReadByte(unsigned char *byte);

// ================= MAIN =================
int main(void)
{
    unsigned char hum_int, hum_dec, temp_int, temp_dec, checksum;

    // Ensure P0.0 is GPIO
    LPC_PINCON->PINSEL0 &= ~(3 << 0);

    // Ensure P2.5 is GPIO
    LPC_PINCON->PINSEL4 &= ~(3 << 10);
    LPC_GPIO2->FIODIR &= ~PIR_PIN;

    initUART0();
    initTimer0();
    initUART2();

    printf("Readings:\n");

    delayMS(1000); // sensor stabilization

    while(1)
    {
        DHT11_Start();

        if(DHT11_CheckResponse())
        {
            if(DHT11_ReadByte(&hum_int) &&
               DHT11_ReadByte(&hum_dec) &&
               DHT11_ReadByte(&temp_int) &&
               DHT11_ReadByte(&temp_dec) &&
               DHT11_ReadByte(&checksum))
            {
                if((unsigned char)(hum_int + hum_dec + temp_int + temp_dec) == checksum)
                {
                    // Print to PC Terminal (UART0)
                    int motion = (LPC_GPIO2->FIOPIN & PIR_PIN) ? 1 : 0;

                    printf("Humidity = %d%%  Temp = %d C Motion = %d\n", hum_int, temp_int, motion);

                    // Prepare and send data to ESP32 (UART2)
                    char esp_buffer[32];
                    sprintf(esp_buffer, "H:%d,T:%d,M:%d\n", hum_int, temp_int, motion);
                    U2WriteStr(esp_buffer);
                }
                else
                {
                    printf("Checksum error\n");
                    U2WriteStr("ERROR: Checksum\n");
                }
            }
            else
            {
                printf("Read timeout\n");
                U2WriteStr("ERROR: Read Timeout\n");
            }
        }
        else
        {
            printf("No response\n");
            U2WriteStr("ERROR: No Sensor Response\n");
        }

        delayMS(2000);
    }
}

void DHT11_Start(void)
{
    LPC_GPIO0->FIODIR |= DATA_PIN;

    LPC_GPIO0->FIOSET = DATA_PIN;
    delayMS(1);

    LPC_GPIO0->FIOCLR = DATA_PIN;
    delayMS(20);   // must be >= 18ms

    LPC_GPIO0->FIOSET = DATA_PIN;
    delayUS(40);

    LPC_GPIO0->FIODIR &= ~DATA_PIN;
}

int DHT11_CheckResponse(void)
{
    int timeout = 0;

    // Wait for sensor LOW response
    while(LPC_GPIO0->FIOPIN & DATA_PIN)
    {
        delayUS(1);
        if(++timeout > 200) return 0;
    }

    timeout = 0;

    // Wait for sensor HIGH response
    while(!(LPC_GPIO0->FIOPIN & DATA_PIN))
    {
        delayUS(1);
        if(++timeout > 200) return 0;
    }

    timeout = 0;

    // Wait for LOW before data bits
    while(LPC_GPIO0->FIOPIN & DATA_PIN)
    {
        delayUS(1);
        if(++timeout > 200) return 0;
    }

    return 1;
}

int DHT11_ReadBit(void)
{
    int timeout = 0;
    unsigned int time = 0;

    // Wait for HIGH pulse to start
    while(!(LPC_GPIO0->FIOPIN & DATA_PIN))
    {
        delayUS(1);
        if(++timeout > 100) return -1;
    }

    // Measure HIGH pulse width
    startTimer0();

    while(LPC_GPIO0->FIOPIN & DATA_PIN)
    {
        if(LPC_TIM0->TC > 100) break;
    }

    time = stopTimer0();

    // Around 26-28us = 0, around 70us = 1
    if(time > 40)
        return 1;
    else
        return 0;
}

int DHT11_ReadByte(unsigned char *byte)
{
    unsigned char i;
    int bit;

    *byte = 0;

    for(i = 0; i < 8; i++)
    {
        bit = DHT11_ReadBit();

        if(bit < 0)
            return 0;

        *byte <<= 1;

        if(bit)
            *byte |= 1;
    }

    return 1;
}
