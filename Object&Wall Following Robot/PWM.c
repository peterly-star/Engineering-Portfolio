// PWM.c
/////////////////////////////////////////////////////////////////////////////
// Course Number: CECS 347
// Assignment: Example project for Hardware PWM controlled Car
// Description: 
/////////////////////////////////////////////////////////////////////////////

#include <stdint.h>
#include "tm4c123gh6pm.h"
#include "PWM.h"

// configure the system to get its clock from the PLL
// configure the system to get its clock from the PLL
// void PLL_Init(void){
//   // 0) configure the system to use RCC2 for advanced features
//   //    such as 400 MHz PLL and non-integer System Clock Divisor
//   SYSCTL_RCC2_R |= SYSCTL_RCC2_USERCC2;
//   // 1) bypass PLL while initializing
//   SYSCTL_RCC2_R |= SYSCTL_RCC2_BYPASS2;
//   // 2) select the crystal value and oscillator source
//   SYSCTL_RCC_R &= ~SYSCTL_RCC_XTAL_M;   // clear XTAL field
//   SYSCTL_RCC_R += SYSCTL_RCC_XTAL_16MHZ;// configure for 16 MHz crystal
//   SYSCTL_RCC2_R &= ~SYSCTL_RCC2_OSCSRC2_M;// clear oscillator source field
//   SYSCTL_RCC2_R += SYSCTL_RCC2_OSCSRC2_MO;// configure for main oscillator source
//   // 3) activate PLL by clearing PWRDN
//   SYSCTL_RCC2_R &= ~SYSCTL_RCC2_PWRDN2;
//   // 4) set the desired system divider and the system divider least significant bit
//   SYSCTL_RCC2_R |= SYSCTL_RCC2_DIV400;  // use 400 MHz PLL
//   SYSCTL_RCC2_R = (SYSCTL_RCC2_R&~0x1FC00000) + (7<<22);
//   // 5) wait for the PLL to lock by polling PLLLRIS
//   while((SYSCTL_RIS_R&SYSCTL_RIS_PLLLRIS)==0){};
//   // 6) enable use of PLL by clearing BYPASS
//   SYSCTL_RCC2_R &= ~SYSCTL_RCC2_BYPASS2;
// }
// Shared globals from FollowingRobot.c
extern robot_modes mode;
extern volatile uint8_t wall_side;

// Initializes the PWM module 0 generator 0 outputs A&B tied to PB54 to be used with the 
//		L298N motor driver allowing for a variable speed of robot car.
void PWM_PB54_Init(void){
	if ((SYSCTL_RCGC2_R&SYSCTL_RCGC2_GPIOB)==0) {
		SYSCTL_RCGC2_R |= SYSCTL_RCGC2_GPIOB;	// Activate B clocks
		while ((SYSCTL_RCGC2_R&SYSCTL_RCGC2_GPIOB)==0){};
	}
	
	GPIO_PORTB_AFSEL_R |= 0x30;	// enable alt funct: PB54 for PWM
  GPIO_PORTB_PCTL_R &= ~0x00FF0000; // PWM to be used
  GPIO_PORTB_PCTL_R |= 0x00440000; // PWM to be used
  GPIO_PORTB_DEN_R |= 0x30;	// enable digital I/O 
	
	// Initializes PWM settings
	SYSCTL_RCGCPWM_R |= 0x01;	// activate PWM0
	SYSCTL_RCC_R &= ~0x001E0000; // Clear any previous PWM divider values
	
	// PWM0_1 output A&B Initialization for PB54
	PWM0_1_CTL_R = 0;	// re-loading down-counting mode
	PWM0_1_GENA_R |= 0xC8;	// low on LOAD, high on CMPA down
	PWM0_1_GENB_R |= 0xC08;  // low on LOAD, high on CMPB down
	PWM0_1_LOAD_R = TOTAL_PERIOD - 1;	// cycles needed to count down to 0
  PWM0_1_CMPA_R = 0;	// count value when output rises
	PWM0_1_CMPB_R = 0;	// count value when output rises
	
	PWM0_1_CTL_R |= 0x00000001;	// Enable PWM0 Generator 0 in Countdown mode
	PWM0_ENABLE_R |= 0x0000000C;	// Disable PB54:PWM0 output 0&1 on initialization
}

