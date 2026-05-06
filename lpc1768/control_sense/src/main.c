#include <lpc17xx.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "ocf_lpc176x_lib.h"

#define DATA_PIN (1 << 0)  // P0.0
#define PIR_PIN  (1 << 5)  // P2.5
#define LED_PIN  (1 << 9)  // P0.9

#define PWM_PERIOD_TICKS 25000

typedef enum {
    MOTOR_DYNAMIC = 0,
    MOTOR_MANUAL
} MotorMode;

MotorMode motorMode = MOTOR_DYNAMIC;
int motorPower = 0;
int manualMotorPower = 0;
int lastTempC = 0;

int ledState = 0;
int lastPirLedState = 0;

char uart2CmdBuffer[32];
unsigned int uart2CmdIndex = 0;

void DHT11_Start(void);
int  DHT11_CheckResponse(void);
int  DHT11_ReadBit(void);
int  DHT11_ReadByte(unsigned char *byte);

void LED_Init(void);
void LED_Set(int on);

void PWM1_Init(void);
void PWM1_SetPower(int percent);
int  GetDynamicCoolingPower(int tempC);
int  GetMotorPower(int tempC);
const char* GetMotorModeText(void);

void UART2_ProcessCommands(void);
void HandleCommand(char *cmd);
void DelayWithCommandPolling(unsigned int ms);

int main(void)
{
    unsigned char hum_int, hum_dec, temp_int, temp_dec, checksum;

    LPC_PINCON->PINSEL0 &= ~(3 << 0);

    LPC_PINCON->PINSEL4 &= ~(3 << 10);
    LPC_GPIO2->FIODIR &= ~PIR_PIN;

    LED_Init();

    initUART0();
    initTimer0();
    initUART2();
    PWM1_Init();

    printf("Readings:\n");

    delayMS(1000);

    while(1)
    {
        UART2_ProcessCommands();

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
                    int motion = (LPC_GPIO2->FIOPIN & PIR_PIN) ? 1 : 0;

                    if(motion && !lastPirLedState)
                    {
                        LED_Set(1);
                    }
                    lastPirLedState = motion;

                    lastTempC = temp_int;
                    motorPower = GetMotorPower(lastTempC);
                    PWM1_SetPower(motorPower);

                    printf("Humidity = %d%%  Temp = %d C  Motion = %d  Motor = %d%%  Mode = %s  LED = %d\n",
                           hum_int, temp_int, motion, motorPower, GetMotorModeText(), ledState);

                    char esp_buffer[96];
                    sprintf(esp_buffer, "H:%d,T:%d,M:%d,P:%d,L:%d,MODE:%s\n",
                            hum_int, temp_int, motion, motorPower, ledState, GetMotorModeText());
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

        DelayWithCommandPolling(2000);
    }
}

void LED_Init(void)
{
    LPC_PINCON->PINSEL0 &= ~(3 << 18);
    LPC_GPIO0->FIODIR |= LED_PIN;
    LED_Set(0);
}

void LED_Set(int on)
{
    if(on)
    {
        LPC_GPIO0->FIOSET = LED_PIN;
        ledState = 1;
    }
    else
    {
        LPC_GPIO0->FIOCLR = LED_PIN;
        ledState = 0;
    }
}

void PWM1_Init(void)
{
    LPC_SC->PCONP |= (1 << 6);

    LPC_PINCON->PINSEL4 &= ~(3 << 0);
    LPC_PINCON->PINSEL4 |=  (1 << 0);

    LPC_PWM1->TCR = (1 << 1);
    LPC_PWM1->PR = 0;
    LPC_PWM1->MR0 = PWM_PERIOD_TICKS;
    LPC_PWM1->MR1 = 0;

    LPC_PWM1->MCR = (1 << 1);
    LPC_PWM1->LER = (1 << 0) | (1 << 1);
    LPC_PWM1->PCR = (1 << 9);
    LPC_PWM1->TCR = (1 << 0) | (1 << 3);
}

