#ifndef __FLASH_STORAGE_H
#define __FLASH_STORAGE_H

#include "stm32l4xx_hal.h"

// Funkcie na zápis a cítanie teploty
void flash_temperatureInit(void);
void flash_temperatureSave(float temperature); // Zápis teploty do FLASH
float flash_temperatureRead(uint32_t address); // Cítanie teploty z FLASH

#endif /* __FLASH_STORAGE_H */