//	duty_L is the value corresponding to the duty cycle of the left wheel
//	duty_R is the value corresponding to the duty cycle of the right wheel
// Changes the duty cycles of PB54 by changing the CMP registers
void PWM_PB54_Duty(unsigned long duty_L, unsigned long duty_R){
	PWM0_1_CMPA_R = duty_L - 1;	// PB4 count value when output rises
  PWM0_1_CMPB_R = duty_R - 1;	// PB5 count value when output rises
}

// Initilize port F and arm PF4, PF0 for falling edge interrupts
void Switch_Init(void){  
	unsigned long volatile delay;
  SYSCTL_RCGC2_R |= SYSCTL_RCGC2_GPIOF; // (a) activate clock for port F
  delay = SYSCTL_RCGC2_R;
  GPIO_PORTF_LOCK_R = GPIO_LOCK_KEY; // unlock GPIO Port F
  GPIO_PORTF_CR_R |= 0x11;           // allow changes to PF4,0
  GPIO_PORTF_DIR_R &= ~0x11;    // (c) make PF4,0 in (built-in button)
  GPIO_PORTF_AFSEL_R &= ~0x11;  //     disable alt funct on PF4,0
  GPIO_PORTF_DEN_R |= 0x11;     //     enable digital I/O on PF4,0
  GPIO_PORTF_PCTL_R &= ~0x000F000F;  //  configure PF4,0 as GPIO
  GPIO_PORTF_AMSEL_R &= ~0x11;  //     disable analog functionality on PF4,0
  GPIO_PORTF_PUR_R |= 0x11;     //     enable weak pull-up on PF4,0
  GPIO_PORTF_IS_R &= ~0x11;     // (d) PF4,PF0 is edge-sensitive
  GPIO_PORTF_IBE_R &= ~0x11;    //     PF4,PF0 is not both edges
  GPIO_PORTF_IEV_R &= ~0x11;    //     PF4,PF0 falling edge event
  GPIO_PORTF_ICR_R = 0x11;      // (e) clear flags 4,0
  GPIO_PORTF_IM_R |= 0x11;      // (f) arm interrupt on PF4,PF0
  NVIC_PRI7_R = (NVIC_PRI7_R&0xFF1FFFFF)|0x00400000; // (g) bits:23-21 for PORTF, set priority to 5
  NVIC_EN0_R |= (1 << 30);      // (h) enable interrupt 30 in NVIC
}


// PORTF ISR:
// Change delivered power based on switch press: 
// sw1: increase 10% until reach 90%
// sw2: changes direction
void GPIOPortF_Handler(void){ // called on touch of either SW1 or SW2
    int TENMS = 727240 * 20 / 91; // ~20ms debounce delay
    while (TENMS) TENMS--;        // debounce delay

    // ===== Handle SW1 (PF4) - Object Follower =====
    if (GPIO_PORTF_RIS_R & 0x10) {  // SW1 pressed
        GPIO_PORTF_ICR_R = 0x10;    // acknowledge flag4

        if (mode != OBJECT_FOLLOWER) {
            mode = OBJECT_FOLLOWER;
            SW1_Pressed_Flag = 1;
            LED = Blue;              // indicate object follower
        } else {
            mode = INACTIVE;
            SW1_Pressed_Flag = 0;
            LED = Yellow;            // return to idle
            StopMotors();
        }
    }

    // ===== Handle SW2 (PF0) - Wall Follower =====
    static uint8_t wall_press_count = 0;  // track how many times SW2 is pressed

    if (GPIO_PORTF_RIS_R & 0x01) {        // SW2 pressed
        GPIO_PORTF_ICR_R = 0x01;          // acknowledge flag0

        wall_press_count++;

        if (wall_press_count == 1) {
            // First press follow RIGHT wall
            mode = WALL_FOLLOWER;
            wall_side = 2;                // right wall
            LED = Green;
        }
        else if (wall_press_count == 2) {
            // Second press follow LEFT wall
            mode = WALL_FOLLOWER;
            wall_side = 1;                // left wall
            LED = Blue;
        }
        else {
            // Third press ? go idle
            wall_press_count = 0;
            mode = INACTIVE;
            LED = Yellow;
            StopMotors();
        }
    }
}

// Function to move forward
void MoveForward(void) {
  WHEEL_DIR = FORWARD;  						 // Set direction to forward
	PWM0_ENABLE_R |= 0x00000003; 			 // enable both wheels
	PWM_PB54_Duty(SPEED_15, SPEED_15); 
}
void MoveForward_2(void) {
  WHEEL_DIR = FORWARD;  						 // Set direction to forward
	PWM0_ENABLE_R |= 0x00000003; 			 // enable both wheels
	PWM_PB54_Duty(SPEED_25, SPEED_25);
}

