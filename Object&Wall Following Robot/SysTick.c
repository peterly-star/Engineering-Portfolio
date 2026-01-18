#include <stdint.h>
#include "tm4c123gh6pm.h"

void SysTick_Init(void) {
    NVIC_ST_CTRL_R = 0;           // Disable SysTick during setup
    NVIC_ST_RELOAD_R = 0x00FFFFFF; // Max reload value
    NVIC_ST_CURRENT_R = 0;        // Clear current value
    NVIC_ST_CTRL_R = 0x05;        // Enable SysTick with system clock
}

// Busy-wait delay using SysTick
void SysTick_Wait(uint32_t delay) {
    NVIC_ST_RELOAD_R = delay - 1;  // Number of counts
    NVIC_ST_CURRENT_R = 0;         // Clear current value
    while ((NVIC_ST_CTRL_R & 0x00010000) == 0) {
        // Wait for COUNT flag to be set
    }
}