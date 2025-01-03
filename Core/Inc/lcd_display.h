#ifndef LCD_DISPLAY_H
#define LCD_DISPLAY_H

#include "stm32l4xx_hal.h"
#include "stm32l4xx_hal_lcd.h"

// Inicializácia LCD displeja
void LCD_Init(void);

// Zobrazenie teploty na LCD
void LCD_DisplayTemperature(float temperature);

// Zobrazenie správy na LCD
void LCD_DisplayMessage(const char *message);

// Zobrazenie poctu uložených teplôt
void LCD_DisplayCount(uint32_t count);

#endif // LCD_DISPLAY_H
