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
DMA_HandleTypeDef hdma;
char uart_buffer[50];
uint32_t adc_buf[1];
volatile uint32_t printFlag = 0;  // Make it volatile for proper interrupt handling

// Function Prototypes
void DMA_Init(void);
void ADC1_Init(void);

int main(void) {
    HAL_Init();                  // Initialize HAL Library
    SystemClock_Config();        // Configure system clock
    DMA_Init();
    UART_Init();                 // Initialize UART
    ADC1_Init(); 

    // Initialize GPIO for ADC input
    GPIO_Init('A', GPIO_PIN_0, GPIO_MODE_ANALOG, GPIO_NOPULL, NULL, NULL);

    // Start ADC with DMA in continuous mode
    HAL_ADC_Start_DMA(&hadc1, adc_buf, 1);

    while (1) {
        float voltage = ((float)adc_buf[0] / 4095) * 3.3;  // Assuming 3.3V VREF

        // Print ADC value and voltage
        snprintf(uart_buffer, sizeof(uart_buffer), "ADC: %lu, Voltage: %.2f V\r\n", adc_buf[0], voltage);
        UART_Transmit(uart_buffer);
        
        HAL_Delay(1000);  // Delay to control print rate
    }
}

void ADC1_Init(void) {
    __HAL_RCC_ADC1_CLK_ENABLE();

    hadc1.Instance = ADC1;
    hadc1.Init.Resolution = ADC_RESOLUTION_12B;
    hadc1.Init.ScanConvMode = DISABLE;        // Single channel
    hadc1.Init.ContinuousConvMode = ENABLE;   // Continuous conversion
    hadc1.Init.DiscontinuousConvMode = DISABLE;
    hadc1.Init.ExternalTrigConvEdge = ADC_EXTERNALTRIGCONVEDGE_NONE;
    hadc1.Init.ExternalTrigConv = ADC_SOFTWARE_START; // No external trigger
    hadc1.Init.DataAlign = ADC_DATAALIGN_RIGHT;
    hadc1.Init.NbrOfConversion = 1;
    hadc1.Init.DMAContinuousRequests = ENABLE;
    hadc1.Init.EOCSelection = ADC_EOC_SINGLE_CONV;

    if (HAL_ADC_Init(&hadc1) != HAL_OK) {
        Error_Handler();
    }

    // Configure ADC channel
    ADC_ChannelConfTypeDef sConfig = {0};
    sConfig.Channel = ADC_CHANNEL_0;           // Channel 0 (PA0)
    sConfig.Rank = 1;                          // Rank 1
    sConfig.SamplingTime = ADC_SAMPLETIME_144CYCLES; // Higher sampling time for stability

    if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK) {
        Error_Handler();
    }
}

void DMA_Init(void) {
    __HAL_RCC_DMA2_CLK_ENABLE();

    hdma.Instance = DMA2_Stream0;
    hdma.Init.Channel = DMA_CHANNEL_0;
    hdma.Init.Direction = DMA_PERIPH_TO_MEMORY;
    hdma.Init.PeriphInc = DMA_PINC_DISABLE;
    hdma.Init.MemInc = DMA_MINC_ENABLE;
    hdma.Init.PeriphDataAlignment = DMA_PDATAALIGN_WORD;
    hdma.Init.MemDataAlignment = DMA_MDATAALIGN_WORD;
    hdma.Init.Mode = DMA_CIRCULAR;  // Circular mode for continuous transfer
    hdma.Init.Priority = DMA_PRIORITY_LOW;
    hdma.Init.FIFOMode = DMA_FIFOMODE_DISABLE;

    if (HAL_DMA_Init(&hdma) != HAL_OK) {
        printf("Failed to initialize DMA \r\n");
        while (1);
    }

    // Link DMA handle to ADC
    __HAL_LINKDMA(&hadc1, DMA_Handle, hdma);

    // Enable DMA interrupt and set priority
    HAL_NVIC_SetPriority(DMA2_Stream0_IRQn, 0, 0);
    HAL_NVIC_EnableIRQ(DMA2_Stream0_IRQn);
}

void DMA2_Stream0_IRQHandler(void) {
     if (__HAL_DMA_GET_FLAG(&hdma, DMA_FLAG_TCIF0_4)) {
        // Transfer complete interrupt
        __HAL_DMA_CLEAR_FLAG(&hdma, DMA_FLAG_TCIF0_4);
        // You can add any code to process the data here if needed
    }
    HAL_DMA_IRQHandler(&hdma);  // Clear the interrupt and handle any callback
}

void SysTick_Handler(void) {
    HAL_IncTick();
}
