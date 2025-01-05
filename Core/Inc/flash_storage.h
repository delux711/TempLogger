#ifndef __FLASH_STORAGE_H
#define __FLASH_STORAGE_H

#include <stdbool.h>
#include "stm32l4xx_hal.h"

// Funkcie na z�pis a c�tanie teploty
void flash_temperatureInit(void);
void flash_updateCounter(void);
bool flash_temperatureSave(float temperature); // Z�pis teploty do FLASH
float flash_temperatureRead(uint32_t address); // C�tanie teploty z FLASH
uint16_t flash_getCounter(void);

#endif /* __FLASH_STORAGE_H */
