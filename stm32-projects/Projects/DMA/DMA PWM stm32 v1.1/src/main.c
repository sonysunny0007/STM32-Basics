#include "stm32f4xx_hal.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "uart.h"
#include "sysclock.h"
#include "gpio.h"
#include "math.h"
#include "timer.h"

// GPIO Pin configuration
#define GPIO_PIN GPIO_PIN_5  // Using GPIOA Pin 5 (LED)

// Global variables for Timer and DMA
TIM_HandleTypeDef htim2;        // Timer handle
DMA_HandleTypeDef hdma_tim2_up; // DMA handle

uint32_t led_data[] = {
    GPIO_PIN,          // Set GPIO_PIN_5 (Lower 16 bits in BSRR)
    GPIO_PIN << 16     // Clear GPIO_PIN_5 (Upper 16 bits in BSRR)
};

void MX_GPIO_Init(void);
void TIM_Init(void);
void DMA_Init(void);

int main(void) {
    HAL_Init();
    SystemClock_Config();
    MX_GPIO_Init();
    TIM_Init();
    DMA_Init();
    UART_Init();

    printf("DMA timer project \r\n");

    // Start Timer
    if (HAL_TIM_Base_Start(&htim2) != HAL_OK) {
        printf("Timer Start Error\r\n");
        while (1);
    }

    
    if (HAL_DMA_Start(&hdma_tim2_up, (uint32_t)led_data, (uint32_t)&GPIOA->BSRR, 2) != HAL_OK) {
        printf("DMA Start Error\r\n");
        while (1);
    }

    // Link DMA to Timer Update Event
    __HAL_TIM_ENABLE_DMA(&htim2, TIM_DMA_UPDATE);

    while (1) {
        printf("Timer is running \r\n");
        HAL_Delay(1000);
    }
}

void MX_GPIO_Init(void) {
    __HAL_RCC_GPIOA_CLK_ENABLE();

    GPIO_InitTypeDef GPIO_InitStruct = {0};
    GPIO_InitStruct.Pin = GPIO_PIN;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
}

void TIM_Init(void) {
    __HAL_RCC_TIM2_CLK_ENABLE();

    htim2.Instance = TIM2;
    htim2.Init.Prescaler = 15999;  // Prescaler to get 1 kHz timer frequency (16 MHz / 16000)
    htim2.Init.CounterMode = TIM_COUNTERMODE_UP;
    htim2.Init.Period = 999;       // Overflow every 100 ms
    htim2.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
    htim2.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;

    if (HAL_TIM_Base_Init(&htim2) != HAL_OK) {
        printf("Timer initialization error\r\n");
        while (1);
    }

    TIM_ClockConfigTypeDef sClockSourceConfig = {0};
    TIM_MasterConfigTypeDef sMasterConfig = {0};

    sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
    if (HAL_TIM_ConfigClockSource(&htim2, &sClockSourceConfig) != HAL_OK) {
        printf("Clock Source error\r\n");   
        while (1);
    }

    sMasterConfig.MasterOutputTrigger = TIM_TRGO_UPDATE;
    sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
    if (HAL_TIMEx_MasterConfigSynchronization(&htim2, &sMasterConfig) != HAL_OK) {
        // Master configuration error
        while (1);
    }
}


void DMA_Init(void) {
    __HAL_RCC_DMA1_CLK_ENABLE();

    hdma_tim2_up.Instance = DMA1_Stream1;
    hdma_tim2_up.Init.Channel = DMA_CHANNEL_3;
    hdma_tim2_up.Init.Direction = DMA_MEMORY_TO_PERIPH;
    hdma_tim2_up.Init.PeriphInc = DMA_PINC_DISABLE;
    hdma_tim2_up.Init.MemInc = DMA_MINC_ENABLE;
    hdma_tim2_up.Init.PeriphDataAlignment = DMA_PDATAALIGN_WORD;
    hdma_tim2_up.Init.MemDataAlignment = DMA_MDATAALIGN_WORD;
    hdma_tim2_up.Init.Mode = DMA_CIRCULAR;
    hdma_tim2_up.Init.Priority = DMA_PRIORITY_LOW;
    hdma_tim2_up.Init.FIFOMode = DMA_FIFOMODE_DISABLE;

    if (HAL_DMA_Init(&hdma_tim2_up) != HAL_OK) {
        printf("DMA initialization error\r\n");
        while (1);
    }

    // Link DMA handle to Timer Update Event
    __HAL_LINKDMA(&htim2, hdma[TIM_DMA_ID_UPDATE], hdma_tim2_up);
}

// System Tick Handler
void SysTick_Handler(void) {
    HAL_IncTick();
}
