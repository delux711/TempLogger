#include "sleep_timer.h"

static void TIM2_Init(void) {
    // Povolenie hodin pre TIM2
    RCC->APB1ENR1 |= RCC_APB1ENR1_TIM2EN;

    // Nastavenie predna��tania na po��tanie v mikrosekund�ch
    TIM2->PSC = (SystemCoreClock / 1000000) - 1; // Prescaler pre 1 us

    // Nastavenie re�imu up-counting
    TIM2->ARR = 0xFFFFFFFF;  // Maxim�lna hodnota pre 32-bitov� �asova�
    TIM2->CNT = 0;           // Vynulovanie po��tadla

    // Povolenie output compare eventu na kan�li 1
    TIM2->CCMR1 |= TIM_CCMR1_OC1M_0 | TIM_CCMR1_OC1M_1; // Set "toggle on match" mode
    TIM2->CCER |= TIM_CCER_CC1E;  // Povolenie v�stupu pre compare na kan�li 1

    TIM2->EGR |= TIM_EGR_UG; // Aktualiz�cia v�etk�ch registrov
    TIM2->CR1 |= TIM_CR1_CEN; // Povolenie �asova�a
}

void delay_init(void) {
    TIM2_Init();
}

void delay_us(uint32_t us) {
    // Vynulovanie flagov eventu
    TIM2->SR &= ~TIM_SR_CC1IF; // Vymazanie compare interrupt flagu
    TIM2->CCR1 = TIM2->CNT + us;
    /*
    __SEV();                   // Nastavenie event flagu
    __WFE();                   // �akaj na prv� event (vyma�e ho)
    */
    while (!(TIM2->SR & TIM_SR_CC1IF)) {
        // __WFE(); // Uspi CPU a �akaj na event
    }

    // Vymazanie flagu po skon�en� �akania
    TIM2->SR &= ~TIM_SR_CC1IF;
}

void TIMER_set(uint32_t *timer, uint32_t duration_ms) {
    *timer = HAL_GetTick() + duration_ms;
}

bool TIMER_isExpired(uint32_t timer) {
    return (int32_t)(HAL_GetTick() - timer) >= 0;
}
