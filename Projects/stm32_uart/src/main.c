#include "stm32f4xx_hal.h"
#include "string.h"

// Define UART handle for UART2
UART_HandleTypeDef huart2;

// Function to initialize UART2
void UART2_Init(void) {
    huart2.Instance = USART2;                       // Use UART2 instance
    huart2.Init.BaudRate = 9600;                    // Set baud rate to 9600
    huart2.Init.WordLength = UART_WORDLENGTH_8B;    // Set word length to 8 bits
    huart2.Init.StopBits = UART_STOPBITS_1;         // Set stop bit to 1
    huart2.Init.Parity = UART_PARITY_NONE;          // No parity
    huart2.Init.Mode = UART_MODE_TX_RX;             // Enable both TX and RX
    huart2.Init.HwFlowCtl = UART_HWCONTROL_NONE;    // No hardware flow control
    huart2.Init.OverSampling = UART_OVERSAMPLING_16; // Oversampling by 16

    if (HAL_UART_Init(&huart2) != HAL_OK) {
        // Initialization Error
        while (1);
    }
}

// Function to transmit a message
void UART_SendString(char* message) {
    HAL_UART_Transmit(&huart2, (uint8_t*)message, strlen(message), HAL_MAX_DELAY);
}

// Function to initialize system clock
void SystemClock_Config(void) {
    RCC_OscInitTypeDef RCC_OscInitStruct = {0};
    RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

    RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
    RCC_OscInitStruct.HSEState = RCC_HSE_ON;
    RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
    RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
    RCC_OscInitStruct.PLL.PLLM = 8;
    RCC_OscInitStruct.PLL.PLLN = 336;
    RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
    RCC_OscInitStruct.PLL.PLLQ = 7;
    HAL_RCC_OscConfig(&RCC_OscInitStruct);

    RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK |
                                  RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2;
    RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
    RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
    RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV4;
    RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;

    HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_5);
}

// Main function
int main(void) {
    HAL_Init();                  // Initialize the HAL library
    SystemClock_Config();         // Configure the system clock
    UART2_Init();                 // Initialize UART2

    // Main loop
    while (1) {
        UART_SendString("Hello, UART!\r\n");   // Send a test message
        HAL_Delay(1000);                       // Delay 1 second
    }
}

// Override the SysTick Handler
void SysTick_Handler(void) {
    HAL_IncTick();
}