// LineFollower.c
// Line Following Robot Starter program 
// CECS 346
// Created by Hugo Razo, Peter Ly, Mark Mendez, Hector Perez 
// California State University, Long Beach
// 4/25/2025

#include "tm4c123gh6pm.h"
#include "Systick.h"
#include "Motors.h"
#include "Sensor.h"
#include "LED.h"
#include <stdint.h>

//Global Variables 
enum states curr_s;   // Initial FSM state
uint8_t Input;        // Sensor input

//FSM State Structure and Definitions
struct State {
  uint8_t motors;              // controls motor power supply
  uint16_t delay;              // Delay in ms
  uint8_t next[4];             // next state
};
typedef const struct State State_t;

enum states {Center, Left, Right, Stop, Stop2};

// FSM Table 
State_t linefollower_fsm[] = {
  {FORWARD,    100,  {Center, Left, Right, Stop}},   // Center
  {TURN_LEFT,  100,  {Center, Left, Right, Stop}},   // Left
  {TURN_RIGHT, 100,  {Center, Left, Right, Stop}},   // Right
  {STOP,       100,  {Center, Left, Right, Stop}}    // Stop
};

// === Function Prototypes ===
void System_Init(void);

// Initializes all hardware modules
void System_Init(void) {
  Sensor_Init();
  Motor_Init();
  LED_Init();
  SysTick_Init();
}

int main(void) {
  uint8_t Input;

  System_Init();       // Initialize all modules

  while (1) {
    // LED Feedback Based on Sensor Input 
    switch (Sensor_CollectData()) {
      case 0:  // Center
        LED = RED;
        break;
      case 1:  // Right
        LED = GREEN;
        break;
      case 2:  // Left
        LED = BLUE;
        break;
      case 3:  // Lost
        LED = WHITE;
        break;
      default:
        LED = DARK;
        break;
    }

    // FSM Engine 
    MOTORS = linefollower_fsm[curr_s].motors;          // Set motor action
    Wait_N_MS(linefollower_fsm[curr_s].delay);         // Delay
    Input = Sensor_CollectData();                      // Read sensor data (PE1, PE0)
    curr_s = linefollower_fsm[curr_s].next[Input];     // Transition to next state
  }
}