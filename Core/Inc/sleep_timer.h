#ifndef SLEEP_TIMER_H
#define SLEEP_TIMER_H

#include "stm32l4xx_hal.h"

// Inicializ�cia casovaca
void SleepTimer_Init(void);

// Uspatie na po�adovan� cas (v mikrosekund�ch)
void SleepTimer_DelayUs(uint32_t delay_us);

void delay_us(uint32_t us);

#endif // SLEEP_TIMER_H
