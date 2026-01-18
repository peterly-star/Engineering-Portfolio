// switch.c
// Runs on TM4C123
//Peter Ly Hugo Razo Mark Mendez Hector Perez
// Configure PF0 (SW2) and PF4 (SW1) as input switches with interrupts
// Used for Space Invaders (SW2 starts the game)

#include "tm4c123gh6pm.h"
#include <stdint.h>

void Switch_Init(void) {
  SYSCTL_RCGCGPIO_R |= 0x20;      // activate clock for Port F
  while((SYSCTL_PRGPIO_R & 0x20) == 0) {}; // wait for clock

  GPIO_PORTF_LOCK_R = 0x4C4F434B; // unlock Port F
  GPIO_PORTF_CR_R |= 0x11;        // allow changes to PF0, PF4
  GPIO_PORTF_DIR_R &= ~0x11;      // make PF0, PF4 inputs
  GPIO_PORTF_DEN_R |= 0x11;       // enable digital I/O
  GPIO_PORTF_PUR_R |= 0x11;       // enable pull-ups on PF0, PF4

  // Interrupt setup
  GPIO_PORTF_IS_R &= ~0x11;       // edge-sensitive
  GPIO_PORTF_IBE_R &= ~0x11;      // not both edges

  // Different edge triggers:
  GPIO_PORTF_IEV_R &= ~0x01;      // PF0 (SW2) -> falling edge (press to start)
  GPIO_PORTF_IEV_R |=  0x10;      // PF4 (SW1) -> rising edge (release to shoot)

  GPIO_PORTF_ICR_R = 0x11;        // clear any prior interrupt flags
  GPIO_PORTF_IM_R |= 0x11;        // arm interrupts on PF0, PF4

  NVIC_PRI7_R = (NVIC_PRI7_R & 0xFF00FFFF) | 0x00A00000; // priority 5
  NVIC_EN0_R |= 0x40000000;       // enable interrupt 30 (Port F)
}

