#ifndef SLEEP_TIMER_H
#define SLEEP_TIMER_H

#include "stm32l4xx_hal.h"

// Inicializácia casovaca
void SleepTimer_Init(void);

// Uspatie na požadovaný cas (v mikrosekundách)
void SleepTimer_DelayUs(uint32_t delay_us);

void delay_us(uint32_t us);

#endif // SLEEP_TIMER_H
