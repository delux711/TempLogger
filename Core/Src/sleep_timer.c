#include "sleep_timer.h"

static void TIM2_Init(void) {
    // Povolenie hodin pre TIM2
    RCC->APB1ENR1 |= RCC_APB1ENR1_TIM2EN;

    // Nastavenie prednaèítania na poèítanie v mikrosekundách
    TIM2->PSC = (SystemCoreClock / 1000000) - 1; // Prescaler pre 1 us

    // Nastavenie režimu up-counting
    TIM2->ARR = 0xFFFFFFFF;  // Maximálna hodnota pre 32-bitový èasovaè
    TIM2->CNT = 0;           // Vynulovanie poèítadla

    // Povolenie output compare eventu na kanáli 1
    TIM2->CCMR1 |= TIM_CCMR1_OC1M_0 | TIM_CCMR1_OC1M_1; // Set "toggle on match" mode
    TIM2->CCER |= TIM_CCER_CC1E;  // Povolenie výstupu pre compare na kanáli 1

    TIM2->EGR |= TIM_EGR_UG; // Aktualizácia všetkých registrov
    TIM2->CR1 |= TIM_CR1_CEN; // Povolenie èasovaèa
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
    __WFE();                   // Èakaj na prvý event (vymaže ho)
    */
    while (!(TIM2->SR & TIM_SR_CC1IF)) {
        // __WFE(); // Uspi CPU a èakaj na event
    }

    // Vymazanie flagu po skonèení èakania
    TIM2->SR &= ~TIM_SR_CC1IF;
}

void TIMER_set(uint32_t *timer, uint32_t duration_ms) {
    *timer = HAL_GetTick() + duration_ms;
}

bool TIMER_isExpired(uint32_t timer) {
    return (int32_t)(HAL_GetTick() - timer) >= 0;
}
