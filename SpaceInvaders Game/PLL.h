// PLL.h
// Runs on TM4C123
// CECS 347 Project 3 - Space Invaders
// Group number: 2 
// Group members:Peter Ly Hugo Razo Mark Mendez Hector Perez

// TODO:
// Determine the #define statement SYSDIV initializes
// the PLL to the desired frequency.
#define SYSDIV2  4 
// bus frequency is 400MMHz/(SYSDIV+1)

void PLL_Init(void);
