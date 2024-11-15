#include "stm32f4xx_hal.h"
#include <stdio.h>
#include <string.h>

void SystemClock_Config(void);
void Configure_PLL(void);
void UART_Init(void);
void Start_UART_Reception(void);
void GPIO_Init(void);
void Error_Handler(void);

// UART Handle
UART_HandleTypeDef huart2;

// Buffer to store received data
#define RX_BUFFER_SIZE 100
uint8_t rx_buffer[RX_BUFFER_SIZE];
uint8_t rx_byte;
volatile uint8_t rx_index = 0;
volatile uint8_t data_received = 0;
char msg[50];

int main(void)
{
    // Initialize the HAL Library
    HAL_Init();

    // Configure system clock
    SystemClock_Config();

    // Initialize GPIO and UART
    GPIO_Init();

    HAL_NVIC_SetPriority(USART2_IRQn, 0, 0);  // Set priority for USART2 interrupt
    HAL_NVIC_EnableIRQ(USART2_IRQn);          // Enable the interrupt in NVIC

    UART_Init();

    //start uart reception
   Start_UART_Reception();

   // Initial boot message
    HAL_UART_Transmit(&huart2, (uint8_t*)"System booting...\r\n", 19, HAL_MAX_DELAY);

    while (1) {
        if (data_received) {
            HAL_UART_Transmit(&huart2, (uint8_t*)"Data received: ", 15, HAL_MAX_DELAY);
            HAL_UART_Transmit(&huart2, rx_buffer, strlen((char*)rx_buffer), HAL_MAX_DELAY);
            HAL_UART_Transmit(&huart2, (uint8_t*)"\r\n", 2, HAL_MAX_DELAY);

            data_received = 0;  // Reset the flag
            rx_index = 0;       // Reset buffer index
        } else {
            // Heartbeat message to check loop execution
            HAL_UART_Transmit(&huart2, (uint8_t*)"Waiting for UART data...\r\n", 27, HAL_MAX_DELAY);
            HAL_Delay(10000);
        }
    }
}
   

void SystemClock_Config(void) {
    // Enable HSE oscillator
    RCC->CR |= RCC_CR_HSEON;
    while (!(RCC->CR & RCC_CR_HSERDY)); // Wait for HSE ready

    // Configure PLL to use HSE as the source and set up for 168 MHz
    Configure_PLL();

    // Set Flash latency for 168 MHz
    FLASH->ACR |= FLASH_ACR_LATENCY_5WS;

    // Switch system clock to PLL
    RCC->CFGR &= ~RCC_CFGR_SW;            // Clear system clock switch bits
    RCC->CFGR |= RCC_CFGR_SW_PLL;          // Select PLL as system clock
    while ((RCC->CFGR & RCC_CFGR_SWS) != RCC_CFGR_SWS_PLL);  // Wait until PLL is used as system clock

    SystemCoreClock = 168000000;  // Update SystemCoreClock to 168 MHz
    Configure_PLL();
}

void Configure_PLL(void) {
    // Set PLL source to HSE
    RCC->PLLCFGR |= RCC_PLLCFGR_PLLSRC_HSE; // Select HSE as PLL source

    // Configure PLLM, PLLN, and PLLP to achieve 168 MHz system clock
    RCC->PLLCFGR &= ~RCC_PLLCFGR_PLLM;             // Clear PLLM bits
    RCC->PLLCFGR |= (8 << RCC_PLLCFGR_PLLM_Pos);   // Set PLLM to 8 (8 MHz HSE / 8 = 1 MHz)

    RCC->PLLCFGR &= ~RCC_PLLCFGR_PLLN;             // Clear PLLN bits
    RCC->PLLCFGR |= (336 << RCC_PLLCFGR_PLLN_Pos); // Set PLLN to 336 (1 MHz * 336 = 336 MHz)

    RCC->PLLCFGR &= ~RCC_PLLCFGR_PLLP;             // Clear PLLP bits
    RCC->PLLCFGR |= (0 << RCC_PLLCFGR_PLLP_Pos);   // Set PLLP to 2 (336 MHz / 2 = 168 MHz)

    // Enable PLL
    RCC->CR |= RCC_CR_PLLON;                       // Turn on PLL
    while (!(RCC->CR & RCC_CR_PLLRDY));            // Wait for PLL to be ready
}


void UART_Init(void) {
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

void GPIO_Init(void) {
    __HAL_RCC_GPIOA_CLK_ENABLE();
    GPIO_InitTypeDef GPIO_InitStruct = {0};

    // Configure LED on PA6
    GPIO_InitStruct.Pin = GPIO_PIN_6;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_6, GPIO_PIN_RESET);
}

void Error_Handler(void) {
    while (1) {
        HAL_GPIO_TogglePin(GPIOA, GPIO_PIN_6);
        HAL_Delay(500);
    }
}

void Start_UART_Reception(void) {
    HAL_UART_Receive_IT(&huart2, &rx_byte, 1);  // Start interrupt-based reception
}

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart) {
    if (huart->Instance == USART2) {
        // Debug message to verify callback functionality
 //       HAL_UART_Transmit(&huart2, (uint8_t*)"Callback triggered: ", 19, HAL_MAX_DELAY);
        //printf("%s", rx_buffer);
        if (rx_index < RX_BUFFER_SIZE - 1) {
            rx_buffer[rx_index++] = rx_byte;
            if (rx_byte == '\n') {
                data_received = 1;  // Set flag when newline is received
                rx_buffer[rx_index] = '\0';  // Null-terminate the string
                rx_index = 0;  // Reset buffer index for next message

                // Debug message when data is fully received
                HAL_UART_Transmit(&huart2, (uint8_t*)"Message complete\r\n", 18, HAL_MAX_DELAY);
            }
        } else {
            rx_index = 0;  // Reset if buffer overflows
        }
        HAL_UART_Receive_IT(&huart2, &rx_byte, 1);  // Restart reception
        
    }
}

void USART2_IRQHandler(void) {
    HAL_UART_IRQHandler(&huart2);
}

int _write(int file, char *data, int len) {
    HAL_UART_Transmit(&huart2, (uint8_t*)data, len, HAL_MAX_DELAY);
    return len;
}

void SysTick_Handler(void) {
    HAL_IncTick();
}