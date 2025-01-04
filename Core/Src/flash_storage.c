#include "flash_storage.h"
#include "stm32l476g_discovery_qspi.h"  // Tento súbor poskytuje funkcie pre prácu s QSPI

#define QSPI_START_ADDRESS  0x00001000U  // Zaciatok QSPI pamäte
#define QSPI_END_ADDRESS    0x000FFFFFU  // Koniec QSPI pamäte (napr. 1 MB)
#define RECORD_SIZE         12           // Velkost jedného záznamu (napr. 12 bajtov)
#define BLOCK_SIZE          256          // Velkost bloku na cítanie (napr. 256 bajtov)
#define QSPI_MEMORY_FULL 0xFFFFFFFFU // Indikuje plnú pamät

static uint32_t freePosition = 0u;

static uint32_t find_free_position(void) {
    uint8_t buffer[BLOCK_SIZE]; // Buffer na cítanie blokov
    uint32_t addr = QSPI_START_ADDRESS;

    while (addr < QSPI_END_ADDRESS) {
        // Precítajte celý blok z pamäte
        if (BSP_QSPI_Read(buffer, addr, BLOCK_SIZE) != QSPI_OK) {
            // Spracujte chybu cítania
            return QSPI_END_ADDRESS; // Vrátime koniec pamäte ako bezpecnú hodnotu
        }

        // Skontrolujte každý záznam v bloku
        for (uint32_t i = 0; i < BLOCK_SIZE; i += RECORD_SIZE) {
            uint32_t *record = (uint32_t *)&buffer[i];

            // Ak je hodnota 0xFFFFFFFF, našli sme volnú pozíciu
            if (*record == 0xFFFFFFFF) {
                return addr + i; // Vrátime adresu volného záznamu
            }
        }

        // Posunte sa na další blok
        addr += BLOCK_SIZE;
    }

    // Ak sa nic nenašlo, pamät je plná
    return QSPI_END_ADDRESS;
}


void flash_temperatureInit(void) {
    if(QSPI_OK != BSP_QSPI_Init()) {
        for(;;);
    }
    freePosition = find_free_position();
}


// Funkcia na zápis teploty do FLASH
void flash_temperatureSave(float temperature) {
    uint8_t data[4];
    int16_t temp = (int16_t)(temperature * 16); // Predpokladáme, že teplota je v 16-bitovom formáte

    // Konverzia teploty na 4-bajtové dáta
    data[0] = (uint8_t)(temp & 0xFF);        // LSB
    data[1] = (uint8_t)((temp >> 8) & 0xFF); // MSB
    data[2] = 0x00;  // Doplnujúci bajt pre cas alebo inú hodnotu
    data[3] = 0x00;  // Doplnujúci bajt pre cas alebo inú hodnotu

    // Zápis do FLASH
    BSP_QSPI_Write(data, freePosition, sizeof(data));

    // Aktualizujeme freePosition na dalšiu volnú pozíciu
    freePosition += sizeof(data);
}

// Funkcia na cítanie teploty z FLASH
float flash_temperatureRead(uint32_t address) {
    uint8_t data[4];
    BSP_QSPI_Read(data, address, sizeof(data));

    int16_t temp = (data[1] << 8) | data[0];
    return (float)temp / 16.0; // Previest na teplotu
}
