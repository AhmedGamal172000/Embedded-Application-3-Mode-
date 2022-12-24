
#ifndef __keypad_H_
#define __keypad_H_
#include <math.h>
#include <stdio.h>
#include "LCD.h"
void keypad_init(void);
unsigned char keypad_read(void);
extern unsigned char keypad_arr[4][4];
uint8 PrintNow(short input);

#endif
