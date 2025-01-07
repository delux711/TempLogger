#include "flash_storage.h"
#include "stm32l476g_discovery_qspi.h"  // Tento s�bor poskytuje funkcie pre pr�cu s QSPI

#define QSPI_START_ADDRESS  0x00000000U  // Za�iatok QSPI pam�te
#define RECORD_SIZE         4           // Ve�kos� jedn�ho z�znamu (napr. 4 bajty)
#define BLOCK_SIZE          256         // Velkost bloku na c�tanie (napr. 256 bajtov)
#define SECTOR_SIZE         0x1000      // Ve�kos� sektora (4 kB)
#define FULL_FILE_SIZE      0x10000     // Ve�kos� "s�boru" (64 kB)
#define QSPI_MEMORY_FULL 0xFFFFFFFFU    // Indikuje pln� pam�t
#define PATTERN_BASE        0x12345670U // Z�kladn� identifik�tor pre sektory
#define PATTERN_END         0x1234567FU // Posledn� platn� identifik�tor

static uint16_t counter = 0u;
static uint32_t freePosition = 0U;       // Adresa vo�nej poz�cie na z�pis

static uint32_t find_free_position(void) {
    uint32_t addr = QSPI_START_ADDRESS;
    uint32_t pattern;

    counter = 0u;
    // Preh�adanie sektorov na pattern od 0x12345670 a� 0x1234567F
    for (uint8_t x = 0x00; x <= 0x0F; x++) {
        uint32_t expectedPattern = 0x70563412U | (x << 24);  // Vytvorte po�adovan� pattern s X od 0 do F

        addr = QSPI_START_ADDRESS; // Reset adresy na za�iatok QSPI pre ka�d� nov� pattern

        // Preh�ad�me sektory v pam�ti
        while (addr < N25Q128A_FLASH_SIZE) {
            // ��tame prv� 4 bajty (pattern) na za�iatku sektora
            BSP_QSPI_Read((uint8_t*)&pattern, addr, sizeof(pattern));

            // Overenie, �i pattern zodpoved� po�adovan�mu patternu
            if (pattern == expectedPattern) {
                // Na�li sme platn� sektor, za��name h�ada� vo�n� poz�ciu
                uint32_t offset = addr + sizeof(pattern);
                uint8_t buffer[BLOCK_SIZE];

                while (offset < (addr + SECTOR_SIZE)) {
                    // ��tanie bloku (256 bajtov naraz)
                    BSP_QSPI_Read(buffer, offset, BLOCK_SIZE);

                    // Preh�adanie bloku po jednotliv�ch z�znamoch (ka�d� m� ve�kos� RECORD_SIZE)
                    for (uint32_t i = 0; i < BLOCK_SIZE; i += RECORD_SIZE) {
                        uint32_t *record = (uint32_t*)&buffer[i];
                        if (*record == 0xFFFFFFFFU) {
                            return offset + i; // Na�li sme vo�n� poz�ciu
                        }
                        counter++; // Zvy�ujeme po��tadlo pri n�jden� platn�ho z�znamu
                    }
                    // Posunieme sa na �al�� blok
                    offset += BLOCK_SIZE;
                }
            }

            // Posunieme sa na �al�� sektor
            addr += SECTOR_SIZE;
        }
    }
    
    return QSPI_MEMORY_FULL; // Ak sme nena�li vo�n� miesto
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

    // Skontrolujeme, �i je vo�n� poz�cia platn�
    if (freePosition >= N25Q128A_FLASH_SIZE || freePosition == QSPI_MEMORY_FULL) {
        return false; // Pam� je pln�
    }
    counter++;  // Inkrement�cia counteru pri ka�dom z�pise teploty
    
    // Konverzia teploty na 4-bajtov� d�ta
    data[0] = (uint8_t)(temp & 0xFF);        // LSB
    data[1] = (uint8_t)((temp >> 8) & 0xFF); // MSB
    data[2] = (counter & 0xFF00u) >> 8u;  // Doplnuj�ci bajt pre �as alebo in� hodnotu
    data[3] = counter & 0xFFu;            // Doplnuj�ci bajt pre �as alebo in� hodnotu

    // Z�pis do FLASH
    BSP_QSPI_Write(data, freePosition, sizeof(data));

    // Aktualiz�cia vo�nej poz�cie
    freePosition += RECORD_SIZE;

    // Ak prekro��me hranicu sektora, h�ad�me �al��
    if (freePosition % SECTOR_SIZE == 0) {
        freePosition = find_free_position();
    }

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
