#include <lpc17xx.h>
#include <stdio.h>
#include "ocf_lpc176x_lib.h"

#define DATA_PIN (1<<0)

// Function prototypes
void DHT11_Start(void);
int  DHT11_CheckResponse(void);
int  DHT11_ReadBit(void);
unsigned char DHT11_ReadByte(void);

// ================= MAIN =================
int main(void)
{
    unsigned char hum_int, hum_dec, temp_int, temp_dec, checksum;

    // Ensure P0.0 is GPIO
    LPC_PINCON->PINSEL0 &= ~(3 << 0);

    initUART0();
    initTimer0();
    initUART2();

    printf("DHT11 LPC1768 Stable Version\n");

    delayMS(1000); // sensor stabilization

    while(1)
    {
        DHT11_Start();

        if(DHT11_CheckResponse())
        {
            hum_int  = DHT11_ReadByte();
            hum_dec  = DHT11_ReadByte();
            temp_int = DHT11_ReadByte();
            temp_dec = DHT11_ReadByte();
            checksum = DHT11_ReadByte();

            if((hum_int + hum_dec + temp_int + temp_dec) == checksum)
            {
                // Print to PC Terminal (UART0)
                printf("Humidity = %d%%  Temp = %d C\n", hum_int, temp_int);

                // Prepare and send data to ESP32 (UART2)
                char esp_buffer[32];
                // Formatting as "H:xx,T:xx" followed by a newline for easy ESP32 parsing
                sprintf(esp_buffer, "H:%d,T:%d\n", hum_int, temp_int);
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
    LPC_GPIO0->FIOSET = DATA_PIN;
}


int DHT11_CheckResponse(void)
{
    int timeout = 0;

    // LOW response
    while(LPC_GPIO0->FIOPIN & DATA_PIN)
    {
        delayUS(1);
        if(++timeout > 200) return 0;
    }

    timeout = 0;

    // HIGH response
    while(!(LPC_GPIO0->FIOPIN & DATA_PIN))
    {
        delayUS(1);
        if(++timeout > 200) return 0;
    }

    timeout = 0;

    // LOW before data
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

    // Wait for LOW (start of bit)
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

    // Threshold decision
    if(time > 40)
        return 1;
    else
        return 0;
}



unsigned char DHT11_ReadByte(void)
{
    unsigned char i, byte = 0;

    for(i = 0; i < 8; i++)
    {
        byte <<= 1;

        if(DHT11_ReadBit())
            byte |= 1;
    }

    return byte;
}
