// Nokia5110.c
// Runs on TM4C123
// CECS 347 Project 3 - Space Invaders
// SSI1 is used to interface with Nokia5110.
// Group number: 2
// Group members: Peter Ly Hugo Razo Mark Mendez Hector Perez

// SSI0 interface for Nokia 5110 LCD (PA7=RESET, PA6=DC, PA5=DIN, PA3=CE, PA2=CLK)

#include "Nokia5110.h"
#include "tm4c123gh6pm.h"
#include <stdint.h>

#define DC                      (*((volatile uint32_t *)0x40004100)) // PA6
#define DC_COMMAND              0
#define DC_DATA                 0x40
#define RESET                   (*((volatile uint32_t *)0x40004200)) // PA7
#define RESET_LOW               0
#define RESET_HIGH              0x80

enum typeOfWrite{
  COMMAND,                              // the transmission is an LCD command
  DATA                                  // the transmission is data
};
// The Data/Command pin must be valid when the eighth bit is
// sent.  The SSI module has hardware input and output FIFOs
// that are 8 locations deep.  Based on the observation that
// the LCD interface tends to send a few commands and then a
// lot of data, the FIFOs are not used when writing
// commands, and they are used when writing data.  This
// ensures that the Data/Command pin status matches the byte
// that is actually being transmitted.
// The write command operation waits until all data has been
// sent, configures the Data/Command pin for commands, sends
// the command, and then waits for the transmission to
// finish.
// The write data operation waits until there is room in the
// transmit FIFO, configures the Data/Command pin for data,
// and then adds the data to the transmit FIFO.

// This is a helper function that sends an 8-bit message to the LCD.
// inputs: type     COMMAND or DATA
//         message  8-bit code to transmit
// outputs: none
// assumes: SSI2 and corresponding GPIO port have already been initialized and enabled
static void lcdwrite(enum typeOfWrite type, char message){
  if(type == COMMAND){
    DC = DC_COMMAND;   // command mode
  } else{
    DC = DC_DATA;      // data mode
  }
  while((SSI0_SR_R & SSI_SR_TNF) == 0){}; // wait until transmit FIFO not full
  SSI0_DR_R = message;                     // transmit data
  while((SSI0_SR_R & SSI_SR_BSY) != 0){}; // wait until done sending
}

void Nokia5110_Init(void){
  volatile uint32_t delay;
  SYSCTL_RCGCGPIO_R |= 0x01;    // activate port A
  SYSCTL_RCGCSSI_R  |= 0x01;    // activate SSI0
  delay = SYSCTL_RCGCGPIO_R;    // allow time for clock to start

  GPIO_PORTA_DIR_R |= 0xC0;     // PA6 (DC) and PA7 (RESET) as output
  GPIO_PORTA_AFSEL_R |= 0x2C;   // enable alt function on PA2,3,5
  GPIO_PORTA_AFSEL_R &= ~0xC0;  // disable alt function on PA6,7
  GPIO_PORTA_DEN_R |= 0xEC;     // enable digital I/O on PA2,3,5,6,7
  GPIO_PORTA_PCTL_R = (GPIO_PORTA_PCTL_R & 0xFF00F0FF) | 0x00202200; // configure SSI0 on PA2,3,5
  GPIO_PORTA_AMSEL_R = 0;       // disable analog

  // --- SSI Configuration ---
  SSI0_CR1_R = 0x00000000;      // disable SSI for configuration
  SSI0_CC_R = 0x00;             // use system clock (80 MHz)
  SSI0_CPSR_R = 40;             // 80 MHz / 40 = 2 MHz clock (safe for LCD)
  SSI0_CR0_R = 0x0007;          // 8-bit data, Freescale format, SPO=0, SPH=0
  SSI0_CR1_R |= 0x00000002;     // enable SSI

  // --- Hardware Reset with delay ---
  RESET = RESET_LOW;            // drive reset low
  for(volatile int d = 0; d < 100000; d++); // ~1–2ms delay
  RESET = RESET_HIGH;           // release reset
  for(volatile int d = 0; d < 100000; d++); // allow LCD to start

  // --- Initialization sequence ---
  lcdwrite(COMMAND, 0x21);      // extended instruction set
  lcdwrite(COMMAND, 0xA6);      // contrast (lowered from 0xB9 to 0xB1)
  lcdwrite(COMMAND, 0x04);      // temperature coefficient
  lcdwrite(COMMAND, 0x14);      // bias mode 1:48
  lcdwrite(COMMAND, 0x20);      // basic instruction set
  lcdwrite(COMMAND, 0x0C);      // normal display mode

  Nokia5110_Clear();            // clear the display
}


void Nokia5110_OutChar(unsigned char data){
  int i;
  lcdwrite(DATA, 0x00);                 // blank vertical line padding
  for(i=0; i<5; i=i+1){
    lcdwrite(DATA, ASCII[data - 0x20][i]);
  }
  lcdwrite(DATA, 0x00);                 // blank vertical line padding
}

