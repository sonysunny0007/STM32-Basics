#ifndef UART_H
#define UART_H

#include "stm32f4xx_hal.h"

//Define the UART buffer size
#define RX_BUFFER_SIZE 100

//Declare global variables to Hold the UART status and data
extern uint8_t rx_buffer[RX_BUFFER_SIZE];
extern uint8_t word_buffer[RX_BUFFER_SIZE];
extern volatile uint8_t rx_index;
extern volatile uint8_t data_received;
extern volatile uint8_t word_index; 


//UART Initialization function 
void UART_Init(void);

//start receiving data via UART
void UART_StartReception(void);

// Transmit a message via UART
void UART_Transmit(const char *message);

//Uart Error handler function
void Error_Handler(void);

//DMA method
char *UART_ProcessBuffer(void);

//polling method
void uart_reception_polling();


#endif
