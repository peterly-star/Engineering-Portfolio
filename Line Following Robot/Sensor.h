// Sensor.h
// Line Following Robot Starter program 
// 4/25/2025

#include <stdint.h>

// PE0 connect to sensor 7; PE1 connects to Sensor 0
// TODO: find the bit addresses
#define SENSORS           (*((volatile uint32_t *)0))

void Sensor_Init(void);
uint8_t Sensor_CollectData(void);
