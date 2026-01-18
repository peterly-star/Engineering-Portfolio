// Sensor.c
// Reflectance Sensor Test Module
// Original by: Hugo Razo, Peter Ly, Mark Mendez, Hector Perez
// Reorganized and documented: 5/3/2025
// California State University, Long Beach

#include "tm4c123gh6pm.h"
#include "Sensor.h"
#include "Systick.h"
#include <stdint.h>

#define SENSOR_CTRL         (*((volatile unsigned long *)0x4000500C))  // Port B Data Register (CTRL pins)
#define SENSORS             (*((volatile unsigned long *)0x4002400C))  // Port E Data Register (Sensor pins)

#define SYSCTL_RCGCGPIO_PORTB   SYSCTL_RCGCGPIO_R1
#define SYSCTL_RCGCGPIO_PORTE   SYSCTL_RCGCGPIO_R4

#define SENSOR_CTRL_PINS    0x03  // PB0 = CTRL EVEN, PB1 = CTRL ODD
#define SENSOR_PINS         0x03  // PE0 = Sensor 0, PE1 = Sensor 7

// This function performs a sensor read by charging and measuring reflectance sensors.
uint8_t Sensor_CollectData(void) {
    uint8_t sensor_data;

    // Disable IR LEDs (turn off emitters)
    SENSOR_CTRL &= ~SENSOR_CTRL_PINS;
    Wait_N_MS(2);

    // Enable IR LEDs (turn on emitters)
    SENSOR_CTRL |= SENSOR_CTRL_PINS;

    // Configure sensor pins as output and drive high to charge capacitors
    GPIO_PORTE_DIR_R |= SENSOR_PINS;     // Set PE0 and PE1 as outputs
    SENSORS |= SENSOR_PINS;              // Output HIGH on PE0 and PE1

    Wait_N_US(10);                       // Allow capacitors to charge for 10us

    // Configure sensor pins as input to measure discharge
    GPIO_PORTE_DIR_R &= ~SENSOR_PINS;    // Set PE0 and PE1 as inputs

    Wait_N_US(460);                      // Wait for discharge time

    // Read reflectance sensor values (higher value = more reflectance)
    sensor_data = SENSORS;

    // Turn off IR LEDs
    SENSOR_CTRL &= ~SENSOR_CTRL_PINS;
    Wait_N_MS(10);

    return sensor_data;
}

//  Port E Initialization 
// Initializes PE0 and PE1 for sensor input (Sensor 0 and Sensor 7)
void PortE_Init(void) {
    SYSCTL_RCGCGPIO_R |= SYSCTL_RCGCGPIO_PORTE;                 // Enable clock for Port E
    while ((SYSCTL_RCGCGPIO_R & SYSCTL_RCGCGPIO_PORTE) == 0);   // Wait until ready

    GPIO_PORTE_DIR_R   &= ~SENSOR_PINS;   // Set PE0 and PE1 as inputs
    GPIO_PORTE_AFSEL_R &= ~SENSOR_PINS;   // Disable alternate functions
    GPIO_PORTE_AMSEL_R &= ~SENSOR_PINS;   // Disable analog mode
    GPIO_PORTE_DEN_R   |= SENSOR_PINS;    // Enable digital I/O
}

// Port B Initialization 
// Initializes PB0 and PB1 to control IR LED emitters (CTRL EVEN/ODD)
void PortB_Init(void) {
    SYSCTL_RCGCGPIO_R |= SYSCTL_RCGCGPIO_PORTB;                 // Enable clock for Port B
    while ((SYSCTL_RCGCGPIO_R & SYSCTL_RCGCGPIO_PORTB) == 0);   // Wait until ready

    GPIO_PORTB_DIR_R   |= SENSOR_CTRL_PINS;   // Set PB0 and PB1 as outputs
    GPIO_PORTB_AFSEL_R &= ~SENSOR_CTRL_PINS;  // Disable alternate functions
    GPIO_PORTB_AMSEL_R &= ~SENSOR_CTRL_PINS;  // Disable analog mode
    GPIO_PORTB_DEN_R   |= SENSOR_CTRL_PINS;   // Enable digital I/O
}

// Initializes all necessary ports for reflectance sensor operation
void Sensor_Init(void) {
    PortB_Init();
    PortE_Init();
}