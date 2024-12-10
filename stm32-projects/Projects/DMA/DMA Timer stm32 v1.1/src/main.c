#include "stm32f4xx_hal.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "uart.h"
#include "sysclock.h"
#include "gpio.h"
#include "math.h"
#include "timer.h"

// Global Variables
TIM_HandleTypeDef htim2;
DMA_HandleTypeDef hdma_tim2_up;
char uart_buffer[50];
uint32_t adc_buf[1];
volatile uint32_t printFlag = 0;  // Make it volatile for proper interrupt handling

// Function Prototypes
void DMA_Init(void);
void TIM2_Init(void);

int main(void) {
    HAL_Init();
    SystemClock_Config();  // Set system clock to 168 MHz

    TIM2_Init();
    DMA_Init();

    uint32_t data[2] = {
        GPIO_PIN_5,          // Set GPIO_PIN_5
        GPIO_PIN_5 << 16     // Clear GPIO_PIN_5
    };

    // Start DMA
    if (HAL_DMA_Start(&hdma_tim2_up, (uint32_t)data, (uint32_t)&GPIOA->BSRR, 2) != HAL_OK) {
        // Handle DMA start error
        while (1);
    }

    // Enable DMA requests on timer update
    __HAL_TIM_ENABLE_DMA(&htim2, TIM_DMA_UPDATE);

    // Start timer
    if (HAL_TIM_Base_Start(&htim2) != HAL_OK) {
        // Handle timer start error
        while (1);
    }

    while (1) {
        // Main loop
    }
}

void TIM2_Init(void) {
    __HAL_RCC_TIM2_CLK_ENABLE();

    TIM_HandleTypeDef htim2;
    htim2.Instance = TIM2;
    htim2.Init.Prescaler = 15999;  // Adjust for required timing
    htim2.Init.CounterMode = TIM_COUNTERMODE_UP;
    htim2.Init.Period = 999;  // Adjust for overflow
    htim2.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
    htim2.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;

    if (HAL_TIM_Base_Init(&htim2) != HAL_OK) {
        // Timer initialization error
        while (1);
    }

    // Configure Timer for DMA Update
    TIM_MasterConfigTypeDef sMasterConfig = {0};
    sMasterConfig.MasterOutputTrigger = TIM_TRGO_UPDATE;
    sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
    if (HAL_TIMEx_MasterConfigSynchronization(&htim2, &sMasterConfig) != HAL_OK) {
        // Master configuration error
        while (1);
    }
}


void DMA_Init(void) {
    // Enable DMA1 Clock
    __HAL_RCC_DMA1_CLK_ENABLE();

    // Configure DMA1 Stream1 (TIM2_UP, Channel 3)
    hdma_tim2_up.Instance = DMA1_Stream1;  // TIM2_UP uses Stream 1
    hdma_tim2_up.Init.Channel = DMA_CHANNEL_3;  // TIM2_UP is on Channel 3
    hdma_tim2_up.Init.Direction = DMA_MEMORY_TO_PERIPH;  // Memory to Peripheral
    hdma_tim2_up.Init.PeriphInc = DMA_PINC_DISABLE;  // Peripheral address doesn't increment
    hdma_tim2_up.Init.MemInc = DMA_MINC_ENABLE;  // Memory address increments
    hdma_tim2_up.Init.PeriphDataAlignment = DMA_PDATAALIGN_WORD;  // Peripheral data alignment: Word
    hdma_tim2_up.Init.MemDataAlignment = DMA_MDATAALIGN_WORD;  // Memory data alignment: Word
    hdma_tim2_up.Init.Mode = DMA_CIRCULAR;  // Circular mode for continuous transfer
    hdma_tim2_up.Init.Priority = DMA_PRIORITY_LOW;  // Low priority
    hdma_tim2_up.Init.FIFOMode = DMA_FIFOMODE_DISABLE;  // FIFO mode disabled

    // Initialize DMA
    if (HAL_DMA_Init(&hdma_tim2_up) != HAL_OK) {
        // Handle initialization error
        while (1);
    }
    // Link DMA to TIM2 Update Event
    __HAL_LINKDMA(&htim2, hdma[TIM_DMA_ID_UPDATE], hdma_tim2_up);

    // Enable DMA interrupt and set priority
    HAL_NVIC_SetPriority(DMA2_Stream0_IRQn, 0, 0);
    HAL_NVIC_EnableIRQ(DMA2_Stream0_IRQn);
}



void SysTick_Handler(void) {
    HAL_IncTick();
}
