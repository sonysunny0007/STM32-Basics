#include "stm32f4xx_hal.h"
#include <string.h>
#include <stdio.h>
#include "uart.h"
#include "sysclock.h"
#include "gpio.h"

//echo -e "Sony Sunny" > /dev/tty.usbserial-0001    //use this code in minicom to send the data

int main(void)
{
    // Initialize the HAL Library
    HAL_Init();

    // Configure system clock
    SystemClock_Config();

    // // Initialize GPIO and UART
    //GPIO_Init();
    UART_Init();

    //start uart reception
    UART_StartReception();

    // Initial boot message
    UART_Transmit("****************************************************************\r\n");
    UART_Transmit("System Booting....\r\n");
    //printf("Data Received: ");

    //Print system clock frequency
    uint32_t sysclk = Get_SYSCLK_Frequency();
    printf("System clock frequency: %lu MHz\r\n", sysclk/1000000);

    //adding 2s delay
    HAL_Delay(2000);

    //Enable PA6 led
    GPIO_Init('A',GPIO_PIN_6,GPIO_MODE_OUTPUT_PP,GPIO_NOPULL,GPIO_SPEED_FREQ_LOW);
    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_6, GPIO_PIN_SET);


    printf("Enter any data followd by entery key\r\n");

    while (1) {

    }
}
   



void SysTick_Handler(void) {
    HAL_IncTick();
}