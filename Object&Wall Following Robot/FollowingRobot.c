// FollowingRobot.c
// Runs TM4C123
// Starter program CECS 347 project 2 - A Following Robot
// by Peter Ly, Hector Perez, Hugo Razo, Mark Mendez 

#include "tm4c123gh6pm.h"
#include <stdint.h>
#include "ADC1SS1.h"
#include "PWM.h"
#include "PLL.h"
#include "GPIO.h"
#include "SysTick.h"

// ======= Distance thresholds (ADC counts) =======
// Front thresholds
#define FRONT_20_ADC       1660
#define FRONT_20_BAND      90    
#define FRONT_10_ADC       2495     
#define FRONT_10_BAND      90
#define FRONT_70_ADC       615    
#define FRONT_70_BAND      50

// Left thresholds 
#define LEFT_20_ADC        1800    
#define LEFT_20_BAND       100
#define LEFT_10_ADC        2600   
#define LEFT_10_BAND       60 
#define LEFT_70_ADC        620      
#define LEFT_70_BAND       40

// Right thresholds
#define RIGHT_20_ADC       1550     
#define RIGHT_20_BAND      60
#define RIGHT_10_ADC       2425
#define RIGHT_10_BAND      60
#define RIGHT_70_ADC       680   
#define RIGHT_70_BAND      40

// external functions
extern void DisableInterrupts(void);
extern void EnableInterrupts(void);
extern void WaitForInterrupt(void);

// functions defined in this file
void System_Init(void);
void object_follower(void);
void wall_follower(void);

// Globals required by PWM.c
volatile int SW1_Pressed_Flag = 0;
robot_modes mode = INACTIVE;

// Debug globals
volatile uint16_t g_left = 0;
volatile uint16_t g_front = 0;
volatile uint16_t g_right = 0;
volatile uint8_t wall_side = 2;   // 1 = left wall, 2 = right wall (default right)


// ===== Small 10ms delay helper =====
static void Wait10ms(uint32_t n){
  for(uint32_t i=0;i<n;i++){
    SysTick_Wait(160000);    // 10ms at 16 MHz
  }
}

// ===== Main =====
int main(void){
  System_Init();
  LED = Red;
  WaitForInterrupt();     // wait for switch press

  while(1){
    switch(mode){
      case OBJECT_FOLLOWER:
        object_follower();
			LED= Blue;
        break;

      case WALL_FOLLOWER:
        wall_follower();
			LED=Green;
        break;

      default:
        break;
    }
  }
}

