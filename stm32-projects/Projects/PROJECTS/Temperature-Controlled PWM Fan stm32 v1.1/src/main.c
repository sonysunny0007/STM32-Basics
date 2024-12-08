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
void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef *hadc);

int main(void) {
    HAL_Init();                  // Initialize HAL Library
    SystemClock_Config();        // Configure system clock
   

    UART_Transmit("System Booting... ADC + PWM Project with DMA\r\n");
    HAL_Delay(2000);  // Adding delay for clarity during debugging

    MX_GPIO_Init();              // Initialize GPIOs
    DMA_Init();                  // Initialize DMA
    UART_Init();                 // Initialize UART
    MX_ADC_Init();               // Initialize ADC
    MX_TIM_Init();               // Initialize Timer
    

    HAL_TIM_Base_Start(&htim3);
    HAL_TIM_PWM_Start_IT(&htim3, TIM_CHANNEL_3); // Start PWM
    HAL_ADC_Start_DMA(&hadc1, (uint32_t *)adc_buf, 1); // Start ADC with DMA

    while (1) {
        // Read and display ADC value
        float voltage = (adc_buf[0] * 3.3) / 4095;
        snprintf(uart_buffer, sizeof(uart_buffer), "Voltage: %.2f V\r\n", voltage);
        UART_Transmit(uart_buffer);

        // Update PWM duty cycle based on ADC value
        PWM_SetDutyCycle((uint8_t)((adc_buf[0] * 100) / 4095));
        HAL_Delay(1000); // Delay for output readability
    }
}

void MX_GPIO_Init(void) {
    // Enable clocks for GPIO ports
    __HAL_RCC_GPIOA_CLK_ENABLE();
    __HAL_RCC_GPIOB_CLK_ENABLE();

    // Configure GPIO PA0 for analog input (ADC)
    GPIO_Init('A', GPIO_PIN_0, GPIO_MODE_ANALOG, GPIO_NOPULL, NULL, NULL);

    // Configure GPIO PB0 for alternate function (PWM output for TIM3_CH3)
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    GPIO_InitStruct.Pin = GPIO_PIN_0;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    GPIO_InitStruct.Alternate = GPIO_AF2_TIM3;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
}

void DMA_Init(void) {
        // Enable the clock for DMA2, required for DMA operations
    __HAL_RCC_DMA2_CLK_ENABLE();

    // Configure the DMA handle for ADC1
    hdma_adc1.Instance = DMA2_Stream0;  // Select DMA2 Stream 0 for ADC1
    hdma_adc1.Init.Channel = DMA_CHANNEL_0;  // Set DMA channel 0 (connected to ADC1)
    hdma_adc1.Init.Direction = DMA_PERIPH_TO_MEMORY;  // Data transfer direction: from peripheral (ADC) to memory
    hdma_adc1.Init.PeriphInc = DMA_PINC_DISABLE;  // Disable peripheral address increment (fixed ADC address)
    hdma_adc1.Init.MemInc = DMA_MINC_ENABLE;  // Enable memory address increment (data stored sequentially in memory)
    hdma_adc1.Init.PeriphDataAlignment = DMA_PDATAALIGN_WORD;  // Set peripheral data alignment to 32-bit word
    hdma_adc1.Init.MemDataAlignment = DMA_MDATAALIGN_WORD;  // Set memory data alignment to 32-bit word
    hdma_adc1.Init.Mode = DMA_CIRCULAR;  // Enable circular mode for continuous data transfer
    hdma_adc1.Init.Priority = DMA_PRIORITY_LOW;  // Set low priority for this DMA stream
    hdma_adc1.Init.FIFOMode = DMA_FIFOMODE_DISABLE;  // Disable FIFO mode for direct data transfer

    // Initialize the DMA with the specified configuration
    if (HAL_DMA_Init(&hdma_adc1) != HAL_OK) {
        while (1); // Loop forever if DMA initialization fails (error handling)
    }

    // Link the DMA handle to the ADC handle
    __HAL_LINKDMA(&hadc1, DMA_Handle, hdma_adc1);  // Associate DMA handle with ADC1 for data transfer

}