// Function for left turn while moving forward
void LeftTurnForward(void) {
  WHEEL_DIR = FORWARD;  						 // Set direction to forward
	PWM0_ENABLE_R |= 0x00000002;			 // Enable right wheel
	PWM0_ENABLE_R &= ~0x00000001; 		 // Disable left wheel
  PWM_PB54_Duty(SPEED_12, SPEED_5); 
}

// Function for right turn while moving forward
void RightTurnForward(void) {
  WHEEL_DIR = FORWARD;  						 // Set direction to forward
	PWM0_ENABLE_R &= ~0x00000002; 		 // Disable right wheel
	PWM0_ENABLE_R |= 0x00000001; 			 // Enable left wheel
  PWM_PB54_Duty(SPEED_5, SPEED_12);
}


// Function to move backward
void MoveBackward(void) {
  WHEEL_DIR = BACKWARD;  						 // Set direction to backward
	PWM0_ENABLE_R |= 0x00000003; 			 // enable both wheels
	PWM_PB54_Duty(SPEED_10, SPEED_10); 
}

// Function for left turn while moving forward
void LeftTurnForward_R(void) {
  WHEEL_DIR = FORWARD;  						 // Set direction to forward
	PWM0_ENABLE_R |= 0x00000002;			 // Enable right wheel
	PWM0_ENABLE_R &= ~0x00000001; 		 // Disable left wheel
  PWM_PB54_Duty(SPEED_50, SPEED_2); 
}

// Function for right turn while moving forward
void RightTurnForward_R(void) {
  WHEEL_DIR = FORWARD;  						 // Set direction to forward
	PWM0_ENABLE_R &= ~0x00000002; 		 // Disable right wheel
	PWM0_ENABLE_R |= 0x00000001; 			 // Enable left wheel
  PWM_PB54_Duty(SPEED_10, SPEED_25); 
}

void LeftTurnForward_L(void) {
  WHEEL_DIR = FORWARD;  						 // Set direction to forward
	PWM0_ENABLE_R |= 0x00000002;			 // Enable right wheel
	PWM0_ENABLE_R &= ~0x00000001; 		 // Disable left wheel
  PWM_PB54_Duty(SPEED_25, SPEED_8); 
}

// Function for right turn while moving forward
void RightTurnForward_L(void) {
  WHEEL_DIR = FORWARD;  						 // Set direction to forward
	PWM0_ENABLE_R &= ~0x00000002; 		 // Disable right wheel
	PWM0_ENABLE_R |= 0x00000001; 			 // Enable left wheel
  PWM_PB54_Duty(SPEED_2, SPEED_40); 
}
// Function for right turn while moving backward
void RightTurnBackward(void) {
  WHEEL_DIR = BACKWARD; 						 // Set direction to backward
	PWM0_ENABLE_R &= ~0x00000002; 		 // Disable right wheel
	PWM0_ENABLE_R |= 0x00000001; 			 // Enable left wheel
  PWM_PB54_Duty(SPEED_15, SPEED_8);
}

// Function for left turn while moving backward
void LeftTurnBackward(void) {
  WHEEL_DIR = BACKWARD; 						 // Set direction to backward
	PWM0_ENABLE_R |= 0x00000002; 			 // Enable right wheel
	PWM0_ENABLE_R &= ~0x00000001; 		 // Disable left wheel
  PWM_PB54_Duty(SPEED_8, SPEED_15); 
}

// Function to pivot right
void PivotRight(void) {
  WHEEL_DIR = RIGHTPIVOT;  					 // Set direction to left pivot (0xC8)
	PWM0_ENABLE_R |= 0x00000003; 			 // Enable both wheels
  PWM_PB54_Duty(SPEED_15, SPEED_15); 
}

// Function to pivot left
void PivotLeft(void) {
  WHEEL_DIR = LEFTPIVOT;  					 // Set direction to right pivot (0x8C)
	PWM0_ENABLE_R |= 0x00000003; 			 // Enable both wheels
  PWM_PB54_Duty(SPEED_15, SPEED_15); 
}

// Function to stop the motors
void StopMotors(void) {
  WHEEL_DIR = STOP;     				// Stop both motors
	PWM0_ENABLE_R &= ~0x00000003; // stop both wheels
  PWM_PB54_Duty(STOP, STOP); 		// Stop both motors
}
