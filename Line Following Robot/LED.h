// LED.h
// Test code for reflectance sensors. 
// Created by TYLOR FRANCA
// Modified by Hugo Razo, Peter Ly, Mark Mendez, Hector Perez
// 4/22/2025
// California State University, Long Beach

#ifndef LED_H
#define LED_H
#include <stdint.h>

#define LED       (*((volatile uint32_t *)0x40025038))
	
// Color    LED(s) PortF
// dark     ---    0
// red      R--    0x02
// blue     --B    0x04
// green    -G-    0x08
// yellow   RG-    0x0A
// sky blue -GB    0x0C
// white    RGB    0x0E
// pink     R-B    0x06
#define GREEN 	0x08
#define RED 		0x02
#define BLUE 		0x04
#define WHITE   0x0E
#define DARK    0x00

void LED_Init(void);

#endif