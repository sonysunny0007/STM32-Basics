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
int status=0;

// Function Prototypes
void DMA_Init(void);
void TIM2_Init(void);
void Debug_TimerStatus(void);
void Debug_DMAStatus(void);

int main(void) {
    HAL_Init();
    SystemClock_Config();  // Set system clock to 168 MHz

    TIM2_Init();
    DMA_Init();
    UART_Init();

    uint32_t data[2] = {
        GPIO_PIN_5,          // Set GPIO_PIN_5
        GPIO_PIN_5 << 16     // Clear GPIO_PIN_5
    };

    printf("System Rebooting....\r\n");

    // Start DMA
    HAL_DMA_Start(&hdma_tim2_up, (uint32_t)data, (uint32_t)&GPIOA->BSRR, 2);
        if (status == HAL_OK) {
            printf("DMA Transfer Complete\r\n");
        } else {
            printf("DMA Transfer Error: %d\r\n", status);
        }
    

    // Enable DMA requests on timer update
    __HAL_TIM_ENABLE_DMA(&htim2, TIM_DMA_UPDATE);

    // Start timer
    printf("Attempting to start timer...\r\n");
    if (HAL_TIM_Base_Start(&htim2) != HAL_OK) {
        printf("Timer Start Error\r\n");
        while (1);
    } else {
        printf("Timer Started Successfully\r\n");
    }

    printf("Timer and DMA initialized successfully\r\n");

    while (1) {
        if (printFlag) {
            printFlag = 0;
            Debug_TimerStatus();
            Debug_DMAStatus();
            HAL_Delay(10000);
        }
    }
}

void TIM2_Init(void) {
    __HAL_RCC_TIM2_CLK_ENABLE();

    htim2.Instance = TIM2;
    htim2.Init.Prescaler = 8399;  // Adjust for required timing
    htim2.Init.CounterMode = TIM_COUNTERMODE_UP;
    htim2.Init.Period = 999;  // Adjust for overflow
    htim2.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
    htim2.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;

    if (HAL_TIM_Base_Init(&htim2) != HAL_OK) {
        printf("Timer Initialization Error\r\n");
        while (1);
    }

    // Configure Timer for DMA Update
    TIM_MasterConfigTypeDef sMasterConfig = {0};
    sMasterConfig.MasterOutputTrigger = TIM_TRGO_UPDATE;
    sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
    if (HAL_TIMEx_MasterConfigSynchronization(&htim2, &sMasterConfig) != HAL_OK) {
        printf("Timer Master Config Error\r\n");
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
        printf("DMA Initialization Error\r\n");
        while (1);
    }

    __HAL_LINKDMA(&htim2, hdma[TIM_DMA_ID_UPDATE], hdma_tim2_up);

    HAL_NVIC_SetPriority(DMA1_Stream1_IRQn, 0, 0);
    HAL_NVIC_EnableIRQ(DMA1_Stream1_IRQn);
}

void Debug_TimerStatus(void) {
    if (__HAL_TIM_GET_FLAG(&htim2, TIM_FLAG_UPDATE) != RESET) {
        printf("Timer Update Event Triggered\r\n");
        __HAL_TIM_CLEAR_FLAG(&htim2, TIM_FLAG_UPDATE);
    }
}

void Debug_DMAStatus(void) {
    if (__HAL_DMA_GET_FLAG(&hdma_tim2_up, DMA_FLAG_TCIF1_5) != RESET) {
        printf("DMA Transfer Complete\r\n");
        __HAL_DMA_CLEAR_FLAG(&hdma_tim2_up, DMA_FLAG_TCIF1_5);
    }

    if (__HAL_DMA_GET_FLAG(&hdma_tim2_up, DMA_FLAG_HTIF1_5) != RESET) {
        printf("DMA Half-Transfer Complete\r\n");
        __HAL_DMA_CLEAR_FLAG(&hdma_tim2_up, DMA_FLAG_HTIF1_5);
    }

    if (__HAL_DMA_GET_FLAG(&hdma_tim2_up, DMA_FLAG_TEIF1_5) != RESET) {
        printf("DMA Transfer Error\r\n");
        __HAL_DMA_CLEAR_FLAG(&hdma_tim2_up, DMA_FLAG_TEIF1_5);
    }
}

void SysTick_Handler(void) {
    HAL_IncTick();
    printFlag = 1;
}
