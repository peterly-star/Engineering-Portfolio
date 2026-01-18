// LED.c
// Test code for reflectance sensors. 
// California State University, Long Beach
#include "tm4c123gh6pm.h"

#define LED_PINS  0x0E

void LED_Init(void){ 
  SYSCTL_RCGCGPIO_R |= SYSCTL_RCGCGPIO_R5;     	// activate F clock
	while ((SYSCTL_RCGCGPIO_R&SYSCTL_RCGCGPIO_R5)!=SYSCTL_RCGCGPIO_R5){} // wait for the clock to be ready
		
  GPIO_PORTF_AMSEL_R &= ~LED_PINS;        // disable analog function
  GPIO_PORTF_PCTL_R &= ~0x0000FFF0; 	    // GPIO pins  
  GPIO_PORTF_DIR_R |= LED_PINS;          	// PF3,PF2,PF1 output   
	GPIO_PORTF_AFSEL_R &= ~LED_PINS;        // no alternate function
  GPIO_PORTF_DEN_R |= LED_PINS;          	// enable digital pins PF3-PF1        
}

