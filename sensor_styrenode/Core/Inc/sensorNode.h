#ifndef SENSOR_NODE_H
#define SENSOR_NODE_H

#include "main.h"

int Is_sensorNode(void);

// Modul-API for Sensor-Node
void SensorNode_Init(void);
void SensorNode_Loop(void);

// Les siste målinger (oppdatert i ADC3_EndOfConversion_Callback)
uint16_t SensorNode_GetLastAdc(void);
uint32_t SensorNode_GetLastTid(void);

int konverter_mV(uint16_t adc_val);
int konverter_mm(uint16_t adc_mV);


#endif // SENSOR_NODE_H