// Object Follower Mode
void object_follower(void){
  uint16_t left, right, front;
  uint8_t i;

  // LED indicator   Blue for active
  LED = Blue;

  // Prime ADC readings
  for (i = 0; i < 10; i++){
    ADC1_ReadAllSensorsFiltered(&left, &front, &right);
    SysTick_Wait(80000);  // 5 ms delay
  }

  uint16_t prev_left  = left;
  uint16_t prev_right = right;
  uint16_t prev_front = front;
  uint8_t stable_mode = 0;  // 0 = idle, 1 = forward, 2 = too close

  // Persistent variables for hysteresis
  static uint8_t turn_state = 0;   // 0 = none, 1 = turning left, 2 = turning right
  static uint8_t turn_counter = 0;

  while (mode == OBJECT_FOLLOWER){
    // Average 5 samples to smooth noise 
    uint32_t sumL = 0, sumF = 0, sumR = 0;
    for (i = 0; i < 5; i++) {
      ADC1_ReadAllSensorsFiltered(&left, &front, &right);
      sumL += left; sumF += front; sumR += right;
      SysTick_Wait(40000); // 2.5ms between samples
    }
    left = sumL / 5;
    front = sumF / 5;
    right = sumR / 5;

    g_left  = left;
    g_front = front;
    g_right = right;

    //  Noise spike detection 
    uint16_t diffL = (left  > prev_left)  ? (left  - prev_left)  : (prev_left  - left);
    uint16_t diffR = (right > prev_right) ? (right - prev_right) : (prev_right - right);
    uint16_t diffF = (front > prev_front) ? (front - prev_front) : (prev_front - front);
    prev_left  = left;
    prev_right = right;
    prev_front = front;

    if (diffL > 400 || diffR > 550){
      StopMotors();
      LED = Red;
      Wait10ms(8);
      continue;
    }

    //  Too close: Stop 
    if ((front > (FRONT_10_ADC + FRONT_10_BAND)) ||
        (left  > (LEFT_10_ADC  + LEFT_10_BAND))  ||
        (right > (RIGHT_10_ADC + RIGHT_10_BAND))){
      if (stable_mode != 2) {
        StopMotors();
        LED = Purple;
        stable_mode = 2;
      }
      Wait10ms(8);
      continue;
    }

    // --- Ideal distance (20 60 cm): move forward ---
    if (front < (FRONT_20_ADC - 60) && front > (FRONT_70_ADC + 60)){
      if (stable_mode != 1) {
        MoveForward();
        LED = Green;
        stable_mode = 1;
      }
    }


    //   TURNING LOGIC   with right sensor bias + filtering
 

    // Apply stronger bias correction for right sensor
    if (right > 100) right += 180;   // compensate for weaker sensor response

    // Dynamic relative difference
    int16_t lr_diff = (int16_t)left - (int16_t)right;

    // Use slightly asymmetric thresholds for balance
    uint16_t left_threshold  = LEFT_20_ADC + LEFT_20_BAND + 200;
    uint16_t right_threshold = RIGHT_20_ADC + RIGHT_20_BAND;   // much lower pickup threshold

    // Left stronger turn right
    if ((lr_diff > 300 && left > left_threshold) || (left > right + 400)) {
      turn_counter++;
      if (turn_counter > 2 || turn_state != 1) {
        RightTurnBackward();
        LED = Cyan;
        turn_state = 1;
        turn_counter = 0;
      }
    }

    // Right stronger turn left 
    else if ((lr_diff < -200 && right > right_threshold) || (right + 300 < left)) {
      turn_counter++;
      if (turn_counter > 2 || turn_state != 2) {
        LeftTurnBackward();
        LED = Orange;
        turn_state = 2;
        turn_counter = 0;
      }
    }

    else {
      turn_state = 0;
      turn_counter = 0;
    }

    
    //  Minor forward turning adjustments (smoothed)
    if (((left  < (LEFT_20_ADC  - LEFT_20_BAND + 100)) && 
          (left  > (LEFT_70_ADC  - LEFT_70_BAND + 100))) ||
        ((right < (RIGHT_20_ADC - RIGHT_20_BAND + 350)) &&   // widened band for right
          (right > (RIGHT_70_ADC - RIGHT_70_BAND - 150)))){  // relaxed lower limit

      // Left side sees object curve left
      if ((left < (LEFT_20_ADC - LEFT_20_BAND + 180)) &&
          (left > (LEFT_70_ADC - LEFT_70_BAND + 180))){
        LeftTurnForward();
        LED = Blue;
        stable_mode = 0;
      }

      // Right side sees object curve right
      if ((right < (RIGHT_20_ADC - RIGHT_20_BAND + 400)) &&  // widened upper band
          (right > (RIGHT_70_ADC - RIGHT_70_BAND - 150))){   // extended lower band
        RightTurnForward();
        LED = Blue;
        stable_mode = 0;
      }
    }

    Wait10ms(8);  // ~80ms decision loop
  }
}



// Wall Follower Mode 

