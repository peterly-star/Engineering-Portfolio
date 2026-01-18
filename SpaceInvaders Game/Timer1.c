// Timer1.c
// Periodic Timer1A used for DAC sound output
// Group number: 2 
// Group members: Peter Ly Hugo Razo Mark Mendez Hector Perez

#include <stdint.h>
#include "tm4c123gh6pm.h"
#include "Timer1.h"

static void (*PeriodicTask1)(void) = 0;

void Timer1_Init(void(*task)(void), uint32_t period){
  // 1) Activate TIMER1 clock
  SYSCTL_RCGCTIMER_R |= 0x02;           // enable clock for Timer1
  volatile uint32_t delay = SYSCTL_RCGCTIMER_R;
  (void)delay;

  PeriodicTask1 = task;

  // 2) Disable TIMER1A during config
  TIMER1_CTL_R   &= ~0x00000001;

  // 3) 32-bit timer, periodic mode, count-down
  TIMER1_CFG_R    = 0x00000000;
  TIMER1_TAMR_R   = 0x00000002;         // periodic mode
  TIMER1_TAILR_R  = period - 1;         // reload value
  TIMER1_TAPR_R   = 0;                  // no prescale

  // 4) Clear timeout flag and mask interrupt (off for now)
  TIMER1_ICR_R    = 0x00000001;         // clear timeout flag
  TIMER1_IMR_R   &= ~0x00000001;        // mask Timer1A timeout interrupt

  // 5) Enable IRQ 21 in NVIC (Timer1A)
  NVIC_EN0_R     |= 1 << 21;
}

void Timer1A_Handler(void){
  TIMER1_ICR_R = 0x00000001;       // acknowledge Timer1A timeout
  if(PeriodicTask1){
    (*PeriodicTask1)();            // execute user function 
  }
}
