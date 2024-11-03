#include "stm32f4xx_hal.h"
#include <stdio.h>

#define BUTTON_PIN              GPIO_PIN_2
#define BUTTON_GPIO_PORT        GPIOE
#define BUTTON_GPIO_CLK_ENABLE   __HAL_RCC_GPIOE_CLK_ENABLE


#define LED_PIN1                 GPIO_PIN_6
#define LED_PIN2                 GPIO_PIN_2
#define LED_GPIO_PORT            GPIOA
#define LED_GPIO_CLK_ENABLE1     __HAL_RCC_GPIOA_CLK_ENABLE
#define LED_GPIO_CLK_ENABLE2     __HAL_RCC_GPIOA_CLK_ENABLE



// Define the UART handle
//UART_HandleTypeDef huart2;

void SystemClock_Config(void);
void LED_Init();
void BUTTON_Init();

int main(void) {
    HAL_Init();
    SystemClock_Config();

    BUTTON_Init();
    LED_Init();


    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_6, GPIO_PIN_RESET); // Start with the LED OFF
    HAL_Delay(1000);  // Initial delay if needed
    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_6, GPIO_PIN_SET); // Start with the LED OFF
     HAL_GPIO_WritePin(LED_GPIO_PORT, LED_PIN2, GPIO_PIN_RESET); // Turn OFF the LED when button is not pressed

    if (HAL_GPIO_ReadPin(BUTTON_GPIO_PORT,BUTTON_PIN) == GPIO_PIN_SET) { // Button pressed
         // printf("Button Pressed\n");
            HAL_GPIO_WritePin(LED_GPIO_PORT,LED_PIN2, GPIO_PIN_SET); // Turn ON the LED
         //  HAL_Delay(500);  // Debounce delay
        }else {
           HAL_GPIO_WritePin(LED_GPIO_PORT, LED_PIN2, GPIO_PIN_RESET); // Turn OFF the LED when button is not pressed
       //  HAL_Delay(500);  // Debounce delay
        }
}



void LED_Init(){
    LED_GPIO_CLK_ENABLE1();
    LED_GPIO_CLK_ENABLE2();
    GPIO_InitTypeDef GPIO_InitStruct={0};
    GPIO_InitStruct.Pin=LED_PIN1;
    GPIO_InitStruct.Mode=GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull=GPIO_NOPULL;
    GPIO_InitStruct.Speed=GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(LED_GPIO_PORT, &GPIO_InitStruct);

    GPIO_InitStruct.Pin=LED_PIN2;
    HAL_GPIO_Init(LED_GPIO_PORT, &GPIO_InitStruct);



}


void BUTTON_Init(){
    BUTTON_GPIO_CLK_ENABLE();
    GPIO_InitTypeDef GPIO_InitStruct={0};
    GPIO_InitStruct.Pin=BUTTON_PIN;
    GPIO_InitStruct.Mode=GPIO_MODE_INPUT;
    GPIO_InitStruct.Pull=GPIO_PULLUP;
    HAL_GPIO_Init(BUTTON_GPIO_PORT, &GPIO_InitStruct);

}


void SystemClock_Config(void) {
    RCC_OscInitTypeDef RCC_OscInitStruct = {0};
    RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

    RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
    RCC_OscInitStruct.HSEState = RCC_HSE_ON;
    RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
    RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
    RCC_OscInitStruct.PLL.PLLM = 8;
    RCC_OscInitStruct.PLL.PLLN = 336;
    RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
    RCC_OscInitStruct.PLL.PLLQ = 7;
    HAL_RCC_OscConfig(&RCC_OscInitStruct);

    RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK |
                                  RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2;
    RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
    RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
    RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV4;
    RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;
    HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_5);
}

void SysTick_Handler(void) {
  HAL_IncTick();
}