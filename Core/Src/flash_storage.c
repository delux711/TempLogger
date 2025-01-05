#include "flash_storage.h"
#include "stm32l476g_discovery_qspi.h"  // Tento s�bor poskytuje funkcie pre pr�cu s QSPI

#define QSPI_START_ADDRESS  0x00000000U  // Zaciatok QSPI pam�te
#define QSPI_END_ADDRESS    0x000FFFFFU  // Koniec QSPI pam�te (napr. 1 MB)
#define RECORD_SIZE         4           // Velkost jedn�ho z�znamu (napr. 4 bajtov)
#define BLOCK_SIZE          256          // Velkost bloku na c�tanie (napr. 256 bajtov)
#define SECTOR_SIZE         0x1000
#define FULL_FILE_SIZE      0x10000
#define QSPI_MEMORY_FULL 0xFFFFFFFFU // Indikuje pln� pam�t

static uint32_t freePosition = 0u;
static uint32_t sector_start = 0u;       // Adresa za�iatku n�jden�ho sektora
static uint16_t counter = 0u;

static uint32_t find_free_position(void) {
    uint8_t buffer[BLOCK_SIZE];       // Buffer na ��tanie blokov
    uint32_t addr = QSPI_START_ADDRESS;

    // H�adanie sektora s identifik�torom 0x12345678
    while (addr < QSPI_END_ADDRESS) {
        if (BSP_QSPI_Read(buffer, addr, 4) != QSPI_OK) {
            // Spracujte chybu ��tania
            return QSPI_END_ADDRESS;
        }

        // Skontrolujeme prv� 4 bajty v sektore
        uint32_t *sector_id = (uint32_t *)&buffer[0];
        if (*sector_id == 0x78563412) {
            sector_start = addr; // Ulo��me adresu za�iatku sektora
            break;
        }

        // Posunieme sa na �al�� sektor
        addr += SECTOR_SIZE; // Ve�kos� sektora je 0x1000
    }

    // Ak nebol sektor n�jden�, vr�time chybu
    if (sector_start == 0) {
        return QSPI_END_ADDRESS;
    }

    // H�adanie vo�nej poz�cie v n�jdenom sektore
    addr = sector_start;
    while (addr < (sector_start + FULL_FILE_SIZE)) {
        if (BSP_QSPI_Read(buffer, addr, BLOCK_SIZE) != QSPI_OK) {
            // Spracujte chybu ��tania
            return QSPI_END_ADDRESS;
        }

        // Skontrolujte ka�d� z�znam v bloku
        for (uint32_t i = 4; i < BLOCK_SIZE; i += RECORD_SIZE) {
            uint32_t *record = (uint32_t *)&buffer[i];

            // Ak je hodnota 0xFFFFFFFF, na�li sme vo�n� poz�ciu
            if (*record == 0xFFFFFFFF) {
                return addr + i; // Vr�time adresu vo�n�ho z�znamu
            }
        }

        // Posunieme sa na �al�� blok v r�mci sektora
        addr += BLOCK_SIZE;
    }

    // Ak sektor nem� vo�n� miesto, vr�time koniec sektora
    return sector_start + 0x1000;
}

void flash_temperatureInit(void) {
    if(QSPI_OK != BSP_QSPI_Init()) {
        for(;;);
    }
    flash_updateCounter();
}

void flash_updateCounter(void) {
    freePosition = find_free_position();    
}

// Funkcia na z�pis teploty do FLASH - true ak OK
bool flash_temperatureSave(float temperature) {
    uint8_t data[RECORD_SIZE];
    int16_t temp = (int16_t)(temperature * 16); // Predpoklad�me, �e teplota je v 16-bitovom form�te
    
    counter = (freePosition - sector_start) / RECORD_SIZE;

    // Skontrolujeme, �i je vo�n� poz�cia platn� a �i sme neprekro�ili maxim�lny po�et z�pisov
    if (freePosition >= (sector_start + FULL_FILE_SIZE)) { // 64kB je velkost suboru
        // Vo�n� miesto nie je dostupn�, n�vrat alebo spracovanie chyby
        return false; // Alternat�vne m��ete vola� funkciu na signaliz�ciu chyby
    }

    // Konverzia teploty na 4-bajtov� d�ta
    data[0] = (uint8_t)(temp & 0xFF);        // LSB
    data[1] = (uint8_t)((temp >> 8) & 0xFF); // MSB
    data[2] = (counter & 0xFF00u) >> 8u;  // Doplnuj�ci bajt pre cas alebo in� hodnotu
    data[3] = counter & 0xFFu;            // Doplnuj�ci bajt pre cas alebo in� hodnotu

    // Z�pis do FLASH
    BSP_QSPI_Write(data, freePosition, sizeof(data));

    // Aktualizujeme freePosition na dal�iu voln� poz�ciu
    freePosition += RECORD_SIZE;
    return true;
}

uint16_t flash_getCounter(void) {
    return counter;
}

// Funkcia na c�tanie teploty z FLASH
float flash_temperatureRead(uint32_t address) {
    uint8_t data[4];
    BSP_QSPI_Read(data, address, sizeof(data));

    int16_t temp = (data[1] << 8) | data[0];
    return (float)temp / 16.0; // Previest na teplotu
}
