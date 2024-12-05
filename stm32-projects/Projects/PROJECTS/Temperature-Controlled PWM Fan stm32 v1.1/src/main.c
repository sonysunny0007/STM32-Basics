#include "stm32f4xx_hal.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "uart.h"
#include "sysclock.h"
#include "gpio.h"
#include "math.h"
#include "timer.h"

//echo -e "Sony Sunny" > /dev/tty.usbserial-0001    //use this code in minicom to send the data

// Global Variables
ADC_HandleTypeDef hadc1;
TIM_HandleTypeDef htim3;
UART_HandleTypeDef huart2;
DMA_HandleTypeDef hdma_adc1;
uint32_t adc_buffer[1];         // ADC buffer
char uart_buffer[50];           // UART buffer

#define ADC_BUF_LEN 1
volatile uint32_t adc_buf[ADC_BUF_LEN]; // ADC buffer

// Function Prototypes
void MX_ADC_Init(void);
void MX_GPIO_Init(void);
void MX_TIM_Init(void);
void DMA_Init(void);
void PWM_SetDutyCycle(uint8_t duty);
void HAL_TIM_MspPostInit(TIM_HandleTypeDef *htim);

int main(void) {
    // Initialize the HAL Library
    HAL_Init();

    // Configure system clock
    SystemClock_Config();

    // Initialize UART
    UART_Init();

    // Initial boot message
    UART_Transmit("****************************************************************\r\n");
    UART_Transmit("System Booting.... #Temperature Controlled Motor DMA Project#\r\n");

    // Print system clock frequency
    uint32_t sysclk = Get_SYSCLK_Frequency();
    printf("System clock frequency: %lu MHz\r\n", sysclk / 1000000);

    HAL_Delay(2000);  // Adding 2s delay

    // Configure GPIOs
    MX_GPIO_Init();

    // Initialize DMA, ADC, and TIM
    DMA_Init();
    MX_ADC_Init();
    MX_TIM_Init();

    // Start PWM
    HAL_TIM_PWM_Start_IT(&htim3, TIM_CHANNEL_1);

    // Start ADC in DMA mode
    HAL_ADC_Start_DMA(&hadc1, (uint32_t *)adc_buf, ADC_BUF_LEN);

    while (1) {
        // Convert ADC to voltage
        float voltage = (adc_buf[0] * 3.3) / 4095;
        snprintf(uart_buffer, sizeof(uart_buffer), "Voltage: %.2f V\r\n", voltage);
        UART_Transmit(uart_buffer);

        // Set PWM duty cycle based on voltage
        PWM_SetDutyCycle((uint8_t)((adc_buf[0] * 100) / 4095));

        HAL_Delay(500);
    }
}

void MX_GPIO_Init(void) {
    // Enable clocks for GPIO ports
    __HAL_RCC_GPIOA_CLK_ENABLE();
    __HAL_RCC_GPIOB_CLK_ENABLE();

    // Configure GPIO PA0 for analog input (ADC)
    GPIO_Init('A', GPIO_PIN_0, GPIO_MODE_ANALOG, GPIO_NOPULL, NULL, NULL);

    // Configure GPIO PB4 for alternate function (PWM output for TIM3_CH1)
    // GPIO_Init('B', GPIO_PIN_4, GPIO_MODE_AF_PP, GPIO_NOPULL, GPIO_SPEED_FREQ_LOW, GPIO_AF2_TIM3);
}

void DMA_Init(void) {
    __HAL_RCC_DMA2_CLK_ENABLE();

    hdma_adc1.Instance = DMA2_Stream0;
    hdma_adc1.Init.Channel = DMA_CHANNEL_0;
    hdma_adc1.Init.Direction = DMA_PERIPH_TO_MEMORY;
    hdma_adc1.Init.PeriphInc = DMA_PINC_DISABLE;
    hdma_adc1.Init.MemInc = DMA_MINC_ENABLE;
    hdma_adc1.Init.PeriphDataAlignment = DMA_PDATAALIGN_WORD;
    hdma_adc1.Init.MemDataAlignment = DMA_MDATAALIGN_WORD;
    hdma_adc1.Init.Mode = DMA_CIRCULAR;
    hdma_adc1.Init.Priority = DMA_PRIORITY_LOW;
    hdma_adc1.Init.FIFOMode = DMA_FIFOMODE_DISABLE;

    if (HAL_DMA_Init(&hdma_adc1) != HAL_OK) {
        // Error Handling
        while (1);
    }

    __HAL_LINKDMA(&hadc1, DMA_Handle, hdma_adc1);
}

void MX_ADC_Init(void) {
    __HAL_RCC_ADC1_CLK_ENABLE();

    hadc1.Instance = ADC1;
    hadc1.Init.Resolution = ADC_RESOLUTION_12B;
    hadc1.Init.ScanConvMode = DISABLE;
    hadc1.Init.ContinuousConvMode = DISABLE;
    hadc1.Init.DiscontinuousConvMode = DISABLE;
    hadc1.Init.ExternalTrigConv = ADC_EXTERNALTRIGCONV_T3_TRGO;
    hadc1.Init.DataAlign = ADC_DATAALIGN_RIGHT;
    hadc1.Init.NbrOfConversion = 1;

    if (HAL_ADC_Init(&hadc1) != HAL_OK) {
        // Error Handling
        while (1);
    }

    ADC_ChannelConfTypeDef sConfig = {0};
    sConfig.Channel = ADC_CHANNEL_0;
    sConfig.Rank = 1;
    sConfig.SamplingTime = ADC_SAMPLETIME_3CYCLES;

    if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK) {
        // Error Handling
        while (1);
    }
}

void MX_TIM_Init(void) {
    __HAL_RCC_TIM3_CLK_ENABLE();

    htim3.Instance = TIM3;
    htim3.Init.Prescaler = 16799;
    htim3.Init.CounterMode = TIM_COUNTERMODE_UP;
    htim3.Init.Period = 999;
    htim3.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
    htim3.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_ENABLE;

    if (HAL_TIM_PWM_Init(&htim3) != HAL_OK) {
        // Error Handling
        while (1);
    }

    TIM_OC_InitTypeDef sConfig = {0};
    sConfig.OCMode = TIM_OCMODE_PWM1;
    sConfig.Pulse = 0;
    sConfig.OCPolarity = TIM_OCPOLARITY_HIGH;
    sConfig.OCFastMode = TIM_OCFAST_DISABLE;

    if (HAL_TIM_PWM_ConfigChannel(&htim3, &sConfig, TIM_CHANNEL_1) != HAL_OK) {
        // Error Handling
        while (1);
    }

    HAL_TIM_MspPostInit(&htim3);
}

void HAL_TIM_MspPostInit(TIM_HandleTypeDef *htim) {
    GPIO_InitTypeDef GPIO_InitStruct = {0};

    if (htim->Instance == TIM3) {
        // Enable GPIOB clock
        __HAL_RCC_GPIOB_CLK_ENABLE();

        // Configure PB4 as TIM3_CH1 (Alternate Function: PWM output)
        GPIO_InitStruct.Pin = GPIO_PIN_4;
        GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
        GPIO_InitStruct.Pull = GPIO_NOPULL;
        GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
        GPIO_InitStruct.Alternate = GPIO_AF2_TIM3;
        HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
    }
}

void PWM_SetDutyCycle(uint8_t duty) {
    if (duty > 100) duty = 100;
    __HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_1, duty);
}

void SysTick_Handler(void) {
    HAL_IncTick();
}
