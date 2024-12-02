#include "stm32f4xx_hal.h"
#include <string.h>
#include <stdio.h>
#include "uart.h"
#include "sysclock.h"
#include "gpio.h"

// Define global variables
TIM_HandleTypeDef htim2;  // Timer 2 handle
UART_HandleTypeDef huart2; // UART 2 handle
volatile uint32_t pulse_count = 0;  // Tracks the total number of pulses

// Function prototypes
void MX_TIM2_Init(void);
void HAL_TIM_IC_CaptureCallback(TIM_HandleTypeDef *htim);

int main(void) {
    // Initialize the HAL Library
    HAL_Init();

    // Configure the system clock
    SystemClock_Config();

    // Initialize UART
    UART_Init();

    // Print initial boot message
    UART_Transmit("************************************************************\r\n");
    UART_Transmit("System Booting... #Pulse Counter Project#\r\n");

    // Print system clock frequency
    uint32_t sysclk = Get_SYSCLK_Frequency();
    char uart_buffer[100];
    snprintf(uart_buffer, sizeof(uart_buffer), "System clock frequency: %lu MHz\r\n", sysclk / 1000000);
    UART_Transmit(uart_buffer);

    HAL_Delay(2000);  // Add a 2-second delay

    // Initialize Timer 2 for input capture
    MX_TIM2_Init();

    // Configure GPIO for input capture (PA0 for TIM2_CH1)
    __HAL_RCC_GPIOA_CLK_ENABLE();
    GPIO_Init('A', GPIO_PIN_0, GPIO_MODE_AF_PP, GPIO_PULLUP, GPIO_SPEED_FREQ_HIGH, GPIO_AF1_TIM2);

    // Configure GPIO for LED (PA6)
    GPIO_Init('A', GPIO_PIN_6, GPIO_MODE_OUTPUT_PP, GPIO_NOPULL, GPIO_SPEED_FREQ_HIGH, NULL);

    // Start input capture in interrupt mode
    if(HAL_TIM_PWM_Start(&htim2, TIM_CHANNEL_1) != HAL_OK){
        printf("Failed to start PWM\r\n");
    };

    // Main loop
    while (1) {
        snprintf(uart_buffer, sizeof(uart_buffer), "Pulse count: %lu\r\n", pulse_count);
        UART_Transmit(uart_buffer);

        HAL_GPIO_WritePin(GPIOA, GPIO_PIN_6, GPIO_PIN_SET);  // Turn on LED
        HAL_Delay(1000);
        HAL_GPIO_WritePin(GPIOA, GPIO_PIN_6, GPIO_PIN_RESET);  // Turn off LED
        HAL_Delay(1000);
    }
}

void MX_TIM2_Init(void) {
    __HAL_RCC_TIM2_CLK_ENABLE();

    htim2.Instance = TIM2;
    htim2.Init.Prescaler = 0;  // No prescaler, use full timer clock
    htim2.Init.CounterMode = TIM_COUNTERMODE_UP;
    htim2.Init.Period = 0xFFFF;  // Maximum period for 16-bit timer
    htim2.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;

    if (HAL_TIM_IC_Init(&htim2) != HAL_OK) {
        while (1);  // Initialization error
    }

    TIM_IC_InitTypeDef sConfig = {0};
    sConfig.ICPolarity = TIM_INPUTCHANNELPOLARITY_RISING;  // Capture on rising edge
    sConfig.ICSelection = TIM_ICSELECTION_DIRECTTI;  // Direct input
    sConfig.ICPrescaler = TIM_ICPSC_DIV1;  // No input prescaler
    sConfig.ICFilter = 0;  // No input filter

    if (HAL_TIM_IC_ConfigChannel(&htim2, &sConfig, TIM_CHANNEL_1) != HAL_OK) {
        while (1);  // Configuration error
    }
}

void HAL_TIM_IC_CaptureCallback(TIM_HandleTypeDef *htim) {
    if (htim->Instance == TIM2 && htim->Channel == HAL_TIM_ACTIVE_CHANNEL_1) {
        pulse_count++;  // Increment pulse count on each rising edge
    }
}

void SysTick_Handler(void) {
    HAL_IncTick();
}