//********Nokia5110_OutString*****************
// Print a string of characters to the Nokia 5110 48x84 LCD.
// The string will automatically wrap, so padding spaces may
// be needed to make the output look optimal.
// inputs: ptr  pointer to NULL-terminated ASCII string
// outputs: none
// assumes: LCD is in default horizontal addressing mode (V = 0)
void Nokia5110_OutString(char *ptr){
  while(*ptr){
    Nokia5110_OutChar((unsigned char)*ptr);
    ptr = ptr + 1;
  }
}

//********Nokia5110_OutUDec*****************
// Output a 16-bit number in unsigned decimal format with a
// fixed size of five right-justified digits of output.
// Inputs: n  16-bit unsigned number
// Outputs: none
// assumes: LCD is in default horizontal addressing mode (V = 0)
void Nokia5110_OutUDec(unsigned short n){
  if(n < 10){
    Nokia5110_OutString("    ");
    Nokia5110_OutChar(n+'0'); /* n is between 0 and 9 */
  } else if(n<100){
    Nokia5110_OutString("   ");
    Nokia5110_OutChar(n/10+'0'); /* tens digit */
    Nokia5110_OutChar(n%10+'0'); /* ones digit */
  } else if(n<1000){
    Nokia5110_OutString("  ");
    Nokia5110_OutChar(n/100+'0'); /* hundreds digit */
    n = n%100;
    Nokia5110_OutChar(n/10+'0'); /* tens digit */
    Nokia5110_OutChar(n%10+'0'); /* ones digit */
  }
  else if(n<10000){
    Nokia5110_OutChar(' ');
    Nokia5110_OutChar(n/1000+'0'); /* thousands digit */
    n = n%1000;
    Nokia5110_OutChar(n/100+'0'); /* hundreds digit */
    n = n%100;
    Nokia5110_OutChar(n/10+'0'); /* tens digit */
    Nokia5110_OutChar(n%10+'0'); /* ones digit */
  }
  else {
    Nokia5110_OutChar(n/10000+'0'); /* ten-thousands digit */
    n = n%10000;
    Nokia5110_OutChar(n/1000+'0'); /* thousands digit */
    n = n%1000;
    Nokia5110_OutChar(n/100+'0'); /* hundreds digit */
    n = n%100;
    Nokia5110_OutChar(n/10+'0'); /* tens digit */
    Nokia5110_OutChar(n%10+'0'); /* ones digit */
  }
}

//********Nokia5110_SetCursor*****************
// Move the cursor to the desired X- and Y-position.  The
// next character will be printed here.  X=0 is the leftmost
// column.  Y=0 is the top row.
// inputs: newX  new X-position of the cursor (0<=newX<=11)
//         newY  new Y-position of the cursor (0<=newY<=5)
// outputs: none
void Nokia5110_SetCursor(unsigned char newX, unsigned char newY){
  if((newX > 11) || (newY > 5)){        // bad input
    return;                             // do nothing
  }
  // multiply newX by 7 because each character is 7 columns wide
  lcdwrite(COMMAND, 0x80|(newX*7));     // setting bit 7 updates X-position
  lcdwrite(COMMAND, 0x40|newY);         // setting bit 6 updates Y-position
}

//********Nokia5110_Clear*****************
// Clear the LCD by writing zeros to the entire screen and
// reset the cursor to (0,0) (top left corner of screen).
// inputs: none
// outputs: none
void Nokia5110_Clear(void){
  int i;
  for(i=0; i<(MAX_X*MAX_Y/8); i=i+1){
    lcdwrite(DATA, 0x00);
  }
  Nokia5110_SetCursor(0, 0);
}

//********Nokia5110_DrawFullImage*****************
// Fill the whole screen by drawing a 48x84 bitmap image.
// inputs: ptr  pointer to 504 byte bitmap
// outputs: none
// assumes: LCD is in default horizontal addressing mode (V = 0)
void Nokia5110_DrawFullImage(const char *ptr){
  int i;
  Nokia5110_SetCursor(0, 0);
  for(i=0; i<(MAX_X*MAX_Y/8); i=i+1){
    lcdwrite(DATA, ptr[i]);
  }
}
char Screen[SCREENW*SCREENH/8]; // buffer stores the next image to be printed on the screen

