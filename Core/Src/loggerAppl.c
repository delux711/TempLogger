#include "stm32l4xx_hal.h"
#include "temperature.h"
#include "lcd_display.h"
#include "stm32l476g_discovery_qspi.h"
#include "stm32l476g_discovery.h"
#include "flash_storage.h"
#include "main.h"
#include <stdbool.h>
//#include "usb_mass_storage.h"
//#include "low_power.h"

static void logger_GPIO_Init(void);

int loggerAppl_start(void) {
    // Inicializácia GPIO
    logger_GPIO_Init();

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
    // Hlavná sluèka
    bool showMessage = false;               // Indikuje, èi sa má zobrazova správa
    bool usbPreviouslyConnected = false;    // Stav predchádzajúceho pripojenia USB
    uint32_t showMessageEndTime = 0;        // Èas, kedy skonèí zobrazovanie správy
    uint32_t saveTimeEnd = 0;               // Èas pre budúce uloženie teploty
    uint32_t counterDisplayEnd = 0;         // Èas pre ukonèenie zobrazenia counter
    uint32_t counterPeriodEnd = 0;          // Èas pre zaèiatok nového obdobia counter

    while (1) {
        // Kontrola, èi je USB pripojené
        bool usbConnected = (HAL_GPIO_ReadPin(USB_VBUS_GPIO_Port, USB_VBUS_Pin) == GPIO_PIN_SET);

        // Ak sa USB novo pripojilo
        if (usbConnected && !usbPreviouslyConnected) {
            LCD_DisplayMessage("-USB- ");    // Zobrazenie správy USB
            showMessage = true;
            showMessageEndTime = DWT->CYCCNT + (SystemCoreClock / 1000) * 5000; // 5 sekúnd
        }
        
        // Ak sa USB odpojilo
        if (!usbConnected && usbPreviouslyConnected) {
            // Akcia pri odpojení USB
            LCD_DisplayMessage("NO USB"); // Zobrazenie správy pri odpojení
            showMessage = true;
            showMessageEndTime = DWT->CYCCNT + (SystemCoreClock / 1000) * 5000; // 5 sekúnd
            flash_updateCounter();
        }

        // Ak uplynulo 5 sekúnd od zobrazenia USB správy
        if (showMessage && (int32_t)(DWT->CYCCNT - showMessageEndTime) >= 0) {
            showMessage = false;
        }

        // Ak sa má zobrazova hodnota `flash_getCounter`
        if (!showMessage) {
            if ((int32_t)(DWT->CYCCNT - counterPeriodEnd) >= 0) {
                counterPeriodEnd = DWT->CYCCNT + (SystemCoreClock / 1000) * 5000; // Každých 5 sekúnd
                LCD_DisplayCount(flash_getCounter());
                counterDisplayEnd = DWT->CYCCNT + (SystemCoreClock / 1000) * 1000; // Zobrazenie na 1 sekundu
                showMessage = true;
            } else if ((int32_t)(DWT->CYCCNT - counterDisplayEnd) >= 0) {
                // Ak uplynula 1 sekunda od zobrazenia counter
                float temperature = Temperature_Read(); // Naèítanie teploty
                LCD_DisplayTemperature(temperature);   // Zobrazenie teploty na LCD

                // Ak USB nie je pripojené, zapisuje sa teplota do FLASH každé 4 sekundy
                if (!usbConnected && (int32_t)(DWT->CYCCNT - saveTimeEnd) >= 0) {
                    saveTimeEnd = DWT->CYCCNT + (SystemCoreClock / 1000) * 4000; // Každé 4 sekundy
                    if (!flash_temperatureSave(temperature)) {
                        LCD_DisplayMessage("-FULL-");
                        showMessage = true;
                        showMessageEndTime = DWT->CYCCNT + (SystemCoreClock / 1000) * 1000; // 1 sekunda
                    }
                }
            }
        }

        // Aktualizácia stavu USB
        usbPreviouslyConnected = usbConnected;
    }
}

static void logger_GPIO_Init(void) {
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
