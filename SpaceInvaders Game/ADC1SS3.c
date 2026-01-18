// ADC1SS3.c
// Runs on TM4C123
// Provides analog input from potentiometer (PE5/AIN8)
// Used for player spaceship movement in Space Invaders
// Author:Peter Ly Hugo Razo Mark Mendez Hector Perez
// Date: November 2025

#include "tm4c123gh6pm.h"
#include <stdint.h>

// Bit masks for ADC1 Sequencer 3
#define ADC1_PSSI_SS3     0x00000008
#define ADC1_ISC_SS3      0x00000008
#define ADC1_RIS_SS3      0x00000008

// ADC1SS3_Init
// Initializes ADC1 Sample Sequencer 3 to read from channel 8 (PE5/AIN8)
// Max sample rate: 125,000 samples/sec
// Trigger: software
void ADC1SS3_Init(void){ 
  volatile uint32_t delay;
  SYSCTL_RCGCADC_R |= 0x02;       // Activate ADC1
  SYSCTL_RCGCGPIO_R |= 0x10;      // Activate clock for Port E
  delay = SYSCTL_RCGCGPIO_R;      // Allow time for clock to start

  GPIO_PORTE_DIR_R &= ~0x20;      // Make PE5 input
  GPIO_PORTE_AFSEL_R |= 0x20;     // Enable alternate function on PE5
  GPIO_PORTE_DEN_R &= ~0x20;      // Disable digital function
  GPIO_PORTE_AMSEL_R |= 0x20;     // Enable analog mode on PE5

  ADC1_PC_R = 0x01;               // Configure for 125K samples/sec
  ADC1_SSPRI_R = 0x0123;          // Sequencer 3 lowest priority
  ADC1_ACTSS_R &= ~0x08;          // Disable sample sequencer 3
  ADC1_EMUX_R &= ~0xF000;         // Seq3 is software trigger
  ADC1_SSMUX3_R = 8;              // Channel AIN8 (PE5)
  ADC1_SSCTL3_R = 0x0006;         // IE0, END0 (generate flag and end)
  ADC1_IM_R &= ~0x08;             // Disable interrupts
  ADC1_ACTSS_R |= 0x08;           // Enable sample sequencer 3
}

// Performs a single ADC conversion on channel 8
// Returns: 12-bit digital value (0 to 4095)
uint16_t ADC1SS3_In(void){ 
  uint16_t ADC_Value;
  ADC1_PSSI_R = ADC1_PSSI_SS3;                 // Start sampling
  while((ADC1_RIS_R & ADC1_RIS_SS3) == 0){};   // Wait for conversion to finish
  ADC_Value = ADC1_SSFIFO3_R & 0xFFF;          // Read 12-bit result
  ADC1_ISC_R = ADC1_ISC_SS3;                   // Acknowledge completion
  return ADC_Value;
}

// ADCValue_To_X_AXIS
// Converts an ADC reading [0,4095] to a position [0, max_x_axis]
// Used to map potentiometer value to spaceship X position
uint8_t ADCValue_To_X_AXIS(uint16_t ADCValue, uint8_t max_x_axis) {
  // Clamping for noisy ADC 
  if (ADCValue > 4000) ADCValue = 4000;
  if (ADCValue < 50)   ADCValue = 50;
  return (ADCValue * max_x_axis) / 4000;  // Scaled mapping
}
