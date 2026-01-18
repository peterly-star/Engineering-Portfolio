// Motor.c
// Line Following Robot - Motor Control Module
// Original Author: Hugo Razo, Peter Ly, Mark Mendez, Hector Perez
// Reformatted and Commented: 5/3/2025

#include "tm4c123gh6pm.h"
#include <stdint.h>
#include "Motors.h"
#include "Systick.h"

//  Motor Pin Definitions 
// PB7 - PWM for Right Motor
// PB6 - PWM for Left Motor
// PB5, PB4 - Direction control for Left Motor
// PB3, PB2 - Direction control for Right Motor
#define MOTOR_CONTROL_PINS  0xFC  // PB7–PB2



//  Motor Initialization 
// Configures PB7–PB2 for digital output use in motor control.
// This includes setting direction and PWM lines.
void Motor_Init(void) {
    SYSCTL_RCGCGPIO_R |= 0x02;               // Enable clock for Port B
    while ((SYSCTL_RCGCGPIO_R & 0x02) == 0); // Wait until Port B is ready

    GPIO_PORTB_AMSEL_R  &= ~MOTOR_CONTROL_PINS;   // Disable analog on PB7–PB2
    GPIO_PORTB_PCTL_R   &= ~0xFFFFFF00;           // Clear PCTL for PB7–PB2 (set to GPIO)
    GPIO_PORTB_DIR_R    |= MOTOR_CONTROL_PINS;    // Set PB7–PB2 as outputs
    GPIO_PORTB_AFSEL_R  &= ~MOTOR_CONTROL_PINS;   // Disable alternate functions
    GPIO_PORTB_DEN_R    |= MOTOR_CONTROL_PINS;    // Enable digital I/O
}

