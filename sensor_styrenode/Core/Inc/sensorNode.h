#ifndef SENSOR_NODE_H
#define SENSOR_NODE_H

#include "main.h"

// Modul-API for Sensor-Node
void SensorNode_Init(void);
void SensorNode_Loop(void);

uint16_t konverter_mV(uint16_t adc_val);
uint16_t konverter_mm(uint16_t adc_mV);


#endif // SENSOR_NODE_H

