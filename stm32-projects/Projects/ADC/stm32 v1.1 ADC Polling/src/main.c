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

//ADC 1 handle
ADC_HandleTypeDef hadc1;


void MX_ADC_Init(void);
void MX_GPIO_Init(void);

int main(void)
{
    // Initialize the HAL Library
    HAL_Init();

    // Configure system clock
    SystemClock_Config();

    // // Initialize UART
    UART_Init();

    // Initial boot message
    UART_Transmit("****************************************************************\r\n");
    UART_Transmit("System Booting.... #ADC Polling Project#\r\n");

    //Print system clock frequency
    uint32_t sysclk = Get_SYSCLK_Frequency();
    printf("System clock frequency: %lu MHz\r\n", sysclk/1000000);
    //adding 2s delay
    HAL_Delay(2000);
  //  MX_GPIO_Init();

    MX_ADC_Init();

    //Configre GPIO PIN6
     GPIO_Init('A',GPIO_PIN_0,GPIO_MODE_ANALOG,GPIO_NOPULL,NULL, NULL);
    

  HAL_ADC_Start(&hadc1);  // Start the ADC

    while (1) {
        // Poll for ADC conversion
        if (HAL_ADC_PollForConversion(&hadc1, HAL_MAX_DELAY) == HAL_OK) {
            uint32_t adc_value = HAL_ADC_GetValue(&hadc1);  // Read ADC value
            float voltage = (adc_value * 3.3) / 4095;  // Convert to voltage
            printf("Voltage: %.2f V\r\n", voltage);
        }
        HAL_Delay(500);  // Delay to avoid spamming
    }
}
//TIM3 Initialization

void MX_ADC_Init(void){

  __HAL_RCC_ADC1_CLK_ENABLE();  // Enable ADC clock

    hadc1.Instance = ADC1;
    hadc1.Init.Resolution = ADC_RESOLUTION_12B;  // 12-bit resolution
    hadc1.Init.ScanConvMode = DISABLE;  // Single channel mode
    hadc1.Init.ContinuousConvMode = ENABLE;  // Continuous conversion
    hadc1.Init.DiscontinuousConvMode = DISABLE;
    hadc1.Init.ExternalTrigConv = ADC_SOFTWARE_START;  // Software trigger
    hadc1.Init.DataAlign = ADC_DATAALIGN_RIGHT;  // Align data to the right
    hadc1.Init.NbrOfConversion = 1;  // One conversion at a time

    if (HAL_ADC_Init(&hadc1) != HAL_OK) {
        // Handle initialization error
        while (1);
    }



    ADC_ChannelConfTypeDef sConfig = {0};
    sConfig.Channel = ADC_CHANNEL_0;  // Use channel 0 (PA0)
    sConfig.Rank = 1;  // First rank in regular conversion sequence
    sConfig.SamplingTime = ADC_SAMPLETIME_3CYCLES;  // Short sampling time

    if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK) {
        // Handle channel configuration error
        while (1);
    }

}

// void MX_GPIO_Init(void) {
//     __HAL_RCC_GPIOA_CLK_ENABLE();  // Enable GPIOA clock

//     GPIO_InitTypeDef GPIO_InitStruct = {0};
//     GPIO_InitStruct.Pin = GPIO_PIN_0;  // Configure PA0 as analog input
//     GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
//     GPIO_InitStruct.Pull = GPIO_NOPULL;
//     HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
// }

void SysTick_Handler(void) {
    HAL_IncTick();
}