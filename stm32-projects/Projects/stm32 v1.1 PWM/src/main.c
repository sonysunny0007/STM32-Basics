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

//Timer 2 handle
TIM_HandleTypeDef htim3;


void MX_TIM3_Init(void);

int main(void)
{
    // Initialize the HAL Library
    HAL_Init();

    // Configure system clock
    SystemClock_Config();

    // // Initialize GPIO and UART
    //GPIO_Init();
    UART_Init();

    // Initial boot message
    UART_Transmit("****************************************************************\r\n");
    UART_Transmit("System Booting.... #PWM Project#\r\n");

    //Print system clock frequency
    uint32_t sysclk = Get_SYSCLK_Frequency();
    printf("System clock frequency: %lu MHz\r\n", sysclk/1000000);
    //adding 2s delay
    HAL_Delay(2000);

    MX_TIM3_Init();

    //Configre GPIO PIN6
    GPIO_Init('A',GPIO_PIN_6,GPIO_MODE_AF_PP,GPIO_NOPULL,GPIO_SPEED_FREQ_LOW, GPIO_AF2_TIM3);


    //Start PWM signal generation on timer 3 channel 1 (PIN6)
    if(HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_1) != HAL_OK){
        printf("Failed to start PWM\r\n");
    };



    while(1){

        //Gradually increase and decrease LED brightness
        for(uint16_t duty=0; duty<=100; duty++){
            __HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_1, (duty * (htim3.Init.Period + 1)) / 100);
            HAL_Delay(50); //increase or decrease the delay to vary the ON/OFF time
        }
        for(uint16_t duty=100;duty>0;duty--){
            __HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_1, (duty * (htim3.Init.Period + 1)) / 100);
            HAL_Delay(50);
        }
        HAL_Delay(1000); //LED Stays fully off for 1 second
    }
}

//TIM3 Initialization

void MX_TIM3_Init(void){

    __HAL_RCC_TIM3_CLK_ENABLE();

    htim3.Instance=TIM3;
    htim3.Init.Prescaler=168-1;
    htim3.Init.CounterMode=TIM_COUNTERMODE_UP;
    htim3.Init.Period=1000-1;
    htim3.Init.ClockDivision=TIM_CLOCKDIVISION_DIV1;
    htim3.Init.AutoReloadPreload=TIM_AUTORELOAD_PRELOAD_DISABLE;

    if(HAL_TIM_PWM_Init(&htim3) != HAL_OK){
        printf("TIM3 Initialization Failed\r\n");
        while (1);
    }



    //Configure PWM channel
    TIM_OC_InitTypeDef sConfigOC={0};
    sConfigOC.OCMode = TIM_OCMODE_PWM1;
    sConfigOC.Pulse=0;
    sConfigOC.OCPolarity = TIM_OCNPOLARITY_HIGH;
    sConfigOC.OCFastMode = TIM_OCFAST_ENABLE;

    if (HAL_TIM_PWM_ConfigChannel(&htim3, &sConfigOC, TIM_CHANNEL_1) != HAL_OK) {
        printf("PWM Channel Configuration Failed\r\n");
        while (1);
    }

}

void SysTick_Handler(void) {
    HAL_IncTick();
}