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
    // Hlavn� slucka
    bool showUsb = false;      // Indikuje, ci sa m� zobrazovat spr�va USB
    bool usbPreviouslyConnected = false; // Stav predch�dzaj�ceho pripojenia USB
    uint32_t usbDisplayEndTime = 0;      // Cas, kedy skonc� zobrazovanie spr�vy USB
    uint32_t saveTimeEnd = 0u;
    while (1) {
        // Kontrola, ci je USB pripojen�
        bool usbConnected = (HAL_GPIO_ReadPin(USB_VBUS_GPIO_Port, USB_VBUS_Pin) == GPIO_PIN_SET);
        // Ak sa USB novo pripojilo
        if (usbConnected && !usbPreviouslyConnected) {
            showUsb = true;
            usbDisplayEndTime = DWT->CYCCNT + (SystemCoreClock / 1000000) * 5000000; // Nastavenie konca casu pre USB spr�vu
            LCD_DisplayMessage("-USB- ");  // Zobrazenie spr�vy USB
        }
        // Ak uplynulo 5 sek�nd od zobrazenia USB spr�vy
        if (showUsb && (int32_t)(DWT->CYCCNT - usbDisplayEndTime) >= 0) {
            showUsb = false;
        }
        // Ak sa m� zobrazovat teplota
        if (!showUsb) {
            float temperature = Temperature_Read();    // Nac�tanie teploty
            LCD_DisplayTemperature(temperature);      // Zobrazenie teploty na LCD
            if (!usbConnected && (int32_t)(DWT->CYCCNT - saveTimeEnd) >= 0) {
                saveTimeEnd = DWT->CYCCNT + (SystemCoreClock / 1000000) * 4000000; // Nastavenie konca casu pre USB spr�vu
                flash_temperatureSave(temperature);
            }
        }
        // Aktualiz�cia stavu USB
        usbPreviouslyConnected = usbConnected;

/*
        // Ukladanie do FLASH
        if (Flash_IsFull()) {
            LCD_DisplayMessage("FULL");
            HAL_Delay(5000);
            EnterLowPowerMode(); // Prechod do low-power re�imu
        } else {
            Flash_SaveTemperature(temperature);
        }

        // Casov� interval: 30 sek�nd
        HAL_Delay(30000); */
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
