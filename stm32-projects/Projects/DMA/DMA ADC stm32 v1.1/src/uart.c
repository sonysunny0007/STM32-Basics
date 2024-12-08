#include "uart.h"
#include <stdio.h>
#include <string.h>

//UART handler (decalred statitc to limit the scope to this file)
static UART_HandleTypeDef huart2;

//UART bufffer to store the recived data
uint8_t rx_buffer[RX_BUFFER_SIZE];
uint8_t word_buffer[RX_BUFFER_SIZE];
volatile uint8_t rx_index = 0;
volatile uint8_t data_received = 0;
volatile uint8_t word_index =0;
uint8_t rx_byte;

//Initialize UART peripheral
void UART_Init(void) {

   HAL_NVIC_SetPriority(USART2_IRQn, 0, 0);  // Set priority for USART2 interrupt
   HAL_NVIC_EnableIRQ(USART2_IRQn);          // Enable the interrupt in NVIC
    
    __HAL_RCC_USART2_CLK_ENABLE();
    __HAL_RCC_GPIOA_CLK_ENABLE();

    GPIO_InitTypeDef GPIO_InitStruct = {0};
    GPIO_InitStruct.Pin = GPIO_PIN_2 | GPIO_PIN_3; // PA2 -> TX, PA3 -> RX
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    GPIO_InitStruct.Alternate = GPIO_AF7_USART2;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

    huart2.Instance = USART2;
    huart2.Init.BaudRate = 115200;
    huart2.Init.WordLength = UART_WORDLENGTH_8B;
    huart2.Init.StopBits = UART_STOPBITS_1;
    huart2.Init.Parity = UART_PARITY_NONE;
    huart2.Init.Mode = UART_MODE_TX_RX;
    huart2.Init.HwFlowCtl = UART_HWCONTROL_NONE;
    huart2.Init.OverSampling = UART_OVERSAMPLING_16;

    if (HAL_UART_Init(&huart2) != HAL_OK) {
        Error_Handler();
    }
}

//polling method
void uart_reception_polling(){

    if(HAL_UART_Receive(&huart2, rx_buffer,RX_BUFFER_SIZE, HAL_MAX_DELAY)==HAL_OK){
        HAL_UART_Transmit(&huart2, rx_buffer, strlen((char*)rx_buffer), HAL_MAX_DELAY);
    }
}


// Start UART reception in interrupt mode
void UART_StartReception(void) {
    HAL_UART_Receive_IT(&huart2, &rx_byte, 1);  // Start interrupt-based reception
}



// Callback function for UART RX Complete
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart) {
    if (huart->Instance == USART2) {

            // Check if the received byte is a newline character or carriage return
        if (rx_index < RX_BUFFER_SIZE - 1) {
            rx_buffer[rx_index++] = rx_byte;  // Store the byte in the buffer

        // Check if the received byte is a newline character or carriage return
        if (rx_byte == '\n' || rx_byte == '\r') {
            // Null-terminate the string at the current index
            rx_buffer[rx_index] = '\0';  // Null-terminate string

            // Debug: Print the received data before resetting
            //printf("Received Data: %s\r\n", rx_buffer); // Verify content of rx_buffer

            data_received = 1;  // Set flag when newline is received

            // Reset buffer index for the next message
            rx_index = 0;
        }
        } else {
            // Handle buffer overflow (optional)
            rx_index = 0;  // Reset index if buffer overflows
        }

        // Restart UART reception interrupt for the next byte
        HAL_UART_Receive_IT(&huart2, &rx_byte, 1);  
    }
}

//UART Interrupt handler
void USART2_IRQHandler(void) {
    HAL_UART_IRQHandler(&huart2);
}

//DMA method
char *UART_ProcessBuffer(void) {
    // Loop through the rx_buffer to process received bytes
    for (int i = 0; i < RX_BUFFER_SIZE; i++) {
        // Check if the character is a newline or carriage return
        if (rx_buffer[i] == '\n' || rx_buffer[i] == '\r') {
            // Transmit the full word stored in word_buffer
            HAL_UART_Transmit(&huart2, word_buffer, word_index, HAL_MAX_DELAY);

            // Null-terminate the word_buffer string
            word_buffer[word_index] = '\0';

            // Clear the rx_buffer character
            rx_buffer[i] = 0;

            // Reset rx_index and word_index
            rx_index = 0;
            word_index = 0;

            // Return the pointer to the processed word
            return (char *)word_buffer;
        } else {
            // Store the received character in word_buffer if not full
            if (word_index < RX_BUFFER_SIZE - 1) {
                word_buffer[word_index++] = rx_buffer[i];
            }

            // Clear the character in the rx_buffer to avoid repeat processing
            rx_buffer[i] = 0;
        }
    }

    // Null-terminate the word_buffer in case no newline is found
    word_buffer[word_index] = '\0';
    return (char *)word_buffer;
}


//printf function
int _write(int file, char *data, int len) {
    HAL_UART_Transmit(&huart2, (uint8_t*)data, len, HAL_MAX_DELAY);
    return len;
}

// Transmit a string via UART
void UART_Transmit(const char *message) {
    HAL_UART_Transmit(&huart2, (uint8_t *)message, strlen(message), HAL_MAX_DELAY);
}

void Error_Handler(void) {
    while (1) {
        // HAL_GPIO_TogglePin(GPIOA, GPIO_PIN_6);
        // HAL_Delay(500);
    }
}