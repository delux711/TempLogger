#ifndef TEMPERATURE_H
#define TEMPERATURE_H

#include "stm32l4xx_hal.h"

// Inicializácia DS18B20
void Temperature_Init(void);

// Cítanie teploty (výsledok v °C)
float Temperature_Read(void);

#endif // TEMPERATURE_H
