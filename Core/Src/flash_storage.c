#include "flash_storage.h"
#include "stm32l476g_discovery_qspi.h"  // Tento s�bor poskytuje funkcie pre pr�cu s QSPI

static uint32_t nextEmptyAddr = 0u;
void flash_temperatureInit(void) {
    uint32_t data;
    if(QSPI_OK != BSP_QSPI_Init()) {
        for(;;);
    }

    for(uint32_t i = 0u; i < 0x4000; i++) { // 128Kb
        (void)BSP_QSPI_Read((uint8_t*)&data, i, sizeof(uint32_t));
        if(data != 0xFFFFu) {
            nextEmptyAddr = i;
            break;
        }
    }
    (void)nextEmptyAddr;
}

// Funkcia na z�pis teploty do FLASH
void flash_temperatureSave(uint32_t address, float temperature) {
    uint8_t data[4];
    int16_t temp = (int16_t)(temperature * 16); // Predpoklad�me, �e teplota je v 16-bitovom form�te

    // Konverzia teploty na 4-bajtov� d�ta
    data[0] = (uint8_t)(temp & 0xFF);        // LSB
    data[1] = (uint8_t)((temp >> 8) & 0xFF); // MSB
    data[2] = 0x00;  // Doplnuj�ci bajt pre cas alebo in� hodnotu
    data[3] = 0x00;  // Doplnuj�ci bajt pre cas alebo in� hodnotu

    // Z�pis do FLASH
    BSP_QSPI_Write(data, address, sizeof(data));
}

// Funkcia na c�tanie teploty z FLASH
float flash_temperatureRead(uint32_t address) {
    uint8_t data[4];
    BSP_QSPI_Read(data, address, sizeof(data));

    int16_t temp = (data[1] << 8) | data[0];
    return (float)temp / 16.0; // Previest na teplotu
}
