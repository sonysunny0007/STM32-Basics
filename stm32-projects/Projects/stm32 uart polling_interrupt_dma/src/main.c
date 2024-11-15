#include "stm32f4xx_hal.h"
#include <stdio.h>
#include <string.h>


#define RX_BUFFER_SIZE 10
uint8_t rxBuffer[RX_BUFFER_SIZE];
uint8_t wordBuffer[RX_BUFFER_SIZE];
uint8_t rxIndex = 0;

void UART2_Init(void);
void uart_reception_polling();
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart);
void processedData(void);
void SystemClock_Config(void);
void Configure_PLL(void);

UART_HandleTypeDef huart2;



int main(void) {
    HAL_Init();
    SystemClock_Config();

    HAL_NVIC_SetPriorityGrouping(NVIC_PRIORITYGROUP_4);  // Set priority grouping
    HAL_NVIC_SetPriority(USART2_IRQn, 0, 1);  // Set the priority of USART2 interrupt
    HAL_NVIC_EnableIRQ(USART2_IRQn);          // Enable the USART2 interrupt

    UART2_Init();

    char msg[] = "System Booting...\r\n";
    HAL_UART_Transmit(&huart2, (uint8_t *)msg, strlen(msg), HAL_MAX_DELAY);

//Polling method
    // while(1){
    //     uart_reception_polling();
    // }

// Start UART reception with interrupt method
    HAL_UART_Receive_IT(&huart2, rxBuffer, 1);

// //DMA
//     while(1){
//         processedData();
//     }

while(1){

}

}

void uart_reception_polling(){

    if(HAL_UART_Receive(&huart2, rxBuffer,RX_BUFFER_SIZE, HAL_MAX_DELAY)==HAL_OK){
        HAL_UART_Transmit(&huart2, rxBuffer, strlen((char*)rxBuffer), HAL_MAX_DELAY);
    }
}


void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart) {
    if (huart->Instance == USART2) {  // Ensure it's USART2 triggering the callback
        char debug_msg[30];
        snprintf(debug_msg, sizeof(debug_msg), "Received: %c\n", rxBuffer[0]);
        HAL_UART_Transmit(&huart2, (uint8_t *)debug_msg, strlen(debug_msg), HAL_MAX_DELAY);

        if (rxBuffer[0] == '\n' || rxBuffer[0] == '\r') {
            // Transmit the full word stored in wordBuffer
            HAL_UART_Transmit(&huart2, wordBuffer, rxIndex, HAL_MAX_DELAY);
            HAL_UART_Transmit(&huart2, (uint8_t *)"\n", 1, HAL_MAX_DELAY);

            // Clear wordBuffer and reset rxIndex for new data
            memset(wordBuffer, 0, RX_BUFFER_SIZE);
            rxIndex = 0;
        } else {
            // Store the received character in wordBuffer if space is available
            if (rxIndex < RX_BUFFER_SIZE - 1) {
                wordBuffer[rxIndex++] = rxBuffer[0];
            }
        }

        // Reset rxBuffer and restart reception
        rxBuffer[0] = 0;
        HAL_UART_Receive_IT(&huart2, rxBuffer, 1);
    }
}


void processedData(void){

    //scan the rxBuffer to find the newline or carriage return character
    for(int i=0; i<RX_BUFFER_SIZE;i++){
        //check the character is a newline or carriage return
        if(rxBuffer[i]=='\n' || rxBuffer[i]=='\r'){
            //Transmit the full word stored in the wordBuffer
            HAL_UART_Transmit(&huart2, wordBuffer, rxIndex, HAL_MAX_DELAY);

            //clear the wordBudder and rxindex after sending
            memset(wordBuffer,0,RX_BUFFER_SIZE);
            rxIndex=0;
        }else{
            //store the received character in wordBuffer if not full
            if(rxIndex<RX_BUFFER_SIZE-1){
                wordBuffer[rxIndex++]=rxBuffer[i];
            }
        }

        //clear the character in the rxBuffer to avoid repeat processing
        rxBuffer[i]=0;
    }
}

void UART2_Init(void) {

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
    huart2.Init.BaudRate = 115200;
    huart2.Init.WordLength = UART_WORDLENGTH_8B;
    huart2.Init.StopBits = UART_STOPBITS_1;
    huart2.Init.Parity = UART_PARITY_NONE;
    huart2.Init.Mode = UART_MODE_TX_RX; 
    huart2.Init.HwFlowCtl = UART_HWCONTROL_NONE;
    huart2.Init.OverSampling = UART_OVERSAMPLING_16;

    if (HAL_UART_Init(&huart2) != HAL_OK) {
        // Initialization Error Handling
//        Error_Handler();
    }

    
}

void SystemClock_Config(void) {
    // Enable HSE oscillator
    RCC->CR |= RCC_CR_HSEON;
    while (!(RCC->CR & RCC_CR_HSERDY)); // Wait for HSE ready

    // Configure PLL to use HSE as the source and set up for 168 MHz
    Configure_PLL();

    // Set Flash latency for 168 MHz
    FLASH->ACR |= FLASH_ACR_LATENCY_5WS;

    // Switch system clock to PLL
    RCC->CFGR &= ~RCC_CFGR_SW;            // Clear system clock switch bits
    RCC->CFGR |= RCC_CFGR_SW_PLL;          // Select PLL as system clock
    while ((RCC->CFGR & RCC_CFGR_SWS) != RCC_CFGR_SWS_PLL);  // Wait until PLL is used as system clock

    SystemCoreClock = 168000000;  // Update SystemCoreClock to 168 MHz

    Configure_PLL();
}

void Configure_PLL(void) {
    // Set PLL source to HSE
    RCC->PLLCFGR |= RCC_PLLCFGR_PLLSRC_HSE; // Select HSE as PLL source

    // Configure PLLM, PLLN, and PLLP to achieve 168 MHz system clock
    RCC->PLLCFGR &= ~RCC_PLLCFGR_PLLM;             // Clear PLLM bits
    RCC->PLLCFGR |= (8 << RCC_PLLCFGR_PLLM_Pos);   // Set PLLM to 8 (8 MHz HSE / 8 = 1 MHz)

    RCC->PLLCFGR &= ~RCC_PLLCFGR_PLLN;             // Clear PLLN bits
    RCC->PLLCFGR |= (336 << RCC_PLLCFGR_PLLN_Pos); // Set PLLN to 336 (1 MHz * 336 = 336 MHz)

    RCC->PLLCFGR &= ~RCC_PLLCFGR_PLLP;             // Clear PLLP bits
    RCC->PLLCFGR |= (0 << RCC_PLLCFGR_PLLP_Pos);   // Set PLLP to 2 (336 MHz / 2 = 168 MHz)

    // Enable PLL
    RCC->CR |= RCC_CR_PLLON;                       // Turn on PLL
    while (!(RCC->CR & RCC_CR_PLLRDY));            // Wait for PLL to be ready
}

// Override _write function to redirect printf to UART
int _write(int file, char *data, int len) {
    HAL_UART_Transmit(&huart2, (uint8_t*)data, len, HAL_MAX_DELAY);
    return len;
}

// SysTick Handler for HAL
void SysTick_Handler(void) {
    HAL_IncTick();
}
