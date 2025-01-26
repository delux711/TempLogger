#include "stm32l4xx_hal.h"
#include "temperature.h"
#include "lcd_display.h"
#include "stm32l476g_discovery_qspi.h"
#include "stm32l476g_discovery.h"
#include "flash_storage.h"
#include "sleep_timer.h"
#include "main.h"
#include "stm32l4xx.h"
#include "gpio.h"
#include <stdbool.h>
//#include "usb_mass_storage.h"
//#include "low_power.h"

static void logger_GPIO_Init(void);
static void checkButtonAndErase(void);
static void flash_resetMemory(void);

int loggerAppl_start(void) {
    // Inicializ�cia GPIO
    logger_GPIO_Init();

    // Inicializ�cia modulov
    LCD_Init();              // Inicializ�cia LCD displeja
    LCD_DisplayMessage("BOOT..");
    Temperature_Init();      // Inicializ�cia DS18B20
    flash_temperatureInit();
    while(HAL_OK != BSP_JOY_Init(JOY_MODE_GPIO));
    checkButtonAndErase();

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
            TIMER_set(&showMessageEndTime, 5000); // 5 sek�nd
        }

        // Ak sa USB odpojilo
        if (!usbConnected && usbPreviouslyConnected) {
            // Akcia pri odpojen� USB
            LCD_DisplayMessage("NO USB"); // Zobrazenie spr�vy pri odpojen�
            showMessage = true;
            TIMER_set(&showMessageEndTime, 2000); // 2 sek�nd
            flash_updateCounter();
        }

        // Ak uplynulo 5 sek�nd od zobrazenia USB spr�vy
        if (showMessage && TIMER_isExpired(showMessageEndTime)) {
            showMessage = false;
        }

        // Ak sa m� zobrazova� hodnota `flash_getCounter`
        if (!showMessage) {
            if (TIMER_isExpired(counterPeriodEnd)) {
                TIMER_set(&counterPeriodEnd, 5000); // Ka�d�ch 5 sek�nd
                LCD_DisplayCount(flash_getCounter());
                TIMER_set(&counterDisplayEnd, 1000); // Zobrazenie na 1 sekundu
                showMessage = true;
            } else if (TIMER_isExpired(counterDisplayEnd)) {
                // Ak uplynula 1 sekunda od zobrazenia counter
                float temperature = Temperature_Read(); // Na��tanie teploty
                LCD_DisplayTemperature(temperature);   // Zobrazenie teploty na LCD

                // Ak USB nie je pripojen�, zapisuje sa teplota do FLASH ka�d� 30 sek�nd
                if (!usbConnected && TIMER_isExpired(saveTimeEnd)) {
                    TIMER_set(&saveTimeEnd, 30000); // Zapis kazdych 30 sek�nd
                    if (!flash_temperatureSave(temperature)) {
                        LCD_DisplayMessage("-FULL-");
                        showMessage = true;
                        TIMER_set(&showMessageEndTime, 1000); // 1 sekunda
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

    MX_GPIO_Init();         // init LED
}

static void flash_resetMemory(void) {
    // Zastavte v�etky z�visl� �lohy (napr. USB alebo ��tanie)
    HAL_NVIC_DisableIRQ(OTG_FS_IRQn);

    LCD_DisplayMessage("Erase-");
    // Vyma�te FLASH pam�
    if (BSP_QSPI_Erase_Chip() != QSPI_OK) {
        HAL_GPIO_WritePin(LED_RED_GPIO_Port, LED_RED_Pin, GPIO_PIN_SET);
        LCD_DisplayMessage("-Fail-");
        for(;;);
    }
    flash_updateCounter();
    // Reaktivujte preru�enia
    HAL_NVIC_EnableIRQ(OTG_FS_IRQn);
}

static void checkButtonAndErase(void) {
    HAL_Delay(100); // Stabiliz�cia tla�idla
    if(JOY_SEL == BSP_JOY_GetState()) {
        HAL_GPIO_WritePin(LED_GREEN_GPIO_Port, LED_GREEN_Pin, GPIO_PIN_SET);
        flash_resetMemory();
        HAL_GPIO_WritePin(LED_GREEN_GPIO_Port, LED_GREEN_Pin, GPIO_PIN_RESET);
    }
}
