#include "keypad.h"
#include "DIO.h"
#include "LCD.h"
unsigned char keypad_arr[4][4] = {
  {'1','2','3','+'},
  {'4','5','6','-'},
  {'7','8','9','/'},
  {'*','0','#','='}
};
void keypad_init(void){
  //Columns
  DIO_Init(PORTE,P0,OUTPUT_PIN);
  DIO_Init(PORTE,P1,OUTPUT_PIN);
  DIO_Init(PORTE,P2,OUTPUT_PIN);
  DIO_Init(PORTE,P3,OUTPUT_PIN);
  //rows init
  DIO_Init(PORTC,P4,INPUT_PIN);
  DIO_Init(PORTC,P5,INPUT_PIN);
  DIO_Init(PORTC,P6,INPUT_PIN);
  DIO_Init(PORTC,P7,INPUT_PIN);
}

unsigned char keypad_read(void){

  delay(15);
  unsigned long portd_value = 0;
  while(1)
  {
      DIO_WritePort(PORTE,0x00);
      for (int j = 0 ; j<4 ; j++)
      {
          DIO_WritePin(PORTE,j,1);

          portd_value = read_port(PORTC);
          if(portd_value != 0x0d)
          {
          switch(portd_value)
            {
                case 0x1d:
                  return keypad_arr[0][j];
                  break;
                case 0x2d:
                  return keypad_arr[1][j];
                  break;
                case 0x4d:
                  return keypad_arr[2][j];
                  break;
                case 0x8d:
                  return keypad_arr[3][j];
                  break;
                default:
                  break;
            }
          }
        DIO_WritePort(PORTE,0x00);
      }
      return '!';
      break;
  }
}

uint8 PrintNow(short input)
{
  switch(input)
  {
  case 1:
    LCD_Char('1');
    return '1';
    break;
  case 2:
    LCD_Char('2');
    return '2';
    break;
  case 3:
    LCD_Char('3');
    return '3';
    break;
  case 4:
    LCD_Char('4');
    break;
  case 5:
    LCD_Char('5');
    break;
  case 6:
    LCD_Char('6');
    break;
  case 7:
    LCD_Char('7');
    break;
  case 8:
    LCD_Char('8');
    break;
  case 9:
    LCD_Char('9');
    break;
  case 0:
    LCD_Char('0');
    break;
  default:
    return '!';
    break;
      
  }
}