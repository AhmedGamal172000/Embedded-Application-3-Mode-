#include "calc.h"

extern double take_in_x = 0;
extern double take_in_y = 0;
extern int take_in_answer = 0;

void Calc_Page()
{
 
  take_in_x = 0;
  static uint8 take_in_hold = '!';
  take_in_y = 0;
  take_in_answer = 0;
  uint8 index = 0;
  uint8 calc_STATE = 'x';
  uint8 sign_hold;
  
  while (INTERRUPT != '1') 
  {   
      delay(200);
  
      if(stop_welcome == 0)
      {
         LCD_Command(ClearScreen);
         LCD_String("Welcome in calc");
         LCD_Command(SecondLine);
         LCD_String("LOADING");
         delay(700);
         LCD_Char('.');
         delay(700);
         LCD_Char('.');
         delay(700);
         LCD_Char('.');
         delay(700);
         LCD_Command(ClearScreen);
         LCD_String("-> ");
         LCD_Command(BlinkingStart);
         stop_welcome = 1;  //lock the calc welcoming screen
      }
     
      switch(calc_STATE)
      {

      case 'x':
        take_in_hold = keypad_read();
        if (take_in_hold == '*' || take_in_hold == '/' || take_in_hold == '+' || take_in_hold == '-')
        {
          sign_hold = take_in_hold;
          LCD_Char(take_in_hold);
          calc_STATE ='y';
        }
        else if(take_in_hold != '!')
        {
            take_in_x =  (take_in_x * 10) + CharToNumber(take_in_hold);
            LCD_Char(take_in_hold);
        }
        take_in_hold = '!';
        break;  
        
      case 'y':
        take_in_hold = keypad_read();
         if (take_in_hold == '=')
        {
          LCD_Command(SecondLine);
           LCD_Char('=');
           take_in_hold = '!';
            calc_STATE ='z';
        }
        else if(take_in_hold != '!')
        {
            take_in_y =  (take_in_y * 10) + CharToNumber(take_in_hold);
            LCD_Char(take_in_hold);
        }
        take_in_hold = '!';
        break;
      case 'z':
        switch(sign_hold)
        {
            case '+':
              take_in_answer = take_in_x + take_in_y;
              break;
            case'-':
              take_in_answer = take_in_x - take_in_y;
              break;
            case '/':
              if(take_in_x < take_in_y)
              {
                take_in_answer = take_in_x / take_in_y;
                PrintIntToLCD(take_in_answer);
                LCD_Char('.');
                take_in_answer = 1000*(take_in_x / take_in_y);

              }
              else
              {
                take_in_answer = take_in_x / take_in_y;
                PrintIntToLCD(take_in_answer);
                LCD_Char('.');
                take_in_answer = 10*(take_in_x/take_in_y) - (take_in_answer*10); 
              }
              break;
            case '*':
              take_in_answer = take_in_x * take_in_y;
              break;
        }
              take_in_hold = '!';
              calc_STATE = 'x';
              PrintIntToLCD(take_in_answer);
              LCD_Command(BlinkingStop);
              take_in_x = 0;
              take_in_y = 0;
              take_in_answer = 0;
              index = 0;
              stop_welcome = 0;
              delay(5000); 
              
        break;     
      }
  }

}