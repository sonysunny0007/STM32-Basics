#ifndef SYSCLOCK_H
#define SYSCLOCK_H

#include "stm32f4xx_hal.h"


//System clock frequency initialization function 168Mhz
void SystemClock_Config(void);

//PLL initialization function
void Configure_PLL(void);

//system cock frequency check function
uint32_t Get_SYSCLK_Frequency(void);

#endif