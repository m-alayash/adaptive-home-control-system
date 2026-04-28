#ifndef OCF_LPC176X_LIB
#define OCF_LPC176X_LIB

#include <lpc17xx.h>

// --- UART Register Bits ---
#define RDR         (1<<0)  // Receiver Data Ready
#define THRE        (1<<5)  // Transmit Holding Register Empty
#define DLAB_BIT    (1<<7)  // Divisor Latch Access Bit
#define Ux_FIFO_EN  (1<<0)
#define Rx_FIFO_RST (1<<1)
#define Tx_FIFO_RST (1<<2)

// --- UART2 Specific Power Control ---
#define PCUART2     (1<<24) // UART2 Power/Clock control bit

// --- Baud Rate Constants (for 25MHz PCLK) ---
#define MULVAL      15
#define DIVADDVAL   2
// For 115200 Baud: DLL=12, DLM=0
// For 9600 Baud:   DLL=163, DLM=0

// --- Character Constants ---
#define LINE_FEED       0x0A
#define CARRIAGE_RETURN 0x0D

// --- Timer Constants ---
#define PRESCALE (25-1) // 25MHz PCLK to 1 microsecond resolution

// --- Function Prototypes ---

// Original UART0 (Keep for Terminal Debugging if needed)
void initUART0(void);
void U0Write(char data);
char U0Read(void);

// NEW UART2 (For ESP32-S3 Communication)
void initUART2(void);
void U2Write(char data);
char U2Read(void);
void U2WriteStr(char *str); // Add this line

// Timer & Delay Functions
void initTimer0(void);
void startTimer0(void);
unsigned int stopTimer0(void);
void delayUS(unsigned int microseconds);
void delayMS(unsigned int milliseconds);

#endif
