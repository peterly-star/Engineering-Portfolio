// Systick.h
// Runs on TM4C123
// CECS 347 Project 3 - Space Invaders
// Group number: 2 
// Group members: Peter Ly Hugo Razo Mark Mendez Hector Perez

#ifndef SYSTICK_H
#define SYSTICK_H
#include <stdint.h>

void SysTick_Init(void);
void SysTick_Wait(uint32_t delay);
void SysTick_Wait1ms(uint32_t n);
void SysTick_Wait100ms(uint32_t n);

#endif