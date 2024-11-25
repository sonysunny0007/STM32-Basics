#include "uart.h"
#include "stm32f4xx_hal.h"
#include <stdio.h>

// Define the maximum buffer size for reception
#define UART_BUFFER_SIZE 256

// Declare the UART handles for all UARTs
UART_HandleTypeDef huart1;
UART_HandleTypeDef huart2;
UART_HandleTypeDef huart3;
UART_HandleTypeDef huart4;
UART_HandleTypeDef huart5;
UART_HandleTypeDef huart6;

// Declare the UART buffers and indices
uint8_t uart_rx_buffer[UART_BUFFER_SIZE];
uint8_t uart_tx_buffer[UART_BUFFER_SIZE];
volatile uint16_t uart_rx_index = 0;
volatile uint16_t uart_tx_index = 0;

// UART Initialization function
void UART_Init(UART_HandleTypeDef *huart, uint32_t baud_rate)
{
    huart->Instance = USART1; // Set the UART instance (change accordingly for other UARTs)
    huart->Init.BaudRate = baud_rate;
    huart->Init.WordLength = UART_WORDLENGTH_8B;
    huart->Init.StopBits = UART_STOPBITS_1;
    huart->Init.Parity = UART_PARITY_NONE;
    huart->Init.Mode = UART_MODE_TX_RX;
    huart->Init.HwFlowCtl = UART_HWCONTROL_NONE;
    huart->Init.OverSampling = UART_OVERSAMPLING_16;

    if (HAL_UART_Init(huart) != HAL_OK)
    {
        // Initialization Error
        printf("UART Initialization Failed\r\n");
        while (1);
    }

    // Enable UART interrupt (for all UARTs)
    if (huart == &huart1) HAL_NVIC_EnableIRQ(USART1_IRQn);
    if (huart == &huart2) HAL_NVIC_EnableIRQ(USART2_IRQn);
    // if (huart == &huart3) HAL_NVIC_EnableIRQ(USART3_IRQn);
    // if (huart == &huart4) HAL_NVIC_EnableIRQ(UART4_IRQn);
    // if (huart == &huart5) HAL_NVIC_EnableIRQ(UART5_IRQn);
    if (huart == &huart6) HAL_NVIC_EnableIRQ(USART6_IRQn);
}

// UART transmit function (non-blocking)
HAL_StatusTypeDef UART_Transmit_IT(UART_HandleTypeDef *huart, uint8_t *data, uint16_t size)
{
    return HAL_UART_Transmit_IT(huart, data, size);
}

// UART receive function (non-blocking)
HAL_StatusTypeDef UART_Receive_IT(UART_HandleTypeDef *huart, uint8_t *data, uint16_t size)
{
    return HAL_UART_Receive_IT(huart, data, size);
}

// UART IRQ Handlers for all UARTs
void USART1_IRQHandler(void)
{
    HAL_UART_IRQHandler(&huart1);
}

void USART2_IRQHandler(void)
{
    HAL_UART_IRQHandler(&huart2);
}

void USART3_IRQHandler(void)
{
    HAL_UART_IRQHandler(&huart3);
}

void UART4_IRQHandler(void)
{
    HAL_UART_IRQHandler(&huart4);
}

void UART5_IRQHandler(void)
{
    HAL_UART_IRQHandler(&huart5);
}

void USART6_IRQHandler(void)
{
    HAL_UART_IRQHandler(&huart6);
}

// UART Transmission callback (called when a byte is transmitted)
void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart)
{
    uart_tx_index++;  // Update the transmission buffer index
    if (uart_tx_index < UART_BUFFER_SIZE)
    {
        // Start the next byte transmission
        UART_Transmit_IT(huart, &uart_tx_buffer[uart_tx_index], 1);
    }
    else
    {
        // Transmission is complete, reset the index
        uart_tx_index = 0;
    }
}

// UART Reception callback (called when a byte is received)
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
    // Store the received byte in the buffer
    uart_rx_buffer[uart_rx_index++] = huart->Instance->DR;  // Read the data register

    if (uart_rx_index >= UART_BUFFER_SIZE)
    {
        uart_rx_index = 0;  // Reset the buffer index if it overflows
    }

    // Re-enable the UART receive interrupt to continue receiving data
    UART_Receive_IT(huart, &uart_rx_buffer[uart_rx_index], 1);
}
