#include "stm32l4xx_hal.h"
#include "temperature.h"
#include "lcd_display.h"
#include "stm32l476g_discovery_qspi.h"
#include "stm32l476g_discovery.h"
//#include "flash_storage.h"
//#include "usb_mass_storage.h"
//#include "low_power.h"

int loggerAppl_start(void) {
    // Inicializácia GPIO
//    MX_GPIO_Init();

    /*
    LCD_Init();
    LCD_DisplayTemperature(25.5);
    HAL_Delay(2000);
    LCD_DisplayMessage("Hello");
    HAL_Delay(2000);
    LCD_DisplayCount(123);
    HAL_Delay(2000);
    */
    // Inicializácia modulov
    LCD_Init();              // Inicializácia LCD displeja
    Temperature_Init();      // Inicializácia DS18B20
    flash_temperatureInit();
    (void)BSP_JOY_Init(JOY_MODE_GPIO);
    if(JOY_SEL == BSP_JOY_GetState()) {
        if(QSPI_OK != BSP_QSPI_Erase_Chip())
        {
            while(1);
        }
    }
/*    Flash_Init();            // Inicializácia FLASH pamäte
    USB_Init();              // Inicializácia USB
    LowPower_Init();         // Inicializácia režimov nízkej spotreby
*/
    // Pocet teplôt uložených vo FLASH
/*    uint32_t tempCount = Flash_GetTemperatureCount();
    LCD_DisplayCount(tempCount); // Zobrazenie poctu teplôt na LCD
    HAL_Delay(5000);             // Zobrazenie na 5 sekúnd
*/
    // Hlavná slucka
    while (1) {
        // Nacítanie teploty z DS18B20
        float temperature = Temperature_Read();
        LCD_DisplayTemperature(temperature); // Zobrazenie na LCD
/*
        // Ukladanie do FLASH
        if (Flash_IsFull()) {
            LCD_DisplayMessage("FULL");
            HAL_Delay(5000);
            EnterLowPowerMode(); // Prechod do low-power režimu
        } else {
            Flash_SaveTemperature(temperature);
        }

        // Casový interval: 30 sekúnd
        HAL_Delay(30000); */
    }
}

void MX_GPIO_Init(void) {
    // Inicializácia GPIO pre DS18B20, LED, tlacidlá a iné periférie
    __HAL_RCC_GPIOA_CLK_ENABLE();
    __HAL_RCC_GPIOB_CLK_ENABLE();
    GPIO_InitTypeDef GPIO_InitStruct = {0};

    // Príklad: GPIO pin pre DS18B20
    GPIO_InitStruct.Pin = GPIO_PIN_0; // Zvolte vhodný pin
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL; // Interný pull-up je možný
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
}
