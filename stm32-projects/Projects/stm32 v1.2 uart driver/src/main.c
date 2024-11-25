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
TIM_HandleTypeDef htim2;

//Software counters for intervals
volatile uint32_t counter_10s = 0;
volatile uint32_t counter_300s = 0;
volatile uint32_t counter_600s = 0;


//flags for each interval
volatile uint8_t flag_10s = 0;
volatile uint8_t flag_300s = 0;
volatile uint8_t flag_600s = 0;

//simulated sensor data buffer
#define BUFFER_SIZE 300
float temperature_buffer[BUFFER_SIZE];
uint32_t buffer_index = 0;


void MX_TIM2_Init(void);
void Timer2Callback(void);
void Timer3Callback(void);


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
//    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_6, GPIO_PIN_SET);

// UART Initialization with baud rate of 9600 for all UARTs
    UART_Init(&huart1, 9600);
    UART_Init(&huart2, 9600);
    UART_Init(&huart6, 9600);

    // Transmit an initial message over UART2
    uint8_t init_message[] = "System Initialized on UART2\r\n";
    UART_Transmit_IT(&huart2, init_message, sizeof(init_message) - 1);

    // Start receiving data on all UARTs
    UART_Receive_IT(&huart1, uart_rx_buffer, 1);
    UART_Receive_IT(&huart2, uart_rx_buffer, 1);
    UART_Receive_IT(&huart6, uart_rx_buffer, 1);

    // Main loop
    while (1)
    {
        // Example: Process received data on UART1 and send an echo back
        if (uart_rx_index > 0)
        {
            // Echo received data on UART1
            UART_Transmit_IT(&huart1, uart_rx_buffer, uart_rx_index);
            uart_rx_index = 0;  // Reset index after transmission
        }

        // Example: UART6 can communicate with another peripheral
        // Send periodic data to a peripheral through UART6
        static uint32_t tick = 0;
        if (HAL_GetTick() - tick > 1000) // 1 second interval
        {
            uint8_t data[] = "Sending data to peripheral on UART6\r\n";
            UART_Transmit_IT(&huart6, data, sizeof(data) - 1);
            tick = HAL_GetTick();
        }
    }
}

void SysTick_Handler(void) {
    HAL_IncTick();
}