void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef *hadc) {
    if (hadc->Instance == ADC1) {
        // ADC value conversion to voltage
        float voltage = ((float)adc_buffer[0] / 4096) * 3.3;
        snprintf(uart_buffer, sizeof(uart_buffer), "ADC Voltage: %.2f V\r\n", voltage);
        UART_Transmit(uart_buffer);
    }
}
void MX_ADC_Init(void) {
        // Enable the clock for ADC1 peripheral
    __HAL_RCC_ADC1_CLK_ENABLE();

    // Configure the ADC instance
    hadc1.Instance = ADC1;  // Specify ADC1 as the ADC instance to configure
    hadc1.Init.Resolution = ADC_RESOLUTION_12B;  // Set ADC resolution to 12 bits
    hadc1.Init.ScanConvMode = DISABLE;  // Disable scan mode (single channel conversion)
    hadc1.Init.ContinuousConvMode = ENABLE;  // Enable continuous conversion mode for back-to-back conversions
    hadc1.Init.DiscontinuousConvMode = DISABLE;  // Disable discontinuous conversion mode
    hadc1.Init.ExternalTrigConv = ADC_EXTERNALTRIGCONV_T3_TRGO;  // Use Timer 3 TRGO signal as an external trigger for ADC
    hadc1.Init.DataAlign = ADC_DATAALIGN_RIGHT;  // Right-align the ADC data
    hadc1.Init.NbrOfConversion = 1;  // Configure for a single conversion in regular group
    hadc1.Init.DMAContinuousRequests = ENABLE;  // Enable continuous DMA requests for ADC data transfer
    hadc1.Init.EOCSelection = ADC_EOC_SINGLE_CONV;  // End of conversion flag set after each conversion

    // Initialize the ADC with the specified parameters
    if (HAL_ADC_Init(&hadc1) != HAL_OK) {
        while (1); // Loop forever if ADC initialization fails (error handling)
    }

    // Configure the specific ADC channel
    ADC_ChannelConfTypeDef sConfig = {0};  // Create and initialize a configuration structure
    sConfig.Channel = ADC_CHANNEL_0;  // Select channel 0 (PA0 pin) for ADC conversion
    sConfig.Rank = 1;  // Set rank to 1 for single-channel conversion
    sConfig.SamplingTime = ADC_SAMPLETIME_144CYCLES;  // Set sampling time to 144 ADC clock cycles for higher stability

    // Apply the channel configuration
    if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK) {
        while (1); // Loop forever if channel configuration fails (error handling)
    }

}

void MX_TIM_Init(void) {
    __HAL_RCC_TIM3_CLK_ENABLE();

    htim3.Instance = TIM3;
    htim3.Init.Prescaler = 16799;  // Adjust for 10 kHz timer frequency
    htim3.Init.CounterMode = TIM_COUNTERMODE_UP;
    htim3.Init.Period = 999;      // Adjust for 10 Hz TRGO rate
    htim3.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
    htim3.Init.RepetitionCounter = 0;
    htim3.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_ENABLE;

    if (HAL_TIM_Base_Init(&htim3) != HAL_OK) {
        while (1); // Error Handling
    }

    TIM_MasterConfigTypeDef sMasterConfig = {0};
    sMasterConfig.MasterOutputTrigger = TIM_TRGO_UPDATE; // Generate TRGO on update
    sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;

    if (HAL_TIMEx_MasterConfigSynchronization(&htim3, &sMasterConfig) != HAL_OK) {
        while (1); // Error Handling
    }

    TIM_OC_InitTypeDef sConfig = {0};
    sConfig.OCMode = TIM_OCMODE_PWM1;
    sConfig.Pulse = 0; // Initialize with 0 duty cycle
    sConfig.OCPolarity = TIM_OCPOLARITY_HIGH;
    sConfig.OCFastMode = TIM_OCFAST_DISABLE;

    if (HAL_TIM_PWM_ConfigChannel(&htim3, &sConfig, TIM_CHANNEL_3) != HAL_OK) {
        while (1); // Error Handling
    }

    HAL_TIM_MspPostInit(&htim3);
}

void HAL_TIM_MspPostInit(TIM_HandleTypeDef *htim) {
    if (htim->Instance == TIM3) {
        GPIO_InitTypeDef GPIO_InitStruct = {0};

        __HAL_RCC_GPIOB_CLK_ENABLE();
        GPIO_InitStruct.Pin = GPIO_PIN_0;
        GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
        GPIO_InitStruct.Pull = GPIO_NOPULL;
        GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
        GPIO_InitStruct.Alternate = GPIO_AF2_TIM3;
        HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
    }
}


void PWM_SetDutyCycle(uint8_t duty) {
    if (duty > 100) duty = 100;
    __HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_3, (htim3.Init.Period + 1) * duty / 100);
}

void SysTick_Handler(void) {
    HAL_IncTick();
}