//********Nokia5110_PrintBMP*****************
// Bitmaps defined above were created for the LM3S1968 or
// LM3S8962's 4-bit grayscale OLED display.  They also
// still contain their header data and may contain padding
// to preserve 4-byte alignment.  This function takes a
// bitmap in the previously described format and puts its
// image data in the proper location in the buffer so the
// image will appear on the screen after the next call to
//   Nokia5110_DisplayBuffer();
// The interface and operation of this process is modeled
// after RIT128x96x4_BMP(x, y, image);
// inputs: xpos      horizontal position of bottom left corner of image, columns from the left edge
//                     must be less than 84
//                     0 is on the left; 82 is near the right
//         ypos      vertical position of bottom left corner of image, rows from the top edge
//                     must be less than 48
//                     2 is near the top; 47 is at the bottom
//         ptr       pointer to a 16 color BMP image
//         threshold grayscale colors above this number make corresponding pixel 'on'
//                     0 to 14
//                     0 is fine for ships, explosions, projectiles, and bunkers
// outputs: none
void Nokia5110_PrintBMP(unsigned char xpos, unsigned char ypos, const unsigned char *ptr, unsigned char threshold){
  long width = ptr[18], height = ptr[22], i, j;
  unsigned short screenx, screeny;
  unsigned char mask;
  // check for clipping
  if((height <= 0) ||              // bitmap is unexpectedly encoded in top-to-bottom pixel order
     ((width%2) != 0) ||           // must be even number of columns
     ((xpos + width) > SCREENW) || // right side cut off
     (ypos < (height - 1)) ||      // top cut off
     (ypos > SCREENH))           { // bottom cut off
    return;
  }
  if(threshold > 14){
    threshold = 14;             // only full 'on' turns pixel on
  }
  // bitmaps are encoded backwards, so start at the bottom left corner of the image
  screeny = ypos/8;
  screenx = xpos + SCREENW*screeny;
  mask = ypos%8;                // row 0 to 7
  mask = 0x01<<mask;            // now stores a mask 0x01 to 0x80
  j = ptr[10];                  // byte 10 contains the offset where image data can be found
  for(i=1; i<=(width*height/2); i=i+1){
    // the left pixel is in the upper 4 bits
    if(((ptr[j]>>4)&0xF) > threshold){
      Screen[screenx] |= mask;
    } else{
      Screen[screenx] &= ~mask;
    }
    screenx = screenx + 1;
    // the right pixel is in the lower 4 bits
    if((ptr[j]&0xF) > threshold){
      Screen[screenx] |= mask;
    } else{
      Screen[screenx] &= ~mask;
    }
    screenx = screenx + 1;
    j = j + 1;
    if((i%(width/2)) == 0){     // at the end of a row
      if(mask > 0x01){
        mask = mask>>1;
      } else{
        mask = 0x80;
        screeny = screeny - 1;
      }
      screenx = xpos + SCREENW*screeny;
      // bitmaps are 32-bit word aligned
      switch((width/2)%4){      // skip any padding
        case 0: j = j + 0; break;
        case 1: j = j + 3; break;
        case 2: j = j + 2; break;
        case 3: j = j + 1; break;
      }
    }
  }
}
// There is a buffer in RAM that holds one screen
// This routine clears this buffer
void Nokia5110_ClearBuffer(void){int i;
  for(i=0; i<SCREENW*SCREENH/8; i=i+1){
    Screen[i] = 0;              // clear buffer
  }
}

//********Nokia5110_SetPixel*****************
// Turn on the specified pixel in the screen buffer.
// inputs: x - horizontal coordinate of the pixel, must be less than 84
//         y - vertical coordinate of the pixel, must be less than 48
//         Nokia5110_SetPixel(0, 0); turns on the upper left pixel
// outputs: none
// Pixel level functions by Tomas
void Nokia5110_SetPixel(unsigned char x, unsigned char y) {
  unsigned short PixelByte;            // byte# in screen buffer
  unsigned char PixelBit;              // bit# in byte
  if ((x<84) && (y<48)) {              // check screen boundaries
    PixelByte = ((y/8)*84) + x;
    PixelBit = y % 8;
    Screen[PixelByte] |= 1U<<PixelBit;
  }
}

//********Nokia5110_ClearPixel*****************
// Turn off the specified pixel in the screen buffer.
// inputs: x - horizontal coordinate of the pixel, must be less than 84
//         y - vertical coordinate of the pixel, must be less than 48
//         Nokia5110_ClearPixel(0, 0); turns off the upper left pixel
// outputs: none
// Pixel level functions by Tomas
void Nokia5110_ClearPixel(unsigned char x, unsigned char y) {
  unsigned short PixelByte;            // byte# in screen buffer
  unsigned char PixelBit;              // bit# in byte
  if ((x<84) && (y<48)) {              // check screen boundaries
    PixelByte = ((y/8)*84) + x;
    PixelBit = y % 8;
    Screen[PixelByte] &= ~(1U<<PixelBit);
  }
}

//********Nokia5110_DisplayBuffer*****************
// Fill the whole screen by drawing a 48x84 screen image.
// inputs: none
// outputs: none
// assumes: LCD is in default horizontal addressing mode (V = 0)
void Nokia5110_DisplayBuffer(void){
  Nokia5110_DrawFullImage(Screen);
}

