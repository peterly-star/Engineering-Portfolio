// Systick.c
// Line Following Robot Starter Program
// Original Author: Hugo Razo, Peter Ly, Mark Mendez, Hector Perez
// Reformatted, Commented, and Reordered: 5/3/2025

#include "tm4c123gh6pm.h"
#include "Systick.h"
#include <stdint.h>

// SysTick Timer Constants
#define ONE_MILLI_S 16000   // 1 ms delay (16 MHz system clock)
#define ONE_MICRO_S 16      // 1 us delay (16 MHz system clock)


// Configures SysTick timer to use core clock.
void SysTick_Init(void) {
    NVIC_ST_CTRL_R = 0;                      // Disable SysTick during setup
    NVIC_ST_CTRL_R |= NVIC_ST_CTRL_CLK_SRC; // Use system clock (assumes 16 MHz)
}


// Generates a 1 millisecond delay using SysTick.
void DelayMs(void) {
    NVIC_ST_RELOAD_R = ONE_MILLI_S - 1;     // Load value for 1ms delay
    NVIC_ST_CURRENT_R = 0;                  // Clear current value
    NVIC_ST_CTRL_R |= NVIC_ST_CTRL_ENABLE;  // Enable SysTick

    while ((NVIC_ST_CTRL_R & NVIC_ST_CTRL_COUNT) == 0); // Wait until COUNT flag is set

    NVIC_ST_CTRL_R &= ~NVIC_ST_CTRL_ENABLE; // Disable SysTick
}

// Generates a 1 microsecond delay using SysTick.
void DelayUs(void) {
    NVIC_ST_RELOAD_R = ONE_MICRO_S - 1;     // Load value for 1us delay
    NVIC_ST_CURRENT_R = 0;                  // Clear current value
    NVIC_ST_CTRL_R |= NVIC_ST_CTRL_ENABLE;  // Enable SysTick

    while ((NVIC_ST_CTRL_R & NVIC_ST_CTRL_COUNT) == 0); // Wait until COUNT flag is set

    NVIC_ST_CTRL_R &= ~NVIC_ST_CTRL_ENABLE; // Disable SysTick
}


// Generates delay in multiples of 1ms
// Input: delay - number of milliseconds to wait
void Wait_N_MS(uint32_t delay) {
    for (uint32_t i = 0; i < delay; i++) {
        DelayMs();
    }
}


// Generates delay in multiples of 1us
// Input: delay - number of microseconds to wait
void Wait_N_US(uint32_t delay) {
    for (uint32_t i = 0; i < delay; i++) {
        DelayUs();
    }
}