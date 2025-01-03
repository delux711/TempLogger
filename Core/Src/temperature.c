#include "temperature.h"
#include "sleep_timer.h"

// GPIO defin�cia pre DS18B20
#define DS18B20_GPIO_PORT GPIOA
#define DS18B20_GPIO_PIN GPIO_PIN_0

// Funkcie pre OneWire protokol
static void OneWire_Reset(void);
static void OneWire_WriteByte(uint8_t data);
static uint8_t OneWire_ReadByte(void);

void Temperature_Init(void) {
    SleepTimer_Init();

    // Nastavenie GPIO pinu pre DS18B20
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    GPIO_InitStruct.Pin = DS18B20_GPIO_PIN;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP; // V�stupn� m�d
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(DS18B20_GPIO_PORT, &GPIO_InitStruct);
	HAL_GPIO_WritePin(DS18B20_GPIO_PORT, DS18B20_GPIO_PIN, GPIO_PIN_SET);
    delay_us(1000); // Dr�at linku n�zko na 1ms
}

float Temperature_Read(void) {
    uint8_t lsb, msb;
    int16_t rawTemperature;
    float temperature;

    // Reset a inicializ�cia senzora
    OneWire_Reset();
    OneWire_WriteByte(0xCC); // Skip ROM command
    OneWire_WriteByte(0x44); // Convert T command (zacne meranie teploty)
    delay_us(750); // Pockat na dokoncenie merania

    // C�tanie teploty
    OneWire_Reset();
    OneWire_WriteByte(0xCC); // Skip ROM command
    OneWire_WriteByte(0xBE); // Read Scratchpad command

    lsb = OneWire_ReadByte();
    msb = OneWire_ReadByte();

    // Senzor neodpoved� spr�vne
    if (lsb == 0xFF && msb == 0xFF) {
        return 0;
    }

    rawTemperature = (msb << 8) | lsb;
    temperature = rawTemperature / 16.0f; // Teplota v �C

    return temperature;
}

static void OneWire_Reset(void) {
    // Reset OneWire linky
    HAL_GPIO_WritePin(DS18B20_GPIO_PORT, DS18B20_GPIO_PIN, GPIO_PIN_RESET);
    delay_us(480); // Dr�at linku n�zko na 480 �s
    HAL_GPIO_WritePin(DS18B20_GPIO_PORT, DS18B20_GPIO_PIN, GPIO_PIN_SET);
    delay_us(70);  // Cakajte na odpoved od senzora
    // Odpoved m��e byt skontrolovan� c�tan�m stavu linky
}

static void OneWire_WriteByte(uint8_t data) {
    for (int i = 0; i < 8; i++) {
        // Poslat jednotliv� bity (LSB first)
        HAL_GPIO_WritePin(DS18B20_GPIO_PORT, DS18B20_GPIO_PIN, GPIO_PIN_RESET);
        if (data & 0x01) {
            delay_us(1);
            HAL_GPIO_WritePin(DS18B20_GPIO_PORT, DS18B20_GPIO_PIN, GPIO_PIN_SET);
        } else {
            delay_us(60);
            HAL_GPIO_WritePin(DS18B20_GPIO_PORT, DS18B20_GPIO_PIN, GPIO_PIN_SET);
        }
        data >>= 1;
        delay_us(1);
    }
}

static uint8_t OneWire_ReadByte(void) {
    uint8_t data = 0;
    for (int i = 0; i < 8; i++) {
        // C�tat jednotliv� bity
        HAL_GPIO_WritePin(DS18B20_GPIO_PORT, DS18B20_GPIO_PIN, GPIO_PIN_RESET);
        delay_us(2); // Zaciatok c�tania
        HAL_GPIO_WritePin(DS18B20_GPIO_PORT, DS18B20_GPIO_PIN, GPIO_PIN_SET);
        if (HAL_GPIO_ReadPin(DS18B20_GPIO_PORT, DS18B20_GPIO_PIN)) {
            data |= (1 << i);
        }
        delay_us(60);
    }
    return data;
}
