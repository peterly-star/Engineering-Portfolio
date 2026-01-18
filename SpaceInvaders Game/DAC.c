// Group number: 2 
// Group members: Peter Ly Hugo Razo Mark Mendez Hector Perez
#include <stdint.h>
#include "tm4c123gh6pm.h"
#include "DAC.h"

#define DAC_DATA   (*((volatile uint32_t *)0x4000503C))  // PB3-0

// Initialize 4-bit DAC on PB3-0
void DAC_Init(void){
  // 1) Enable clock for Port B, but DON'T turn off others
  SYSCTL_RCGCGPIO_R |= 0x02;          // bit1 = Port B clock
  volatile uint32_t delay = SYSCTL_RCGCGPIO_R; // allow clock to start (dummy read)

  // 2) Configure PB3-0 as simple digital outputs
  GPIO_PORTB_AMSEL_R &= ~0x0F;        // disable analog on PB3-0
  GPIO_PORTB_PCTL_R  &= ~0x0000FFFF;  // GPIO on PB3-0
  GPIO_PORTB_DIR_R   |= 0x0F;         // PB3-0 out
  GPIO_PORTB_AFSEL_R &= ~0x0F;        // regular port function
  GPIO_PORTB_DEN_R   |= 0x0F;         // enable digital on PB3-0
}

// 4-bit output to DAC
void DAC_Out(uint8_t data){
  data &= 0x0F;          // keep only lower 4 bits
  DAC_DATA = data;      
}
