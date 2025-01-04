#include "flash_storage.h"
#include "stm32l476g_discovery_qspi.h"  // Tento s�bor poskytuje funkcie pre pr�cu s QSPI

#define QSPI_START_ADDRESS  0x00001000U  // Zaciatok QSPI pam�te
#define QSPI_END_ADDRESS    0x000FFFFFU  // Koniec QSPI pam�te (napr. 1 MB)
#define RECORD_SIZE         12           // Velkost jedn�ho z�znamu (napr. 12 bajtov)
#define BLOCK_SIZE          256          // Velkost bloku na c�tanie (napr. 256 bajtov)
#define QSPI_MEMORY_FULL 0xFFFFFFFFU // Indikuje pln� pam�t

static uint32_t freePosition = 0u;

static uint32_t find_free_position(void) {
    uint8_t buffer[BLOCK_SIZE]; // Buffer na c�tanie blokov
    uint32_t addr = QSPI_START_ADDRESS;

    while (addr < QSPI_END_ADDRESS) {
        // Prec�tajte cel� blok z pam�te
        if (BSP_QSPI_Read(buffer, addr, BLOCK_SIZE) != QSPI_OK) {
            // Spracujte chybu c�tania
            return QSPI_END_ADDRESS; // Vr�time koniec pam�te ako bezpecn� hodnotu
        }

        // Skontrolujte ka�d� z�znam v bloku
        for (uint32_t i = 0; i < BLOCK_SIZE; i += RECORD_SIZE) {
            uint32_t *record = (uint32_t *)&buffer[i];

            // Ak je hodnota 0xFFFFFFFF, na�li sme voln� poz�ciu
            if (*record == 0xFFFFFFFF) {
                return addr + i; // Vr�time adresu voln�ho z�znamu
            }
        }

        // Posunte sa na dal�� blok
        addr += BLOCK_SIZE;
    }

    // Ak sa nic nena�lo, pam�t je pln�
    return QSPI_END_ADDRESS;
}


void flash_temperatureInit(void) {
    if(QSPI_OK != BSP_QSPI_Init()) {
        for(;;);
    }
    freePosition = find_free_position();
}


// Funkcia na z�pis teploty do FLASH
void flash_temperatureSave(float temperature) {
    uint8_t data[4];
    int16_t temp = (int16_t)(temperature * 16); // Predpoklad�me, �e teplota je v 16-bitovom form�te

    // Konverzia teploty na 4-bajtov� d�ta
    data[0] = (uint8_t)(temp & 0xFF);        // LSB
    data[1] = (uint8_t)((temp >> 8) & 0xFF); // MSB
    data[2] = 0x00;  // Doplnuj�ci bajt pre cas alebo in� hodnotu
    data[3] = 0x00;  // Doplnuj�ci bajt pre cas alebo in� hodnotu

    // Z�pis do FLASH
    BSP_QSPI_Write(data, freePosition, sizeof(data));

    // Aktualizujeme freePosition na dal�iu voln� poz�ciu
    freePosition += sizeof(data);
}

// Funkcia na c�tanie teploty z FLASH
float flash_temperatureRead(uint32_t address) {
    uint8_t data[4];
    BSP_QSPI_Read(data, address, sizeof(data));

    int16_t temp = (data[1] << 8) | data[0];
    return (float)temp / 16.0; // Previest na teplotu
}
