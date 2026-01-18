// ADC1SS1.c
// Runs on TM4C123
// Provide functions that initialize ADC1 SS2 to be triggered by
// software and trigger a conversion

// This file provide initialization function for three analog channels:
// PE2/AIN1
// PE1/AIN2
// PE0/AIN3

#include "tm4c123gh6pm.h"
#include <stdint.h>

// Initializes AIN2 (PE1), AIN3 (PE0), and AIN1 (PE2) sampling using ADC1 SS1
// 125k max sampling
// SS1 triggering event: software trigger, busy-wait sampling
// SS1 1st sample source: AIN2 (PE1)
// SS1 2nd sample source: AIN3 (PE0)
// SS1 3rd sample source: AIN1 (PE2)
// SS1 interrupts: enabled after 3rd sample but not promoted to controller
void ADC1_SS1_Init(void){
  volatile uint32_t delay;

  // 1) activate ADC1 and Port E clocks
  SYSCTL_RCGCADC_R |= 0x02;                 // ADC1 clock on
  SYSCTL_RCGCGPIO_R |= SYSCTL_RCGCGPIO_R4;  // Port E clock on
  delay = SYSCTL_RCGCGPIO_R;                // allow time for clock to stabilize
  delay = SYSCTL_RCGCGPIO_R;

  // 2) configure PE0, PE1, PE2 as analog inputs
  GPIO_PORTE_DIR_R   &= ~0x07;              // PE2-0 inputs
  GPIO_PORTE_AFSEL_R |=  0x07;              // enable alt func on PE2-0
  GPIO_PORTE_DEN_R   &= ~0x07;              // disable digital on PE2-0
  GPIO_PORTE_AMSEL_R |=  0x07;              // enable analog on PE2-0
  GPIO_PORTE_PCTL_R  &= ~0x00000FFF;        // clear PCTL for PE2-0 (not used for analog)

  // 3) configure ADC1, sequencer 1 for 3 samples: AIN2, AIN3, AIN1
ADC1_PC_R &= ~0xF;                        // clear max sample rate field
ADC1_PC_R |=  0x1;                        // 125K samples/sec
ADC1_SSPRI_R = 0x3210;                    // SS0 highest, SS3 lowest
ADC1_ACTSS_R &= ~0x0002;                  // disable SS1 during config
ADC1_EMUX_R  &= ~0x00F0;                  // SS1 trigger = software
ADC1_SSMUX1_R  = (2) | (3<<4) | (1<<8);   // MUX: 1st=AIN2(PE1), 2nd=AIN3(PE0), 3rd=AIN1(PE2)
ADC1_SSCTL1_R = (0 << 0) | (0 << 4) | (0x06 << 8);  // END3 + IE3
ADC1_IM_R     &= ~0x0002;                 // disable SS1 interrupts (busy-wait)
ADC1_ACTSS_R  |=  0x0002;                 // enable SS1

}

//------------ADC1_InSS1------------
// Busy-wait Analog to digital conversion
// Input: none
// Output: three 12-bit result of ADC conversions
// Samples AIN2 (PE1), AIN3 (PE0), and AIN1 (PE2)
// 125k max sampling
// software trigger, busy-wait sampling
// data returned by reference
// ain2 (PE1) 0 to 4095
// ain3 (PE0) 0 to 4095
// ain1 (PE2) 0 to 4095
/*void ADC1_InSS1(unsigned long *ain2, unsigned long *ain3, unsigned long *ain1){
  ADC1_PSSI_R = 0x0002;            // 1) Initiate SS1
  while((ADC1_RIS_R&0x02)==0){};   // 2) Wait for conversion done
  *ain2 = ADC1_SSFIFO1_R&0xFFF;    // 3A) Read first result, LEFT
  *ain3 = ADC1_SSFIFO1_R&0xFFF;    // 3B) Read second result, FRONT
  *ain1 = ADC1_SSFIFO1_R&0xFFF;    // 3C) Read third result, RIGHT
  ADC1_ISC_R = 0x0002;             // 4) Acknowledge completion
}
*/
void ADC1_InSS1(uint16_t *left, uint16_t *front, uint16_t *right)
{
    // Example implementation — reading SS1 FIFO for 3 sensors
    ADC1_PSSI_R = 0x0002;                  // initiate SS1
    while((ADC1_RIS_R & 0x02) == 0){}      // wait for conversion complete
    *left  = ADC1_SSFIFO1_R & 0xFFF;       // read left channel
    *front = ADC1_SSFIFO1_R & 0xFFF;       // read front channel
    *right = ADC1_SSFIFO1_R & 0xFFF;       // read right channel
    ADC1_ISC_R = 0x0002;                   // acknowledge completion
}


uint16_t median(uint16_t u1, uint16_t u2, uint16_t u3)
{
    uint16_t result;
    
    if (u1 > u2)
        if (u2 > u3)
            result = u2;     // u1>u2>u3
        else if (u1 > u3)
            result = u3;     // u1>u3>u2
        else
            result = u1;     // u3>u1>u2
    else if (u3 > u2)
        result = u2;         // u3>u2>u1
    else if (u1 > u3)
        result = u1;         // u2>u1>u3
    else
        result = u3;         // u2>u3>u1
    
    return result;
}

// Read all three sensors with median filtering
void ADC1_ReadAllSensorsFiltered(uint16_t *left, uint16_t *front, uint16_t *right)
{
    // Static variables to keep history for median filtering
    static uint16_t left_oldest = 0, left_middle = 0;
    static uint16_t front_oldest = 0, front_middle = 0;
    static uint16_t right_oldest = 0, right_middle = 0;
    
    uint16_t left_newest, front_newest, right_newest;
    
    // Read newest values
    ADC1_InSS1(&left_newest, &front_newest, &right_newest);
    
    // Calculate median for each sensor
    *left = median(left_newest, left_middle, left_oldest);
    *front = median(front_newest, front_middle, front_oldest);
    *right = median(right_newest, right_middle, right_oldest);
    
    // Update history
    left_oldest = left_middle;
    left_middle = left_newest;
    
    front_oldest = front_middle;
    front_middle = front_newest;
    
    right_oldest = right_middle;
    right_middle = right_newest;
}
