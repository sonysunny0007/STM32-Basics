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

// Timer 2 configuration (Temperature Sensor - 1 second interval)
    Timer_Config timer2_config;
    timer2_config.Instance = TIM2;
    timer2_config.Prescaler = 16799;  // Prescaler for 10 kHz (assuming 84 MHz clock)
    timer2_config.Period = 10000 - 1;    // 1-second period (10,000 ticks at 10 kHz)
    timer2_config.ClockDivision = TIM_CLOCKDIVISION_DIV1;
    timer2_config.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
    timer2_config.Callback = Timer2Callback;

    // Timer 3 configuration (LED Toggle - 1-second interval)
    Timer_Config timer3_config;
    timer3_config.Instance = TIM3;
    timer3_config.Prescaler = 16800 - 1;  // Prescaler for 10 kHz (168 MHz clock)
    timer3_config.Period = 10000 - 1;     // 1-second period (10,000 ticks at 10 kHz)
    timer3_config.ClockDivision = TIM_CLOCKDIVISION_DIV1;
    timer3_config.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
    timer3_config.Callback = Timer3Callback;

    // Initialize both timers
    Timer_Init(&timer2_config);
    Timer_Init(&timer3_config);

    // Start both timers
    Timer_Start(&timer2_config);
    Timer_Start(&timer3_config);

    while (1) {
        if(flag_10s) {
            flag_10s = 0;
            static uint8_t led_state = 0;
            // Toggle an LED for system health status
            led_state = !led_state;
            HAL_GPIO_WritePin(GPIOA, GPIO_PIN_6, led_state ? GPIO_PIN_SET : GPIO_PIN_RESET);
            printf("Timer 3 interrupt triggered! LED toggled: %s\r\n", led_state ? "ON" : "OFF");
        }
    }
}

// Timer callback functions
void Timer2Callback(void) {
    static uint8_t temperature_sensor_read_count = 0;

    printf("Timer 2 interrupt triggered! Reading temperature sensor...\r\n");

    // Simulate temperature reading (actual sensor reading code would go here)
    float temperature = 25.0 + (rand() % 100) / 10.0;
    printf("Temperature Reading: %.2f°C\r\n", temperature);

    // Count every 60 readings (60 seconds) for logging or sending to cloud
    if (++temperature_sensor_read_count >= 60) {
        temperature_sensor_read_count = 0;
        printf("1-minute temperature summary sent to cloud.\r\n");
    }
}

// Timer 3 callback function
void Timer3Callback(void)
{
    counter_10s++;
    if (counter_10s >= 10) {
        counter_10s = 0;
        flag_10s = 1;  // Trigger 10s flag
    }

  //  printf("Timer3Callback executed. Counter: %d\r\n", counter_10s);
}

void SysTick_Handler(void) {
    HAL_IncTick();
}