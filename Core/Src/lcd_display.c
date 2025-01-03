#include <stdio.h>
#include "lcd_display.h"
#include "stm32l476g_discovery_glass_lcd.h" // Knižnica pre LCD displej

void LCD_Init(void) {
    // Inicializácia interného LCD displeja
    BSP_LCD_GLASS_Init();
}

void LCD_DisplayTemperature(float temperature) {
    char buffer[7]; // Priestor pre zobrazenie, napr. "25.5 C"
    snprintf(buffer, sizeof(buffer), "%.1f C", temperature);
    BSP_LCD_GLASS_DisplayString((uint8_t *)buffer);
}

void LCD_DisplayMessage(const char *message) {
    // Zobrazenie textovej správy (maximálne 6 znakov)
    BSP_LCD_GLASS_DisplayString((uint8_t *)message);
}

void LCD_DisplayCount(uint32_t count) {
    char buffer[7]; // Priestor pre zobrazenie, napr. "Cnt123"
    snprintf(buffer, sizeof(buffer), "Cnt%lu", count);
    BSP_LCD_GLASS_DisplayString((uint8_t *)buffer);
}
