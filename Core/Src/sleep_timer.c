#include "sleep_timer.h"

static TIM_HandleTypeDef htim;

// Inicializ�cia casovaca
void SleepTimer_Init(void) {
    __HAL_RCC_TIM2_CLK_ENABLE(); // Zapnutie hodinov�ho sign�lu pre TIM2

    htim.Instance = TIM2;
    htim.Init.Prescaler = (SystemCoreClock / 1000000) - 1;  // Nastavenie na 1 MHz (1 �s)
    htim.Init.CounterMode = TIM_COUNTERMODE_UP;
    htim.Init.Period = 0xFFFF;  // Maxim�lna hodnota
    htim.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;

    HAL_TIM_Base_Init(&htim);
}

// Uspatie na po�adovan� cas (v mikrosekund�ch)
void SleepTimer_DelayUs(uint32_t delay_us) {
    if (delay_us > 0xFFFF) return; // Skontrolujeme, ci cas nepresahuje maxim�lnu hodnotu casovaca

    __HAL_TIM_SET_COUNTER(&htim, 0);         // Resetovanie poc�tadla
    HAL_TIM_Base_Start(&htim);              // Spustenie casovaca

    while (__HAL_TIM_GET_COUNTER(&htim) < delay_us) {
        __WFE(); // Cak� na udalost alebo prejde do sp�nku
    }

    HAL_TIM_Base_Stop(&htim);               // Zastavenie casovaca
}

void delay_us(uint32_t us) {
    uint32_t count = (SystemCoreClock / 1000000) * us / 3; // Prepocet pre 16 MHz
    while (count--) {
        __NOP();  // No operation (pr�zdna in�trukcia)
    }
}
