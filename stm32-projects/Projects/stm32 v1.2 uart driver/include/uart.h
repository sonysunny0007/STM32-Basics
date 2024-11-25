#ifndef __UART_H
#define __UART_H

#include "stm32f4xx_hal.h"

// Define the maximum buffer size for reception
#define UART_BUFFER_SIZE 256

// Declare the UART handle
extern UART_HandleTypeDef huart1;
extern UART_HandleTypeDef huart2;
extern UART_HandleTypeDef huart3;
extern UART_HandleTypeDef huart4;
extern UART_HandleTypeDef huart5;
extern UART_HandleTypeDef huart6;

// Buffer for UART reception and transmission
extern uint8_t uart_rx_buffer[UART_BUFFER_SIZE];
extern uint8_t uart_tx_buffer[UART_BUFFER_SIZE];
extern volatile uint16_t uart_rx_index;
extern volatile uint16_t uart_tx_index;

// UART Initialization function
void UART_Init(UART_HandleTypeDef *huart, uint32_t baud_rate);

// UART transmit function (non-blocking)
HAL_StatusTypeDef UART_Transmit_IT(UART_HandleTypeDef *huart, uint8_t *data, uint16_t size);

// UART receive function (non-blocking)
HAL_StatusTypeDef UART_Receive_IT(UART_HandleTypeDef *huart, uint8_t *data, uint16_t size);

// UART IRQ Handlers
void USART1_IRQHandler(void);
void USART2_IRQHandler(void);
void USART3_IRQHandler(void);
void UART4_IRQHandler(void);
void UART5_IRQHandler(void);
void USART6_IRQHandler(void);

// UART Transmission callback
void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart);

// UART Reception callback
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart);

#endif /* __UART_H */
