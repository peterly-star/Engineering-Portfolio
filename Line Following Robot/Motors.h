#ifndef MOTORS_H
#define MOTORS_H

#include "tm4c123gh6pm.h"
#include <stdint.h>

// Bit masks
#define MOTOR_PWM_MASK   0xC0  // PB6 and PB7
#define MOTOR_DIR_MASK   0x3C  // PB5 to PB2

// Motion definitions
#define FORWARD      0xFC  // DIR=0x3C, PWM=0xC0 (both motors forward)
#define TURN_LEFT    0xF4  // DIR=0x3C, PWM=0x80 (right motor only)
#define TURN_RIGHT   0xF8  // DIR=0x3C, PWM=0x40 (left motor only)
#define STOP         0x3C  // DIR=0x3C, PWM=0x00 (no PWM)

#define MOTORS       (*((volatile uint32_t *)0x400053FC))  // Full Port B access

void Motor_Init(void);
void Motor_Output(uint8_t data);

#endif