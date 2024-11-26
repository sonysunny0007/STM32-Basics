#include "stm32f4xx_hal.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "uart.h"
#include "sysclock.h"
#include "gpio.h"
#include "math.h"
#include "timer.h"

// echo -e "Sony Sunny" > /dev/tty.usbserial-0001    //use this code in minicom to send the data
// PA0 for TIM2_CH1

TIM_HandleTypeDef htim2;   // Timer 2 handle
UART_HandleTypeDef huart2; // Uart 2 handle

volatile uint32_t pulse_count = 0;            //Tracks total number of pulses

void MX_TIM2_Init(void);
void HAL_TIM_IC_CaptureCallback(TIM_HandleTypeDef *htim);

int main(void)
{
    // Initialize the HAL Library
    HAL_Init();

    // Configure system clock
    SystemClock_Config();

    // // Initialize GPIO and UART
    // GPIO_Init();
    UART_Init();

    // Initial boot message
    UART_Transmit("****************************************************************\r\n");
    UART_Transmit("System Booting.... #PWM Project#\r\n");

    // Print system clock frequency
    uint32_t sysclk = Get_SYSCLK_Frequency();
    printf("System clock frequency: %lu MHz\r\n", sysclk / 1000000);
    // adding 2s delay
    HAL_Delay(2000);

    // Configre GPIO PIN6
    GPIO_Init('A', GPIO_PIN_6, GPIO_MODE_OUTPUT_PP, GPIO_NOPULL, GPIO_SPEED_FREQ_LOW, GPIO_AF2_TIM3);

    MX_TIM2_Init();

    // Start input capture in interrupt mode
    HAL_TIM_IC_Start_IT(&htim2, TIM_CHANNEL_1);

    char uart_buffer[100];

    while (1)
    {
        snprintf(uart_buffer, sizeof(uart_buffer), "Pulse count: %lu\r\n", pulse_count);
        UART_Transmit(uart_buffer);
        HAL_Delay(1000);                //wait for 1 second

    }
}

// TIM3 Initialization

void MX_TIM2_Init(void)
{

    __HAL_RCC_TIM3_CLK_ENABLE();

    htim2.Instance = TIM2;
    htim2.Init.Prescaler = 16800 - 1;
    htim2.Init.CounterMode = TIM_COUNTERMODE_UP;
    htim2.Init.Period = 1000 - 1;
    htim2.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;

    if (HAL_TIM_PWM_Init(&htim2) != HAL_OK)
    {
        while (1); // Error handling
    }

    // Input capture configuration
    TIM_IC_InitTypeDef sConfigOC = {0};
    sConfigOC.ICPolarity = TIM_INPUTCHANNELPOLARITY_RISING; // capture on rising edge
    sConfigOC.ICSelection = TIM_ICSELECTION_DIRECTTI;       // Map directly to input pin
    sConfigOC.ICPrescaler = TIM_ICPSC_DIV1;                 // No prescaler for input capture
    sConfigOC.ICFilter = 0;
    HAL_TIM_IC_ConfigChannel(&htim2, &sConfigOC, TIM_CHANNEL_1);
}

void HAL_TIM_IC_CaptureCallback(TIM_HandleTypeDef *htim){
    if(htim->Channel == HAL_TIM_ACTIVE_CHANNEL_1){
        pulse_count++;          //increment pulse count every rising edge
    }
}

void SysTick_Handler(void)
{
    HAL_IncTick();
}