#include "flash_storage.h"
#include "stm32l476g_discovery_qspi.h"  // Tento súbor poskytuje funkcie pre prácu s QSPI

#define QSPI_START_ADDRESS  0x00000000U  // Zaèiatok QSPI pamäte
#define RECORD_SIZE         4           // Ve¾kos jedného záznamu (napr. 4 bajty)
#define BLOCK_SIZE          256         // Velkost bloku na cítanie (napr. 256 bajtov)
#define SECTOR_SIZE         0x1000      // Ve¾kos sektora (4 kB)
#define FULL_FILE_SIZE      0x10000     // Ve¾kos "súboru" (64 kB)
#define QSPI_MEMORY_FULL 0xFFFFFFFFU    // Indikuje plnú pamät
#define PATTERN_BASE        0x12345670U // Základnı identifikátor pre sektory
#define PATTERN_END         0x1234567FU // Poslednı platnı identifikátor

static uint16_t counter = 0u;
static uint32_t freePosition = 0U;       // Adresa vo¾nej pozície na zápis

static uint32_t find_free_position(void) {
    uint32_t addr = QSPI_START_ADDRESS;
    uint32_t pattern;

    counter = 0u;
    // Preh¾adanie sektorov na pattern od 0x12345670 a 0x1234567F
    for (uint8_t x = 0x00; x <= 0x0F; x++) {
        uint32_t expectedPattern = 0x70563412U | (x << 24);  // Vytvorte poadovanı pattern s X od 0 do F

        addr = QSPI_START_ADDRESS; // Reset adresy na zaèiatok QSPI pre kadı novı pattern

        // Preh¾adáme sektory v pamäti
        while (addr < N25Q128A_FLASH_SIZE) {
            // Èítame prvé 4 bajty (pattern) na zaèiatku sektora
            BSP_QSPI_Read((uint8_t*)&pattern, addr, sizeof(pattern));

            // Overenie, èi pattern zodpovedá poadovanému patternu
            if (pattern == expectedPattern) {
                // Našli sme platnı sektor, zaèíname h¾ada vo¾nú pozíciu
                uint32_t offset = addr + sizeof(pattern);
                uint8_t buffer[BLOCK_SIZE];

                while (offset < (addr + SECTOR_SIZE)) {
                    // Èítanie bloku (256 bajtov naraz)
                    BSP_QSPI_Read(buffer, offset, BLOCK_SIZE);

                    // Preh¾adanie bloku po jednotlivıch záznamoch (kadı má ve¾kos RECORD_SIZE)
                    for (uint32_t i = 0; i < BLOCK_SIZE; i += RECORD_SIZE) {
                        uint32_t *record = (uint32_t*)&buffer[i];
                        if (*record == 0xFFFFFFFFU) {
                            return offset + i; // Našli sme vo¾nú pozíciu
                        }
                        counter++; // Zvyšujeme poèítadlo pri nájdení platného záznamu
                    }
                    // Posunieme sa na ïalší blok
                    offset += BLOCK_SIZE;
                }
            }

            // Posunieme sa na ïalší sektor
            addr += SECTOR_SIZE;
        }
    }
    
    return QSPI_MEMORY_FULL; // Ak sme nenašli vo¾né miesto
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

bool flash_temperatureSave(float temperature) {
    uint8_t data[RECORD_SIZE];
    int16_t temp = (int16_t)(temperature * 16); // Konverzia teploty

    // Skontrolujeme, èi je vo¾ná pozícia platná
    if (freePosition >= N25Q128A_FLASH_SIZE || freePosition == QSPI_MEMORY_FULL) {
        return false; // Pamä je plná
    }
    counter++;  // Inkrementácia counteru pri kadom zápise teploty
    
    // Konverzia teploty na 4-bajtové dáta
    data[0] = (uint8_t)(temp & 0xFF);        // LSB
    data[1] = (uint8_t)((temp >> 8) & 0xFF); // MSB
    data[2] = (counter & 0xFF00u) >> 8u;  // Doplnujúci bajt pre èas alebo inú hodnotu
    data[3] = counter & 0xFFu;            // Doplnujúci bajt pre èas alebo inú hodnotu

    // Zápis do FLASH
    BSP_QSPI_Write(data, freePosition, sizeof(data));

    // Aktualizácia vo¾nej pozície
    freePosition += RECORD_SIZE;

    // Ak prekroèíme hranicu sektora, h¾adáme ïalší
    if (freePosition % SECTOR_SIZE == 0) {
        freePosition = find_free_position();
    }

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
