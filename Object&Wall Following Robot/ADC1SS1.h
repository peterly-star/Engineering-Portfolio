#ifndef _ADC1SS1_H_
#define _ADC1SS1_H_

#include <stdint.h>

// Initialize ADC1 Sample Sequencer 1 for three channels (e.g. PE2, PE1, PE0)
void ADC1_SS1_Init(void);

// Read three channels: left, front, right (returns 12-bit ADC values)
void ADC1_InSS1(unsigned long *left, unsigned long *front, unsigned long *right);

// Optional: your wrapper for filtered reads
void ADC1_ReadAllSensorsFiltered(uint16_t *left, uint16_t *front, uint16_t *right);

#endif
