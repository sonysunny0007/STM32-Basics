#include "timer.h"
#include <stddef.h>

// Maximum number of timers supported
#define MAX_TIMERS 13

// Array to store timer configurations
static Timer_Config *timer_configurations[MAX_TIMERS] = { NULL };

// Timer handle definitions
TIM_HandleTypeDef htim[MAX_TIMERS];

// Helper function to get timer index based on instance
static int GetTimerIndex(TIM_TypeDef *instance) {
    if (instance == TIM1) return 0;
    if (instance == TIM2) return 1;
    if (instance == TIM3) return 2;
    if (instance == TIM4) return 3;
    if (instance == TIM5) return 4;
//    if (instance == TIM6) return 5;
//    if (instance == TIM7) return 6;
//    if (instance == TIM8) return 7;
    if (instance == TIM9) return 8;
    if (instance == TIM10) return 9;
    if (instance == TIM11) return 10;
    // if (instance == TIM12) return 11;
    // if (instance == TIM13) return 12;
    // if (instance == TIM14) return 13;
    return -1; // Invalid instance
}

// Timer initialization function
void Timer_Init(Timer_Config *timer_config) {
    int index = GetTimerIndex(timer_config->Instance);
    if (index < 0 || index >= MAX_TIMERS) {
        // Invalid timer instance
        return;
    }

    // Enable clock for the timer
    if (timer_config->Instance == TIM1) __HAL_RCC_TIM1_CLK_ENABLE();
    else if (timer_config->Instance == TIM2) __HAL_RCC_TIM2_CLK_ENABLE();
    else if (timer_config->Instance == TIM3) __HAL_RCC_TIM3_CLK_ENABLE();
    else if (timer_config->Instance == TIM4) __HAL_RCC_TIM4_CLK_ENABLE();
    else if (timer_config->Instance == TIM5) __HAL_RCC_TIM5_CLK_ENABLE();
    // else if (timer_config->Instance == TIM6) __HAL_RCC_TIM6_CLK_ENABLE();
    // else if (timer_config->Instance == TIM7) __HAL_RCC_TIM7_CLK_ENABLE();
    // else if (timer_config->Instance == TIM8) __HAL_RCC_TIM8_CLK_ENABLE();
    else if (timer_config->Instance == TIM9) __HAL_RCC_TIM9_CLK_ENABLE();
    else if (timer_config->Instance == TIM10) __HAL_RCC_TIM10_CLK_ENABLE();
    else if (timer_config->Instance == TIM11) __HAL_RCC_TIM11_CLK_ENABLE();
    // else if (timer_config->Instance == TIM12) __HAL_RCC_TIM12_CLK_ENABLE();
    // else if (timer_config->Instance == TIM13) __HAL_RCC_TIM13_CLK_ENABLE();
    // else if (timer_config->Instance == TIM14) __HAL_RCC_TIM14_CLK_ENABLE();

    // Configure the timer base
    htim[index].Instance = timer_config->Instance;
    htim[index].Init.Prescaler = timer_config->Prescaler;
    htim[index].Init.Period = timer_config->Period;
    htim[index].Init.ClockDivision = timer_config->ClockDivision;
    htim[index].Init.CounterMode = TIM_COUNTERMODE_UP;
    htim[index].Init.AutoReloadPreload = timer_config->AutoReloadPreload;

    if (HAL_TIM_Base_Init(&htim[index]) != HAL_OK) {
        // Initialization Error
        while (1);
    }

    // Store the timer configuration in the array
    timer_configurations[index] = timer_config;

    // Enable interrupt for the timer
    uint32_t irq_number;
    if (timer_config->Instance == TIM1) irq_number = TIM1_UP_TIM10_IRQn;
    else if (timer_config->Instance == TIM2) irq_number = TIM2_IRQn;
    else if (timer_config->Instance == TIM3) irq_number = TIM3_IRQn;
    else if (timer_config->Instance == TIM4) irq_number = TIM4_IRQn;
    else if (timer_config->Instance == TIM5) irq_number = TIM5_IRQn;
    // else if (timer_config->Instance == TIM6) irq_number = TIM6_DAC_IRQn;
    // else if (timer_config->Instance == TIM7) irq_number = TIM7_IRQn;
    // else if (timer_config->Instance == TIM8) irq_number = TIM8_UP_TIM13_IRQn;
    else if (timer_config->Instance == TIM9) irq_number = TIM1_BRK_TIM9_IRQn;
    else if (timer_config->Instance == TIM10) irq_number = TIM1_UP_TIM10_IRQn;
    else if (timer_config->Instance == TIM11) irq_number = TIM1_TRG_COM_TIM11_IRQn;
    // else if (timer_config->Instance == TIM12) irq_number = TIM8_BRK_TIM12_IRQn;
    // else if (timer_config->Instance == TIM13) irq_number = TIM8_UP_TIM13_IRQn;
    // else if (timer_config->Instance == TIM14) irq_number = TIM8_TRG_COM_TIM14_IRQn;
    HAL_NVIC_SetPriority(irq_number, 0, 0);
    HAL_NVIC_EnableIRQ(irq_number);
}

// Timer start function
void Timer_Start(Timer_Config *timer_config) {
    int index = GetTimerIndex(timer_config->Instance);
    if (index >= 0 && index < MAX_TIMERS) {
        HAL_TIM_Base_Start_IT(&htim[index]);
    }
}

// Timer stop function
void Timer_Stop(Timer_Config *timer_config) {
    int index = GetTimerIndex(timer_config->Instance);
    if (index >= 0 && index < MAX_TIMERS) {
        HAL_TIM_Base_Stop_IT(&htim[index]);
    }
}

// Timer set callback function
void Timer_SetCallback(TIM_TypeDef *timer_instance, void (*callback)(void)) {
    int index = GetTimerIndex(timer_instance);
    if (index >= 0 && index < MAX_TIMERS) {
        if (timer_configurations[index]) {
            timer_configurations[index]->Callback = callback;
        }
    }
}

// Example IRQ handler (add for all supported timers)
void TIM2_IRQHandler(void) {
    HAL_TIM_IRQHandler(&htim[1]); // TIM2 is at index 1
    if (timer_configurations[1] && timer_configurations[1]->Callback) {
        timer_configurations[1]->Callback();
    }
}

// Example IRQ handler (add for all supported timers)
void TIM3_IRQHandler(void) {
    HAL_TIM_IRQHandler(&htim[2]); // TIM2 is at index 2
    if (timer_configurations[2] && timer_configurations[2]->Callback) {
        timer_configurations[2]->Callback();
    }
}