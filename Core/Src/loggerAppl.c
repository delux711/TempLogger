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
    // Inicializ�cia GPIO
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
    // Inicializ�cia modulov
    LCD_Init();              // Inicializ�cia LCD displeja
    Temperature_Init();      // Inicializ�cia DS18B20
    flash_temperatureInit();
    (void)BSP_JOY_Init(JOY_MODE_GPIO);
    if(JOY_SEL == BSP_JOY_GetState()) {
        if(QSPI_OK != BSP_QSPI_Erase_Chip())
        {
            while(1);
        }
    }
/*    Flash_Init();            // Inicializ�cia FLASH pam�te
    USB_Init();              // Inicializ�cia USB
    LowPower_Init();         // Inicializ�cia re�imov n�zkej spotreby
*/
    // Pocet tepl�t ulo�en�ch vo FLASH
/*    uint32_t tempCount = Flash_GetTemperatureCount();
    LCD_DisplayCount(tempCount); // Zobrazenie poctu tepl�t na LCD
    HAL_Delay(5000);             // Zobrazenie na 5 sek�nd
*/
    // Hlavn� slu�ka
    bool showMessage = false;               // Indikuje, �i sa m� zobrazova� spr�va
    bool usbPreviouslyConnected = false;    // Stav predch�dzaj�ceho pripojenia USB
    uint32_t showMessageEndTime = 0;        // �as, kedy skon�� zobrazovanie spr�vy
    uint32_t saveTimeEnd = 0;               // �as pre bud�ce ulo�enie teploty
    uint32_t counterDisplayEnd = 0;         // �as pre ukon�enie zobrazenia counter
    uint32_t counterPeriodEnd = 0;          // �as pre za�iatok nov�ho obdobia counter

    while (1) {
        // Kontrola, �i je USB pripojen�
        bool usbConnected = (HAL_GPIO_ReadPin(USB_VBUS_GPIO_Port, USB_VBUS_Pin) == GPIO_PIN_SET);

        // Ak sa USB novo pripojilo
        if (usbConnected && !usbPreviouslyConnected) {
            LCD_DisplayMessage("-USB- ");    // Zobrazenie spr�vy USB
            showMessage = true;
            showMessageEndTime = DWT->CYCCNT + (SystemCoreClock / 1000) * 5000; // 5 sek�nd
        }
        
        // Ak sa USB odpojilo
        if (!usbConnected && usbPreviouslyConnected) {
            // Akcia pri odpojen� USB
            LCD_DisplayMessage("NO USB"); // Zobrazenie spr�vy pri odpojen�
            showMessage = true;
            showMessageEndTime = DWT->CYCCNT + (SystemCoreClock / 1000) * 5000; // 5 sek�nd
            flash_updateCounter();
        }

        // Ak uplynulo 5 sek�nd od zobrazenia USB spr�vy
        if (showMessage && (int32_t)(DWT->CYCCNT - showMessageEndTime) >= 0) {
            showMessage = false;
        }

        // Ak sa m� zobrazova� hodnota `flash_getCounter`
        if (!showMessage) {
            if ((int32_t)(DWT->CYCCNT - counterPeriodEnd) >= 0) {
                counterPeriodEnd = DWT->CYCCNT + (SystemCoreClock / 1000) * 5000; // Ka�d�ch 5 sek�nd
                LCD_DisplayCount(flash_getCounter());
                counterDisplayEnd = DWT->CYCCNT + (SystemCoreClock / 1000) * 1000; // Zobrazenie na 1 sekundu
                showMessage = true;
            } else if ((int32_t)(DWT->CYCCNT - counterDisplayEnd) >= 0) {
                // Ak uplynula 1 sekunda od zobrazenia counter
                float temperature = Temperature_Read(); // Na��tanie teploty
                LCD_DisplayTemperature(temperature);   // Zobrazenie teploty na LCD

                // Ak USB nie je pripojen�, zapisuje sa teplota do FLASH ka�d� 4 sekundy
                if (!usbConnected && (int32_t)(DWT->CYCCNT - saveTimeEnd) >= 0) {
                    saveTimeEnd = DWT->CYCCNT + (SystemCoreClock / 1000) * 4000; // Ka�d� 4 sekundy
                    if (!flash_temperatureSave(temperature)) {
                        LCD_DisplayMessage("-FULL-");
                        showMessage = true;
                        showMessageEndTime = DWT->CYCCNT + (SystemCoreClock / 1000) * 1000; // 1 sekunda
                    }
                }
            }
        }

        // Aktualiz�cia stavu USB
        usbPreviouslyConnected = usbConnected;
    }
}

static void logger_GPIO_Init(void) {
    // Inicializ�cia GPIO pre DS18B20, LED, tlacidl� a in� perif�rie
    __HAL_RCC_GPIOA_CLK_ENABLE();
    __HAL_RCC_GPIOB_CLK_ENABLE();
    GPIO_InitTypeDef GPIO_InitStruct = {0};

    // Pr�klad: GPIO pin pre DS18B20
    GPIO_InitStruct.Pin = GPIO_PIN_0; // Zvolte vhodn� pin
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL; // Intern� pull-up je mo�n�
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
}
