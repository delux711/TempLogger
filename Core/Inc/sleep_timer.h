#ifndef SLEEP_TIMER_H
#define SLEEP_TIMER_H

#include <stdbool.h>
#include "stm32l4xx_hal.h"

// Inicializ�cia casovaca
void SleepTimer_Init(void);

// Uspatie na po�adovan� cas (v mikrosekund�ch)
void SleepTimer_DelayUs(uint32_t delay_us);

void delay_init(void);
void delay_us(uint32_t us);
void TIMER_set(uint32_t *timer, uint32_t duration_ms);
bool TIMER_isExpired(uint32_t timer);

#endif // SLEEP_TIMER_H
