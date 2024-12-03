#ifndef TIMER_H
#define TIMER_H

#include "stm32f4xx_hal.h"

// Timer configuration structure
typedef struct {
    TIM_TypeDef *Instance;                 // Timer instance (e.g., TIM2, TIM3)
    uint32_t Prescaler;                    // Timer prescaler value
    uint32_t Period;                       // Timer period value
    uint32_t ClockDivision;                // Clock division
    uint32_t AutoReloadPreload;            // Auto-reload preload setting
    void (*Callback)(void);                // Timer interrupt callback
} Timer_Config;

// Function prototypes
void Timer_Init(Timer_Config *timer_config);
void Timer_Start(Timer_Config *timer_config);
void Timer_Stop(Timer_Config *timer_config);
void Timer_SetCallback(TIM_TypeDef *timer_instance, void (*callback)(void));

#endif // TIMER_H
