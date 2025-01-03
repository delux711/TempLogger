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
    SleepTimer_Init();

    // Nastavenie GPIO pinu pre DS18B20
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    GPIO_InitStruct.Pin = DS18B20_GPIO_PIN;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_OD; // Výstupný mód - open drain
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    HAL_GPIO_Init(DS18B20_GPIO_PORT, &GPIO_InitStruct);
    HAL_GPIO_WritePin(DS18B20_GPIO_PORT, DS18B20_GPIO_PIN, GPIO_PIN_SET);
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
    delay_us(480); // Držat linku nízko na 480 µs
    HAL_GPIO_WritePin(DS18B20_GPIO_PORT, DS18B20_GPIO_PIN, GPIO_PIN_SET);
    delay_us(70);  // Cakajte na odpoved od senzora
    
    uint32_t masterRxTime = DWT->CYCCNT + (SystemCoreClock / 1000000) * 500;  // 480us minimalny cas pre MASTER Rx reset pulz
    
        uint32_t waitToAnswer = DWT->CYCCNT + (SystemCoreClock / 1000000) * 60;  // Nastavenie cielového casu
        while (DWT->CYCCNT < waitToAnswer);
        if(HAL_GPIO_ReadPin(DS18B20_GPIO_PORT, DS18B20_GPIO_PIN) == GPIO_PIN_RESET) {
            retState = true;
        }
    while (DWT->CYCCNT < masterRxTime);
    return retState;
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
            delay_us(44);
            HAL_GPIO_WritePin(DS18B20_GPIO_PORT, DS18B20_GPIO_PIN, GPIO_PIN_SET);
            delay_us(1);
        }
        data >>= 1;
    }
}

static uint8_t OneWire_ReadByte(void) {
    uint8_t data = 0;
    for (int i = 0; i < 8; i++) {
        // Cítat jednotlivé bity
        HAL_GPIO_WritePin(DS18B20_GPIO_PORT, DS18B20_GPIO_PIN, GPIO_PIN_RESET);
        delay_us(2); // Zaciatok cítania
        HAL_GPIO_WritePin(DS18B20_GPIO_PORT, DS18B20_GPIO_PIN, GPIO_PIN_SET);
        if (HAL_GPIO_ReadPin(DS18B20_GPIO_PORT, DS18B20_GPIO_PIN)) {
            data |= (1 << i);
        }
        delay_us(60);
    }
    return data;
}