void PWM1_SetPower(int percent)
{
    if(percent < 0) percent = 0;
    if(percent > 100) percent = 100;

    LPC_PWM1->MR1 = (PWM_PERIOD_TICKS * percent) / 100;
    LPC_PWM1->LER = (1 << 1);
}

int GetDynamicCoolingPower(int tempC)
{
    if(tempC < 24) return 0;
    if(tempC < 27) return 35;
    if(tempC < 30) return 65;
    return 100;
}

int GetMotorPower(int tempC)
{
    if(motorMode == MOTOR_MANUAL)
        return manualMotorPower;

    return GetDynamicCoolingPower(tempC);
}

const char* GetMotorModeText(void)
{
    if(motorMode == MOTOR_MANUAL)
        return "MANUAL";

    return "AUTO";
}

void UART2_ProcessCommands(void)
{
    while(LPC_UART2->LSR & 0x01)
    {
        char c = LPC_UART2->RBR;

        if(c == '\n' || c == '\r')
        {
            if(uart2CmdIndex > 0)
            {
                uart2CmdBuffer[uart2CmdIndex] = '\0';
                HandleCommand(uart2CmdBuffer);
                uart2CmdIndex = 0;
            }
        }
        else
        {
            if(uart2CmdIndex < sizeof(uart2CmdBuffer) - 1)
                uart2CmdBuffer[uart2CmdIndex++] = c;
            else
                uart2CmdIndex = 0;
        }
    }
}

void HandleCommand(char *cmd)
{
    printf("Received command: [%s]\n", cmd);

    if(strcmp(cmd, "CMD:AUTO") == 0)
    {
        motorMode = MOTOR_DYNAMIC;
    }
    else if(strncmp(cmd, "CMD:MANUAL:", 11) == 0)
    {
        manualMotorPower = atoi(cmd + 11);

        if(manualMotorPower < 0) manualMotorPower = 0;
        if(manualMotorPower > 100) manualMotorPower = 100;

        motorMode = MOTOR_MANUAL;
    }
    else if(strcmp(cmd, "CMD:LED:ON") == 0)
    {
        LED_Set(1);
    }
    else if(strcmp(cmd, "CMD:LED:OFF") == 0)
    {
        LED_Set(0);
    }
    else
    {
        return;
    }

    motorPower = GetMotorPower(lastTempC);
    PWM1_SetPower(motorPower);

    printf("Command state: Motor = %d%%  Mode = %s  LED = %d\n",
           motorPower, GetMotorModeText(), ledState);

    char reply[60];
    sprintf(reply, "P:%d,L:%d,MODE:%s\n", motorPower, ledState, GetMotorModeText());
    U2WriteStr(reply);
}

void DelayWithCommandPolling(unsigned int ms)
{
    while(ms >= 20)
    {
        UART2_ProcessCommands();
        delayMS(20);
        ms -= 20;
    }

    if(ms > 0)
        delayMS(ms);
}

void DHT11_Start(void)
{
    LPC_GPIO0->FIODIR |= DATA_PIN;

    LPC_GPIO0->FIOSET = DATA_PIN;
    delayMS(1);

    LPC_GPIO0->FIOCLR = DATA_PIN;
    delayMS(20);

    LPC_GPIO0->FIOSET = DATA_PIN;
    delayUS(40);

    LPC_GPIO0->FIODIR &= ~DATA_PIN;
}

int DHT11_CheckResponse(void)
{
    int timeout = 0;

    while(LPC_GPIO0->FIOPIN & DATA_PIN)
    {
        delayUS(1);
        if(++timeout > 200) return 0;
    }

    timeout = 0;

    while(!(LPC_GPIO0->FIOPIN & DATA_PIN))
    {
        delayUS(1);
        if(++timeout > 200) return 0;
    }

    timeout = 0;

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

    while(!(LPC_GPIO0->FIOPIN & DATA_PIN))
    {
        delayUS(1);
        if(++timeout > 100) return -1;
    }

    startTimer0();

    while(LPC_GPIO0->FIOPIN & DATA_PIN)
    {
        if(LPC_TIM0->TC > 100) break;
    }

    time = stopTimer0();

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
