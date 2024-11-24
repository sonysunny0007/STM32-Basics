#include "stm32f4xx_hal.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "uart.h"
#include "sysclock.h"
#include "gpio.h"
#include "math.h"

//echo -e "Sony Sunny" > /dev/tty.usbserial-0001    //use this code in minicom to send the data

//Timer 2 handle
TIM_HandleTypeDef htim2;

//Software counters for intervals
volatile uint32_t counter_60s = 0;
volatile uint32_t counter_300s = 0;
volatile uint32_t counter_600s = 0;


//flags for each interval
volatile uint8_t flag_60s = 0;
volatile uint8_t flag_300s = 0;
volatile uint8_t flag_600s = 0;

//simulated sensor data buffer
#define BUFFER_SIZE 300
float temperature_buffer[BUFFER_SIZE];
uint32_t buffer_index = 0;

void MX_TIM2_Init(void);
float ReadTemperatureSensor(void);
float CalculateAverageTemperature(void);
void LogData(float avgTemp);
void SendDataToCloud(float avgTemp);



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

    float temp=112.3;
    printf("Temperature: %f", temp);
    
    

    //adding 2s delay
    HAL_Delay(2000);

    //Enable PA6 led
    GPIO_Init('A',GPIO_PIN_6,GPIO_MODE_OUTPUT_PP,GPIO_NOPULL,GPIO_SPEED_FREQ_LOW);
    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_6, GPIO_PIN_RESET);

    MX_TIM2_Init(); // Initialize the timer
    HAL_TIM_Base_Start_IT(&htim2); // Start the timer interrupt
    HAL_NVIC_SetPriority(TIM2_IRQn, 0, 0);
    HAL_NVIC_EnableIRQ(TIM2_IRQn);
    
    srand(HAL_GetTick());

    while (1) {

        if (flag_60s) {
        flag_60s = 0;

        float temperature = ReadTemperatureSensor();
        printf("Read Temperature: %.2f°C\r\n", temperature); // Debug the raw reading
        temperature_buffer[buffer_index] = temperature;
        printf("Stored Temperature: %.2f°C\r\n", temperature_buffer[buffer_index]);

        buffer_index++;
        if (buffer_index >= BUFFER_SIZE) {
        buffer_index = 0; // Circular buffer
    }
}

        if(flag_300s){
            flag_300s=0;
            //calculate and log avergare temperature
            float avgTemp = CalculateAverageTemperature();
            LogData(avgTemp);
        }

        if (flag_600s)
        {
            flag_600s = 0;
            // Send data summary to the cloud
            float avgTemp = CalculateAverageTemperature();
            SendDataToCloud(avgTemp);
        }

    }
}

// TIM2 Initialization (1-second interval)
void MX_TIM2_Init(void)
{
    __HAL_RCC_TIM2_CLK_ENABLE();
    htim2.Instance = TIM2;
    htim2.Init.Prescaler = 16799;          // for 10 Khz, prescaler = (168000,000/ 10,000)-1 = 16799
    htim2.Init.CounterMode = TIM_COUNTERMODE_UP;
    htim2.Init.Period = 10000 - 1;            // 1-second interval (10,000 ticks at 10 kHz)
    htim2.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
    htim2.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
    if (HAL_TIM_Base_Init(&htim2) != HAL_OK)
    {
        while (1); // Error handling
    }
}

// Timer interrupt callback
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
    if (htim->Instance == TIM2)
    {
        // Increment counters
        counter_60s++;
        counter_300s++;
        counter_600s++;
    
        printf("Counter_60s: %ld, Counter_300s: %ld, Counter_600s: %ld\r\n", counter_60s, counter_300s, counter_600s);
        // Set flags when thresholds are reached
        if (counter_60s >= 60)
        {
            counter_60s = 0;
            flag_60s = 1;
            
        }

        if (counter_300s >= 300)
        {
            counter_300s = 0;
            flag_300s = 1;
        }

        if (counter_600s >= 600)
        {
            counter_600s = 0;
            flag_600s = 1;
        }
    }
}

// Interrupt handler for Timer 2
void TIM2_IRQHandler(void)
{
    HAL_TIM_IRQHandler(&htim2); // Pass control to the HAL
}

// Dummy functions to simulate sensor and cloud interactions
float ReadTemperatureSensor(void)
{
    // Simulate temperature reading (random data for demonstration)
    return 25.0 + (rand() % 100) / 10.0; // Random temperature between 25.0 and 34.9
}

float CalculateAverageTemperature(void)
{
    float sum=0.0;
    for(uint32_t i=0; i<buffer_index; i++)
        sum+=temperature_buffer[i];
    return (buffer_index>0)?(sum/buffer_index):0.0;
}

void LogData(float avgTemp)
{
    // Simulate logging data to storage
    printf("Logging Data: Avg Temp = %.2f°C\n", avgTemp);
}

void SendDataToCloud(float avgTemp)
{
    // Simulate sending data to the cloud
    printf("Sending Data to Cloud: Avg Temp = %.2f°C\n", avgTemp);
}

void SysTick_Handler(void) {
    HAL_IncTick();
}