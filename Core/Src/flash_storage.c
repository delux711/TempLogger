#include "flash_storage.h"
#include "stm32l476g_discovery_qspi.h"  // Tento súbor poskytuje funkcie pre prácu s QSPI

#define QSPI_START_ADDRESS  0x00000000U  // Zaciatok QSPI pamäte
#define QSPI_END_ADDRESS    0x000FFFFFU  // Koniec QSPI pamäte (napr. 1 MB)
#define RECORD_SIZE         4           // Velkost jedného záznamu (napr. 4 bajtov)
#define BLOCK_SIZE          256          // Velkost bloku na cítanie (napr. 256 bajtov)
#define SECTOR_SIZE         0x1000
#define FULL_FILE_SIZE      0x10000
#define QSPI_MEMORY_FULL 0xFFFFFFFFU // Indikuje plnú pamät

static uint32_t freePosition = 0u;
static uint32_t sector_start = 0u;       // Adresa zaèiatku nájdeného sektora
static uint16_t counter = 0u;

static uint32_t find_free_position(void) {
    uint8_t buffer[BLOCK_SIZE];       // Buffer na èítanie blokov
    uint32_t addr = QSPI_START_ADDRESS;

    // H¾adanie sektora s identifikátorom 0x12345678
    while (addr < QSPI_END_ADDRESS) {
        if (BSP_QSPI_Read(buffer, addr, 4) != QSPI_OK) {
            // Spracujte chybu èítania
            return QSPI_END_ADDRESS;
        }

        // Skontrolujeme prvé 4 bajty v sektore
        uint32_t *sector_id = (uint32_t *)&buffer[0];
        if (*sector_id == 0x78563412) {
            sector_start = addr; // Uložíme adresu zaèiatku sektora
            break;
        }

        // Posunieme sa na ïalší sektor
        addr += SECTOR_SIZE; // Ve¾kos sektora je 0x1000
    }

    // Ak nebol sektor nájdený, vrátime chybu
    if (sector_start == 0) {
        return QSPI_END_ADDRESS;
    }

    // H¾adanie vo¾nej pozície v nájdenom sektore
    addr = sector_start;
    while (addr < (sector_start + FULL_FILE_SIZE)) {
        if (BSP_QSPI_Read(buffer, addr, BLOCK_SIZE) != QSPI_OK) {
            // Spracujte chybu èítania
            return QSPI_END_ADDRESS;
        }

        // Skontrolujte každý záznam v bloku
        for (uint32_t i = 4; i < BLOCK_SIZE; i += RECORD_SIZE) {
            uint32_t *record = (uint32_t *)&buffer[i];

            // Ak je hodnota 0xFFFFFFFF, našli sme vo¾nú pozíciu
            if (*record == 0xFFFFFFFF) {
                return addr + i; // Vrátime adresu vo¾ného záznamu
            }
        }

        // Posunieme sa na ïalší blok v rámci sektora
        addr += BLOCK_SIZE;
    }

    // Ak sektor nemá vo¾né miesto, vrátime koniec sektora
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

// Funkcia na zápis teploty do FLASH - true ak OK
bool flash_temperatureSave(float temperature) {
    uint8_t data[RECORD_SIZE];
    int16_t temp = (int16_t)(temperature * 16); // Predpokladáme, že teplota je v 16-bitovom formáte
    
    counter = (freePosition - sector_start) / RECORD_SIZE;

    // Skontrolujeme, èi je vo¾ná pozícia platná a èi sme neprekroèili maximálny poèet zápisov
    if (freePosition >= (sector_start + FULL_FILE_SIZE)) { // 64kB je velkost suboru
        // Vo¾né miesto nie je dostupné, návrat alebo spracovanie chyby
        return false; // Alternatívne môžete vola funkciu na signalizáciu chyby
    }

    // Konverzia teploty na 4-bajtové dáta
    data[0] = (uint8_t)(temp & 0xFF);        // LSB
    data[1] = (uint8_t)((temp >> 8) & 0xFF); // MSB
    data[2] = (counter & 0xFF00u) >> 8u;  // Doplnujúci bajt pre cas alebo inú hodnotu
    data[3] = counter & 0xFFu;            // Doplnujúci bajt pre cas alebo inú hodnotu

    // Zápis do FLASH
    BSP_QSPI_Write(data, freePosition, sizeof(data));

    // Aktualizujeme freePosition na dalšiu volnú pozíciu
    freePosition += RECORD_SIZE;
    return true;
}

uint16_t flash_getCounter(void) {
    return counter;
}

// Funkcia na cítanie teploty z FLASH
float flash_temperatureRead(uint32_t address) {
    uint8_t data[4];
    BSP_QSPI_Read(data, address, sizeof(data));

    int16_t temp = (data[1] << 8) | data[0];
    return (float)temp / 16.0; // Previest na teplotu
}
