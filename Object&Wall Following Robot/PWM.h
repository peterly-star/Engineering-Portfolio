// PWM.h
// Runs on TM4C123
// Use PWM0AA/PB4 and PWM0B/PB4 to generate pulse-width modulated outputs.

#include <stdint.h>
// LED Definitions
#define LED       (*((volatile unsigned long *)0x40025038))  // use onboard three LEDs: PF321
#define Dark      0x00
#define Red       0x02
#define Blue      0x04
#define Green     0x08
#define Yellow    0x0A
#define Cran      0x0C
#define White     0x0E
#define Purple    0x06
#define Cyan      0x0C   // Green + Blue (same as Cran)
#define Orange    0x0A   // Red + Green (same as Yellow)
#define Pink      0x06   // Red + Blue (same as Purple)


// PWM Definitions
#define TOTAL_PERIOD 16000 	// 16MHz/1000=16000
#define START_SPEED 16000*0.5

// Constant definitions based on the following hardware interface:
// PB7632 are used for direction control on L298.
// Motor 1 is connected to the left wheel, Motor 2 is connected to the right wheel.
#define WHEEL_DIR (*((volatile unsigned long *)0x40005330)) // PB7632 for L298N
#define FORWARD      0xCC  // 1111 -> 1100 1100 PB32,76 (1)
#define BACKWARD     0x88  // 0101 -> 1000 1000 PB7(0), PB6(1), PB3(0), PB2(1)
#define RIGHTPIVOT   0xC8  // 0110 -> 1100 1000 PB7(0), PB6(1), PB3(1), PB2(0)
#define LEFTPIVOT    0x8C  // 1001 -> 1000 1100 PB7(1), PB6(0), PB3(0), PB2(1)
#define PERIOD 10000				// Total PWM period
#define STEP 1000						// increments of duty cycle
#define MAX_DUTY 15999			// max duty cycle (90%)
#define STOP 1							// min duty cycle (0%)
#define SPEED_50 8000    // 50% duty cycle
#define SPEED_30 4800    // 30% duty cycle (0.30 * 16000)
#define SPEED_35 5600    // 35% duty cycle (0.35 * 16000)
#define SPEED_40 6400    // 40% duty cycle (0.40 * 16000)
#define SPEED_25 4000    // 25% duty cycle (0.25 * 16000)
#define SPEED_20 3200    // 20% duty cycle (0.20 * 16000)
#define SPEED_15 2400    // 15% duty cycle (0.15 * 16000)
#define SPEED_12 1920    // 12% duty cycle (0.12 * 16000)
#define SPEED_10 1600    // 10% duty cycle (0.10 * 16000)
#define SPEED_9  1440    // 9% duty cycle  (0.09 * 16000)
#define SPEED_8  1280    // 8% duty cycle
#define SPEED_6   960    // 6% duty cycle
#define SPEED_5   800    // 5% duty cycle (800/16000 = 0.05)
#define SPEED_2 320     // 2% duty cycle (0.02 * 16000)


extern volatile int SW1_Pressed_Flag;

#ifndef MODES_H
#define MODES_H

// Define the modes
typedef enum {
    INACTIVE,
    OBJECT_FOLLOWER,
    WALL_FOLLOWER
} robot_modes;

// Declare the global mode variable
extern robot_modes mode;

#endif

void PLL_Init(void);

// Duty and Initialization of PWM
void PWM_PB54_Init();
void PWM_PB54_Duty(unsigned long duty_L, unsigned long duty_R);

// Switches Initialization
void Switch_Init(void);

// Power/Direction Initialization
void GPIOPortF_Handler(void);

// controls direction of robot car
void MoveForward(void);
void MoveBackward(void);
void LeftTurnForward(void);
void RightTurnForward(void);
void LeftTurnBackward(void);
void RightTurnBackward(void);
void PivotLeft(void);
void PivotRight(void);
void StopMotors(void);
void MoveForward_2(void);
void RightTurnForward_R(void);
void LeftTurnForward_R(void);
void RightTurnForward_L(void);
void LeftTurnForward_L(void);