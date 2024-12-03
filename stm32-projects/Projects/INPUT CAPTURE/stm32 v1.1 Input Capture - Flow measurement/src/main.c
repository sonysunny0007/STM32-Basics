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

volatile uint32_t capture_val1 = 0; // First captured value
volatile uint32_t capture_val2 = 0; // Second captured value
volatile uint8_t is_captured = 0;   // Flag to indicate the capture complete
volatile uint32_t pulse_count = 0;  // Tracks the total number of pulses
volatile float flow_rate = 0;       // Flow rate in litres per second
const float kfactor = 1050.0;       // Pulses per litre.

void MX_TIM2_Init(void);

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

    MX_TIM3_Init();

    // Start input capture in interrupt mode
    HAL_TIM_IC_Start_IT(&htim2, TIM_CHANNEL_1);

    char uart_buffer[100];

    while (1)
    {

        if (is_captured == 1)
        {                                                         // if two captures are done
            uint32_t period = (capture_val2 > capture_val1)       // If capture_val2 > capture_val1, the period is simply capture_val2 - capture_val1.
                                  ? (capture_val2 - capture_val1) // If the timer counter has overflowed between the two captures, the period is adjusted to account for the overflow
                                  : ((0xffffffff - capture_val1) + capture_val2);
            uint32_t timer_clock = HAL_RCC_GetPCLK1Freq() / (htim2.Init.Prescaler + 1); // Calculate Timer Clock Frequency
            float frequency = (float)timer_clock / period;

            // calculate the flow in litre per second
            flow_rate = frequency / kfactor;

            // Reset the capture flag
            is_captured = 0;

            // Trasnmit the pulse count and flow rate through uart
            snprintf(uart_buffer, sizeof(uart_buffer), "Pulses: %lu, Flow rate: %.3f L/s \r\n", pulse_count, flow_rate);
            UART_Transmit(uart_buffer);
        }
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
    if(htim->Channel = HAL_TIM_ACTIVE_CHANNEL_1){
        if(is_captured == 0){
            capture_val1 = HAL_TIM_ReadCapturedValue(htim, TIM_CHANNEL_1);
            pulse_count++;
            is_captured=1;
        }else if(is_captured==1){
            capture_val2 = HAL_TIM_ReadCapturedValue(htim, TIM_CHANNEL_1);
            is_captured=1;
        }
    }
}

void SysTick_Handler(void)
{
    HAL_IncTick();
}