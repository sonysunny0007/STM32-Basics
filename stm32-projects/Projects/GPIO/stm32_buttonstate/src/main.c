#include "stm32f4xx_hal.h"
#include <stdio.h>

#define BUTTON_PIN       GPIO_PIN_3
#define BUTTON_GPIO_PORT GPIOE
#define BUTTON_GPIO_CLK_ENABLE   __HAL_RCC_GPIOE_CLK_ENABLE

#define LED_PIN1                 GPIO_PIN_6
#define LED_PIN2                 GPIO_PIN_2
#define LED_GPIO_PORT            GPIOA
#define LED_GPIO_CLK_ENABLE1     __HAL_RCC_GPIOA_CLK_ENABLE
#define LED_GPIO_CLK_ENABLE2     __HAL_RCC_GPIOA_CLK_ENABLE

UART_HandleTypeDef huart2;

void BUTTON_Init();
void SystemClock_Config(void);


void UART_Init(void) {
    __HAL_RCC_USART2_CLK_ENABLE();
    __HAL_RCC_GPIOA_CLK_ENABLE();

    GPIO_InitTypeDef GPIO_InitStruct = {0};
    GPIO_InitStruct.Pin = GPIO_PIN_2 | GPIO_PIN_3; // PA2 -> TX, PA3 -> RX
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    GPIO_InitStruct.Alternate = GPIO_AF7_USART2;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

    huart2.Instance = USART2;
    huart2.Init.BaudRate = 9600;
    huart2.Init.WordLength = UART_WORDLENGTH_8B;
    huart2.Init.StopBits = UART_STOPBITS_1;
    huart2.Init.Parity = UART_PARITY_NONE;
    huart2.Init.Mode = UART_MODE_TX_RX;
    huart2.Init.HwFlowCtl = UART_HWCONTROL_NONE;
    huart2.Init.OverSampling = UART_OVERSAMPLING_16;
    HAL_UART_Init(&huart2);
}

int main(void) {
    HAL_Init();
    LED_Init();
 //   SystemClock_Config();
    UART_Init();
    // Initialize GPIO for the button (Assume BUTTON_Init sets up the pin as input)
    BUTTON_Init();
    
    
    // Variable to hold the GPIO status
    GPIO_PinState buttonState;

    while (1) {
        // Read the button GPIO pin state and store it in the variable
        buttonState = HAL_GPIO_ReadPin(BUTTON_GPIO_PORT, BUTTON_PIN);
        
        // You can now use buttonState in conditions or for other logic
        if (buttonState == GPIO_PIN_SET) {
            char msg[] = "gpio enabled...\r\n";
            HAL_UART_Transmit(&huart2, (uint8_t *)msg, strlen(msg), HAL_MAX_DELAY);
            HAL_GPIO_WritePin(LED_GPIO_PORT,LED_PIN1, GPIO_PIN_RESET); // Turn ON the LED
            HAL_Delay(1000);  // Debounce delay

        } else {
            char msg[] = "gpio_disbled...\r\n";
            HAL_UART_Transmit(&huart2, (uint8_t *)msg, strlen(msg), HAL_MAX_DELAY);
            HAL_GPIO_WritePin(LED_GPIO_PORT,LED_PIN1, GPIO_PIN_SET); // Turn ON the LED
            HAL_Delay(1000);  // Debounce delay
        }
    }
}

void BUTTON_Init(void) {
    // Assuming BUTTON_GPIO_CLK_ENABLE is defined to enable the GPIO port clock
    BUTTON_GPIO_CLK_ENABLE();

    GPIO_InitTypeDef GPIO_InitStruct = {0};
    GPIO_InitStruct.Pin = BUTTON_PIN;
    GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
    GPIO_InitStruct.Pull = GPIO_PULLUP;  // Use pull-up if button is active low
    HAL_GPIO_Init(BUTTON_GPIO_PORT, &GPIO_InitStruct);
}

void LED_Init(){
    LED_GPIO_CLK_ENABLE1();
 //   LED_GPIO_CLK_ENABLE2();
    GPIO_InitTypeDef GPIO_InitStruct={0};
    GPIO_InitStruct.Pin=LED_PIN1;
    GPIO_InitStruct.Mode=GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull=GPIO_NOPULL;
    GPIO_InitStruct.Speed=GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(LED_GPIO_PORT, &GPIO_InitStruct);



}

void SystemClock_Config(void) {
    RCC_OscInitTypeDef RCC_OscInitStruct = {0};
    RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

    // Configure the HSE oscillator and PLL
    RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
    RCC_OscInitStruct.HSEState = RCC_HSE_ON;
    RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
    RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
    RCC_OscInitStruct.PLL.PLLM = 8;  // HSE input frequency = 8 MHz
    RCC_OscInitStruct.PLL.PLLN = 336; // PLL VCO output frequency = 336 MHz
    RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2; // CPU clock = 168 MHz
    RCC_OscInitStruct.PLL.PLLQ = 7; // USB OTG FS clock = 48 MHz
    HAL_RCC_OscConfig(&RCC_OscInitStruct);

    // Select the PLL as system clock source
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