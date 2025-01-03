#ifndef TEMPERATURE_H
#define TEMPERATURE_H

#include "stm32l4xx_hal.h"

// Inicializ�cia DS18B20
void Temperature_Init(void);

// C�tanie teploty (v�sledok v �C)
float Temperature_Read(void);

#endif // TEMPERATURE_H