void wall_follower(void) {
  uint16_t left, right, front;
  uint8_t i;

  LED = Green;  // active mode indicator

  // --- Prime ADC readings ---
  for (i = 0; i < 10; i++) {
    ADC1_ReadAllSensorsFiltered(&left, &front, &right);
    SysTick_Wait(80000);  // 5 ms delay
  }

  uint16_t prev_left  = left;
  uint16_t prev_right = right;
  uint16_t prev_front = front;

  // wall_side = 1  Left wall follower
  // wall_side = 2  Right wall follower
  while (mode == WALL_FOLLOWER) {
    // --- Average 5 samples to reduce noise ---
    uint32_t sumL = 0, sumF = 0, sumR = 0;
    for (i = 0; i < 5; i++) {
      ADC1_ReadAllSensorsFiltered(&left, &front, &right);
      sumL += left; sumF += front; sumR += right;
      SysTick_Wait(40000);  // 2.5 ms between samples
    }
    left  = sumL / 5;
    front = sumF / 5;
    right = sumR / 5;

    g_left  = left;
    g_front = front;
    g_right = right;

    // Compute differences for sudden-change detection
    uint16_t diffL = (left  > prev_left)  ? (left  - prev_left)  : (prev_left  - left);
    uint16_t diffR = (right > prev_right) ? (right - prev_right) : (prev_right - right);
    uint16_t diffF = (front > prev_front) ? (front - prev_front) : (prev_front - front);
    prev_left  = left;
    prev_right = right;
    prev_front = front;

    // --- Corner Safety Backup (shared logic) 
    // If front < 10 cm and wall on followed side is near, back up
    if (wall_side == 2 && front > (FRONT_10_ADC + FRONT_10_BAND) &&
        right > (RIGHT_20_ADC - RIGHT_20_BAND)) {
      StopMotors();
      LED = Red;
      MoveBackward();
      Wait10ms(40);
      StopMotors();
      PivotLeft();
      Wait10ms(40);
      StopMotors();
      LED = Green;
      continue;
    }
    else if (wall_side == 1 && front > (FRONT_10_ADC + FRONT_10_BAND) &&
             left  > (LEFT_20_ADC - LEFT_20_BAND)) {
      StopMotors();
      LED = Red;
      MoveBackward();
      Wait10ms(40);
      StopMotors();
      PivotRight();
      Wait10ms(40);
      StopMotors();
      LED = Green;
      continue;
    }

    // --- Follow the Right Wall ---
    if (wall_side == 2) {
      if (right > 100) right += 100;  // compensate for weaker sensor

      uint16_t right_close = RIGHT_20_ADC + RIGHT_20_BAND + 400;   // steer left much sooner
      uint16_t right_far   = RIGHT_70_ADC - RIGHT_70_BAND + 80;    // steer right later

      // Too close to right wall: steer forward left (Orange)
      if (right > right_close) {
        LeftTurnForward_R();   // steer away early
        LED = Orange;
      }
      // steer forward right
      else {
        RightTurnForward_R();  // keep hugging wall
        LED = Cyan;
      }
    }

    // --- Follow the Left Wall ---
    else if (wall_side == 1) {
      if (left > 100) left += 100;  // compensate for weaker sensor

      uint16_t left_close = LEFT_20_ADC + LEFT_20_BAND + 400;   // steer right much sooner
      uint16_t left_far   = LEFT_70_ADC - LEFT_70_BAND + 80;    // steer left later

      // Too close to left wall: steer forward right (Orange) 
      if (left > left_close) {
        RightTurnForward_L();   // steer away early
        LED = Orange;
      }
      // steer forward left 
      else {
        LeftTurnForward_L();    // keep hugging wall
        LED = Cyan;
      }
    }

    Wait10ms(8);  // ~80 ms loop delay
  }

  // --- Exit cleanup ---
  StopMotors();
  LED = Blue;
}

// ===== System Initialization =====
void System_Init(void){
  DisableInterrupts();
  PLL_Init();               
  ADC1_SS1_Init();          
  Car_Dir_Init();
  Switch_Init();            
  LED_Init();
  SysTick_Init();
  PWM_PB54_Init();
  PWM_PB54_Duty(START_SPEED, START_SPEED);
  EnableInterrupts();	
}

void SysTick_Wait10ms(uint32_t delay) {
  volatile uint32_t i;
  for (i = 0; i < delay; i++) {
    SysTick_Wait(160000);  // 10ms delay at 16MHz
  }
}
