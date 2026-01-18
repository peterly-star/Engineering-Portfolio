// Systick.c
// Runs on TM4C123
// CECS 347 Project 3 - Space Invaders
// Group number: 2
// Group members: Peter Ly Hugo Razo Mark Mendez Hector Perez

#include <stdint.h>
#include "tm4c123gh6pm.h"

void SysTick_Init(void) {
    NVIC_ST_CTRL_R = 0;            // Disable SysTick during setup
    NVIC_ST_RELOAD_R = 0x00FFFFFF; // Max reload value
    NVIC_ST_CURRENT_R = 0;         // Clear current value
    // DO NOT enable here; System_Init will configure CTRL
    // NVIC_ST_CTRL_R = 0x05;
}

void SysTick_Wait(uint32_t delay) {
    NVIC_ST_RELOAD_R = delay - 1;  // Load delay
    NVIC_ST_CURRENT_R = 0;         // Clear current value
    while ((NVIC_ST_CTRL_R & 0x00010000) == 0) {
        // Wait for COUNT flag
    }
}

// Wait for 1 millisecond each time
void SysTick_Wait1ms(uint32_t n) {
    for (uint32_t i = 0; i < n; i++) {
        SysTick_Wait(80000);       // 1 ms at 80 MHz
    }
}

// Wait for n×100 ms intervals
void SysTick_Wait100ms(uint32_t n) {
    for (uint32_t i = 0; i < n; i++) {
        SysTick_Wait(8000000);     // 0.1 s at 80 MHz
    }
}
