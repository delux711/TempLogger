#include "temperature.h"
#include "sleep_timer.h"
#include <stdbool.h>

// GPIO definícia pre DS18B20
#define DS18B20_GPIO_PORT GPIOB
#define DS18B20_GPIO_PIN GPIO_PIN_6

// Funkcie pre OneWire protokol
static bool OneWire_Reset(void);
static void OneWire_WriteByte(uint8_t data);
static uint8_t OneWire_ReadByte(void);

void Temperature_Init(void) {
    delay_init();

    // Nastavenie GPIO pinu pre DS18B20
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    GPIO_InitStruct.Pin = DS18B20_GPIO_PIN;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_OD; // Výstupný mód - open drain
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    HAL_GPIO_Init(DS18B20_GPIO_PORT, &GPIO_InitStruct);
    HAL_GPIO_WritePin(DS18B20_GPIO_PORT, DS18B20_GPIO_PIN, GPIO_PIN_SET);
    /* // test pin ak by trebalo
    GPIO_InitStruct.Pin = GPIO_PIN_7;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP; // Výstupný mód - Push-Pull
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_7, GPIO_PIN_SET);
    HAL_GPIO_TogglePin(GPIOB, GPIO_PIN_7);
    */
    delay_us(1000); // Držat linku nízko na 1ms
}

float Temperature_Read(void) {
    uint8_t lsb, msb;
    int16_t rawTemperature;
    float temperature;

    // Reset a inicializácia senzora
    if(!OneWire_Reset())
        return 0u;
    OneWire_WriteByte(0xCC); // Skip ROM command
    OneWire_WriteByte(0x44); // Convert T command (zacne meranie teploty)
    delay_us(750); // Pockat na dokoncenie merania

    // Cítanie teploty
    if(!OneWire_Reset())
        return 0;
    OneWire_WriteByte(0xCC); // Skip ROM command
    OneWire_WriteByte(0xBE); // Read Scratchpad command

    lsb = OneWire_ReadByte();
    msb = OneWire_ReadByte();

    // Senzor neodpovedá správne
    if (lsb == 0xFF && msb == 0xFF) {
        return 0;
    }
    rawTemperature = (msb << 8) | lsb;
    temperature = rawTemperature / 16.0f; // Teplota v °C
    return temperature;
}

static bool OneWire_Reset(void) {
    bool retState = false;

    // Reset OneWire linky
    HAL_GPIO_WritePin(DS18B20_GPIO_PORT, DS18B20_GPIO_PIN, GPIO_PIN_RESET);
    delay_us(480); // Držte linku nízko na 480 µs
    HAL_GPIO_WritePin(DS18B20_GPIO_PORT, DS18B20_GPIO_PIN, GPIO_PIN_SET);

    // Nastavte èas na èakanie
    uint32_t startTime = TIM2->CNT;
    uint32_t waitToAnswerEnd = startTime + (SystemCoreClock / 1000000) * 240;  // 240 µs
    uint32_t waitToReleaseEnd = startTime + (SystemCoreClock / 1000000) * 480; // 480 µs

    // Èakajte na zaèiatok odpovede (po 15 µs)
    delay_us(15);

    // Senzor drží linku na 0 medzi 60–240 µs
    while (TIM2->CNT < waitToAnswerEnd) {
        if (HAL_GPIO_ReadPin(DS18B20_GPIO_PORT, DS18B20_GPIO_PIN) == GPIO_PIN_RESET) {
            retState = true; // Senzor odpovedá
            break; // Odpoveï bola detegovaná, ukonèite cyklus
        }
    }
    if (retState) {
        // Senzor drží linku na 0 medzi 60–240 µs
        while (TIM2->CNT < waitToReleaseEnd) {
            if (HAL_GPIO_ReadPin(DS18B20_GPIO_PORT, DS18B20_GPIO_PIN) == GPIO_PIN_SET) {
                break; // senzor uvolnil linku, komunikácia bola úspešná
            }
        }
    }

    return retState; // Vrátte výsledok
}

static void OneWire_WriteByte(uint8_t data) {
    for (int i = 0; i < 8; i++) {
        // Poslat jednotlivé bity (LSB first)
        HAL_GPIO_WritePin(DS18B20_GPIO_PORT, DS18B20_GPIO_PIN, GPIO_PIN_RESET);
        delay_us(15);
        if (data & 0x01) {
            HAL_GPIO_WritePin(DS18B20_GPIO_PORT, DS18B20_GPIO_PIN, GPIO_PIN_SET);
            delay_us(45);
        } else {
            delay_us(42);
            HAL_GPIO_WritePin(DS18B20_GPIO_PORT, DS18B20_GPIO_PIN, GPIO_PIN_SET);
            delay_us(3);
        }
        data >>= 1;
    }
}

static uint8_t OneWire_ReadByte(void) {
    uint8_t data = 0;
    for (int i = 0; i < 8; i++) {
        // Cítat jednotlivé bity
        HAL_GPIO_WritePin(DS18B20_GPIO_PORT, DS18B20_GPIO_PIN, GPIO_PIN_RESET);
        delay_us(10); // Zaciatok cítania
        HAL_GPIO_WritePin(DS18B20_GPIO_PORT, DS18B20_GPIO_PIN, GPIO_PIN_SET);
        delay_us(10);
        if (HAL_GPIO_ReadPin(DS18B20_GPIO_PORT, DS18B20_GPIO_PIN)) {
            data |= (1 << i);
        }
        delay_us(40);
    }
    return data;
}
