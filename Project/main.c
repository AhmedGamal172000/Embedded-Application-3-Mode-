#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include "inc/hw_ints.h"
#include "tm4c123gh6pm.h"
#include "inc/hw_types.h"
#include "driverlib/sysctl.h"
#include "driverlib/gpio.h"
#include "inc/hw_memmap.h"
#include "driverlib/systick.h"
#include "driverlib/interrupt.h"
#include "driverlib/timer.h"
#include "DIO.h"
#include "LCD.h"
#include "keypad.h"
#include "math.h"

/*****************Defines***********************/
#define WelcomeHome '0'
#define CalcPage '1'
#define TimerPage '2'
#define SWPage '3'

/*****************Global Variables**************/
uint8 STATE = '0';
uint8 hold = '!';
uint8 stop_welcome = 0;
uint8 INTERRUPT = '~';
uint8 pause = '1';
uint8 ticked = '0';

/************ calculator Global Variables *******/
double take_in_x = 0;
double take_in_y = 0;
int take_in_answer = 0;


/********** STOPWATCH Global Variables *********/
// X1 X0 : Y1 Y0
short y0 = 0;
short y1 = 0;
short x0 = 0;
short x1 = 0;


/**************** TIMER Global Variables *********/
short y0_t0 = 0;
short y1_t0 = 0;
short x0_t0 = 0;
short x1_t0 = 0;

/*******************ProtoType********************/
void Calc_Page();
void Welcome_Home();
void change_state(uint8 in);
void Timer_Page();
void SW_Page();
short CharToNumber(uint8 x);
void PrintIntToLCD(int answer);
void Interrupt_init();
void ISR_PortaHandler();
 
/***************** USING ASM TO ENABLE INTERRUPT IN PROCESSOR **************/
__asm("CPSIE I");

//*********************************************************************************************************
//***********************************************  MAIN  **************************************************
//*********************************************************************************************************
int main()
{
   Interrupt_init(); //PORTA init pin 2 & init Interrupt in PORTA for p2
   keypad_init(); // init keypad 4x4 portc as input p4-P7 --- PORTE AS OUTPUT for rolling 1 on p0-p3 
   LCD_Init(); //lcd init on portB
   LCD_Command(ClearScreen); //clear screen for any last printed things
   while(1)
   {
     INTERRUPT = '~'; // AS a flag for there is no interrupt happened
     switch(STATE)
     { 
       case WelcomeHome:
         LCD_Command(ClearScreen);
         LCD_Command(ReturnHome);
         Welcome_Home();
         break;     
       case CalcPage:
         Calc_Page();
         stop_welcome = 0;
         break;
       case TimerPage:
         LCD_Command(ClearScreen);
         Timer_Page();
         break;
       case SWPage:
         LCD_Command(ClearScreen);
         SW_Page();
         break;
       default:
         break;
      }
   }
   return 0;
}
//*****************************************************************************************************
//***************************************** All ISRs **************************************************
//*****************************************************************************************************


/****************************************** ISR PORTA ************************************************/
void ISR_PortaHandler()
{
  //interrupt caused by PA2
  //PA2 interrupt set for returning to welcome me from any STATE
  if((GPIO_PORTA_MIS_R  & 0x04) == 0x04)
  {
      Set_Bit(GPIO_PORTA_ICR_R,P2); //Clearing Interrupt flag caused by this pin 
      INTERRUPT = '1';
      ticked='0';
      pause = '1';
   
      if(STATE == SWPage)
      {
        NVIC_ST_CTRL_R = 0x00;
      }
      else if(STATE == TimerPage)
      {
        TimerDisable(TIMER0_BASE,TIMER_A);
      }
      else if(STATE == WelcomeHome)
      {
        NVIC_ST_CTRL_R = 0x00;
        TimerDisable(TIMER0_BASE,TIMER_A);
      }
      STATE = WelcomeHome;
  }
  //interrupt caused by PA3
  //PA3 interrupt set for start the StopWatch OR TIMER MODEs
  else if((GPIO_PORTA_MIS_R & 0x08) == 0x08)
  {
       pause = '0';
       Set_Bit(GPIO_PORTA_ICR_R,P3); //Clearing Interrupt flag caused by this pin 
  }
}
/******************************************** ISR PORTF *****************************************/
void ISR_PortfHandler()
{
  //interrupt caused by PF0
  //PF0 is for resetting the stopwatch
  if((GPIO_PORTF_MIS_R  & 0x01) == 0x01)
  {
      Set_Bit(GPIO_PORTF_ICR_R,P0); //Clearing Interrupt flag caused by this pin 
      x0 = 0;
      x1=0;
      y0=0;
      y1=0;
      LCD_Command(ClearScreen);
      LCD_Command(ReturnHome);
      LCD_String("00:00");
      
      
  }
  //interrupt caused by PF4
  //PF4 is for pausing the timer or stopwatch
  else if((GPIO_PORTF_MIS_R & 0x10) == 0x10)
  {
    Set_Bit(GPIO_PORTF_ICR_R,P4); //Clearing Interrupt flag caused by this pin 
    pause = '1';
  }
}
/*********************************** ISR Timer0 **********************************************/

void ISR_Timer0Handler()
{
  ticked = '1';
  TIMER0_ICR_R = 0x1;
}
//*********************************************************************************************
//*********************************** All ISRs  *END*  *****************************************
//**********************************************************************************************

//**************SET INTERRUPT TO EACH TARGET PIN *************************************************
void Interrupt_init()
{
  DIO_Init(PORTF,P0,INPUT_PIN);
  DIO_Init(PORTF,P4,INPUT_PIN);
  DIO_Init(PORTA,P2,INPUT_PIN);
  DIO_Init(PORTA,P3,INPUT_PIN);
  DIO_InterruptInit(PORTF,P4); //pause stopwatch
  DIO_InterruptInit(PORTF,P0); //reset stopwatch
  DIO_InterruptInit(PORTA,P2); //global interrupt to return to welcome screen
  DIO_InterruptInit(PORTA,P3); //start stopwatch
  
}
/******************************* ISR SystickTimer *******************************************/
void SysHandler()
{ 
  ticked = '1';
}

// //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/***************************************************************************************************

************************************ Screens Functions *********************************************

****************************************************************************************************/



/*****************************************************************************
******************* OUR FIRST SCREEN (WELCOME SCREEN) **********************
****************************************************************************/

// Taking from user only 1-calc 2-Timer 3-STOPWATCH
// Any other input will not affect the flow

void Welcome_Home()
{
  STATE = '!';
  LCD_Command(ClearScreen);
  delay(50);
  LCD_String("Welcome  1-calc");
  LCD_Command(SecondLine);
  LCD_String("2-Timer 3-SW");
  while((hold != '1' && hold != '2' && hold !='3'))
  {
    hold = keypad_read();
  }
  change_state(hold);
}
/************************************************************************
*********************  Calculator Funtion (Calculator Screen) ***********
*************************************************************************/

//  INTERRUPT flag will be 1 only in the ISR_PortaHandler
//  BUT INTERRUPT flag with 0 value ... while no INTERRUPT to welcome home happened

//      This calc depend on X "Sign" Y = ANSWER
//default is set to start reading x until a sign of * / + - is written. each have it's if condition

//      stop_welcome   ----> is a lock for calc welcoming screen to prevent re-printing this on each iteration
//      sign_hold      ----> is for saving the sign to get the equation in a switch case on the sign according to the sign_hold
//      take_in_hold   ----> is for saving values of any input
//      take_in_x      ----> is for saving values of x input
//      take_in_y      ----> is for saving values of y input

//at the end of the function we reset all variables according to the default state to use the calc again until INTERRUPT happen

void Calc_Page()
{
 
  take_in_x = 0;
  static uint8 take_in_hold = '!';
  take_in_y = 0;
  take_in_answer = 0;
  static uint8 index = 0;
  static uint8 calc_STATE = 'x';
  static uint8 sign_hold;
  
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
              if(take_in_x < take_in_y)
              {
                LCD_Char('-');
                take_in_answer = take_in_y - take_in_x; 
              }
              else
              {
                take_in_answer = take_in_x - take_in_y;                
              }

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
/****************************************************************************/

/************************************************************************
*********************  Timer Funtion (Timer Screen) *********************
*************************************************************************/

//************************* X1_T0   X0_T0   :   Y1_T0   Y0_T0 *****************************************

// This Function is for setting a time to start count down till zero then a led will toggle for 3 sec and Timer Finished will printed

//      First we initialize Timer0
//      First While is for entering the 00:00 all values. means 4 inputs by user
//      so the user will counter i = 4 inputs so the i varibale incremented until i = 5  then the while will break
//      Then Counting down until reached 00:00
void Timer_Page()
{
  DIO_Init(PORTD,P3,OUTPUT_PIN);
  SysCtlPeripheralEnable(SYSCTL_PERIPH_TIMER0);
  while(!SysCtlPeripheralReady(SYSCTL_PERIPH_TIMER0)){}
  IntEnable(INT_TIMER0A);
  TimerDisable(TIMER0_BASE,TIMER_A);

  TimerIntEnable(TIMER0_BASE, TIMER_TIMA_TIMEOUT);
  //TimerIntRegister(TIMER0_BASE,TIMER_A,ISR_Timer0Handler);
  TimerConfigure(TIMER0_BASE,TIMER_CFG_PERIODIC);
  TimerControlStall(TIMER0_BASE,TIMER_A,true);
  TimerLoadSet(TIMER0_BASE,TIMER_A,16000000);
  
  STATE = '!';
  LCD_String("Welcome in Timer");
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
   LCD_String("->  :");
   LCD_Command(ShiftCursorLeft);
   LCD_Command(ShiftCursorLeft);
   LCD_Command(ShiftCursorLeft);
   LCD_Command(BlinkingStart);
   static short i=0;
   static uint8 input = '~';
   static short input_int = 0;
   x0_t0 = 0;
   y1_t0 = 0;
   x1_t0 = 0;
   y0_t0 = 0;
  
  PrintNow(x1_t0);
  PrintNow(x0_t0);
  LCD_Char(':');
  PrintNow(y1_t0);
  PrintNow(y0_t0);
  LCD_Command(ShiftCursorLeft);
  LCD_Command(ShiftCursorLeft);
  LCD_Command(ShiftCursorLeft);
  LCD_Command(ShiftCursorLeft);
  LCD_Command(ShiftCursorLeft);
   i = 0;
   while(i!=5)
   {
     input = keypad_read();
     delay(100);
     if( input != '!')
     {
       input_int = CharToNumber(input);
       i++;
       input = '!';
       delay(100);
       switch(i)
       { 
       case 1:
         x1_t0 = input_int;
         PrintNow(x1_t0);
         delay(500);
         break;
       case 2:
         x0_t0 = input_int;
         PrintNow(x0_t0);
         LCD_Command(ShiftCursorRight);
         break;
       case 3:
         y1_t0 = input_int;
         PrintNow(y1_t0);
         break;
       case 4:
         y0_t0 = input_int;
         PrintNow(y0_t0);
         i++;
         break;
       }
     }
   }
   ticked='0';
  LCD_Command(BlinkingStop);
  TimerEnable(TIMER0_BASE,TIMER_A);
  // x1 x0 : y1 y0
  while(INTERRUPT != '1')
  { 
    
    if(ticked=='1')
    {
        if(pause =='0')
        {
          LCD_Command(ShiftCursorLeft);
          y0_t0--;
          if(x0_t0 == 0 && x1_t0 == 0 && y1_t0 == 0 && y0_t0 == 0)
          {
            TimerDisable(TIMER0_BASE,TIMER_A);
            PrintNow(y0_t0);
                LCD_Command(SecondLine);
                LCD_String("Timer Finished");
                DIO_WritePin(PORTD,P3,1);
                delay(1000);
                DIO_WritePin(PORTD,P3,0);    
                delay(1000);
                DIO_WritePin(PORTD,P3,1);
                delay(1000);
                DIO_WritePin(PORTD,P3,0);  
                delay(1000);
                DIO_WritePin(PORTD,P3,1); 
                delay(1000);
                DIO_WritePin(PORTD,P3,0);  
                pause = '1';
          }
          else if(y0_t0>=0)
          {
              PrintNow(y0_t0);
          }
          else if(y0_t0 < 0)
          {
             y0_t0 = 9;
             PrintNow(y0_t0);
             LCD_Command(ShiftCursorLeft);
             LCD_Command(ShiftCursorLeft);
             y1_t0--;
             if(y1_t0 >=0)
             {
               PrintNow(y1_t0);
             }
             else if(y1_t0 < 0)
             {
               y1_t0 = 5;
               PrintNow(y1_t0);
             LCD_Command(ShiftCursorLeft);               
             LCD_Command(ShiftCursorLeft);
             LCD_Command(ShiftCursorLeft);
                x0_t0--;
                if(x0_t0 >=0)
                {
                  PrintNow(x0_t0);
                }
                else if(x0_t0 < 0)
                {
                  x0_t0 = 9;
                  PrintNow(x0_t0);
                  x1_t0--;
                  
                  LCD_Command(ShiftCursorLeft);
                  LCD_Command(ShiftCursorLeft);
                  if(x1_t0 >= 0)
                  {
                    PrintNow(x1_t0);
                    LCD_Command(ShiftCursorRight); 
                  }
                  else if(x1_t0 < 0)
                  {
                    LCD_Command(ShiftCursorRight); 
                  }
                  
                }
             LCD_Command(ShiftCursorRight);               
             LCD_Command(ShiftCursorRight);
              
             }
             LCD_Command(ShiftCursorRight);
          }

        }
        ticked = '0';
  }
}
}
  

/****************************************************************************/

/************************************************************************
*********************  StopWatch Funtion  *******************************
*************************************************************************/

//************************* X1 X0 : Y1 Y0 ***********************************
// This Function is for Counting up from 00:00 till 59:59

//      First we initialize Systick Timer with interrupt mode
//      Then Counting up until reached 59:59

void SW_Page()
{
  
  //Init the systick timer in interrupt mode to start using the STOPWATCH
  NVIC_ST_RELOAD_R = 16000000;
  NVIC_ST_CTRL_R = 7;
  //Init the PF0 & PF4 as PF0 ---->RESET // PF4---->Pause // PA3 ---->start 
  
  STATE = '!';
  pause = '1';
  ticked = '0';
 LCD_Command(ClearScreen);
         LCD_String("Welcome in SW");
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


  
         
  PrintNow(x1);
  PrintNow(x0);
  LCD_Char(':');
  PrintNow(y1);
  PrintNow(y0);
  
  while(INTERRUPT != '1')
  { 
    if(ticked=='1')
    {

        if(pause =='0')
        {     
              LCD_Command(ShiftCursorLeft);
              short i = NVIC_ST_CTRL_R;
              y0++;
              if(y0<10)
              {
                 PrintNow(y0);
              }
              else if(y0 >= 10)
              {
                y0 = 0;
                PrintNow(y0);
                LCD_Command(ShiftCursorLeft);
                y1++;
                LCD_Command(ShiftCursorLeft);
                if(y1<6)
                {
                  PrintNow(y1);
                }
                else if(y1>=6)
                {
                  PrintNow(0);
                }
                LCD_Command(ShiftCursorRight);
                if(y1 >= 6) 
                {
                  LCD_Command(ShiftCursorLeft);
                  LCD_Command(ShiftCursorLeft);
                  LCD_Command(ShiftCursorLeft);
                  LCD_Command(ShiftCursorLeft);
                  y1 =0;
                  y0 =0;
                  x0++;
                  if(x0<10)
                  {
                     PrintNow(x0);
                  }
                  else if(x0>=10)
                  {
                     PrintNow(0);
                  }
                 
                  LCD_Command(ShiftCursorRight);
                  LCD_Command(ShiftCursorRight);
                  LCD_Command(ShiftCursorRight);
                  if(x0 >=10)
                  {
                    LCD_Command(ShiftCursorLeft);
                    LCD_Command(ShiftCursorLeft);
                    LCD_Command(ShiftCursorLeft);
                    LCD_Command(ShiftCursorLeft);
                    LCD_Command(ShiftCursorLeft);
                    x0 =0;
                    x1++;
                    PrintNow(x1);
                    LCD_Command(ShiftCursorRight);
                    LCD_Command(ShiftCursorRight);
                    LCD_Command(ShiftCursorRight);
                    LCD_Command(ShiftCursorRight);
                  }//if x0>=10
                 }//y1>=6
                } // if y0>=10
          } //if pause 
        ticked ='0';
     }//while 
  }
}


/************************************************************************************************

************************************* Functions Helped us ***************************************

*************************************************************************************************/

/*******************************************************************************
********************************** change_state function ***********************
*******************************************************************************/

// this fucntion is for selecting from the welcome screen a state to start using this choosing state
// by taking input at welcome function of 1- calc 2- Timer 3- Stopwatch

void change_state(uint8 in)
{
  switch(in){
     
  
  case '1':
    STATE ='1';
    hold = '!';
    break;
  case '2':
    STATE = '2';
    hold = '!';
    break;
  case '3':
    STATE ='3';
    hold = '!';
    break;
  case '0':
    STATE = '0';
    hold = '!';
    break;
  default:
    STATE = '!';
    hold = '!';
    break;
  }
  
}


/*******************************************************************************
******************************* Char to Integer function ***********************
*******************************************************************************/

// According to that we take an characters from the input of the keypad
// there is a function needed to switch on each number and converted it into an interger to start using it in equations
// this function is returning short as we apply this on 1 digit level so no need to waste memory with int or double
// i is a variable incremented according to the case to write return statment for ONLY 1 time at the end of the function
// as this function used in 1 digit level
short CharToNumber(uint8 x)
{
  short i = 0;
  switch(x)
  {
   
  case '1':
    i= i +1;
    break;
  case '2':
    i =i +2;
    break;
  case '3':
    i =i +3;
    break;
  case '4':
    i =i +4;
    break;
  case '5':
    i =i +5;
    break;
  case '6':
    i =i +6;
    break;
  case '7':
    i =i +7;
    break;
  case '8':
    i =i +8;
    break;
  case '9':
    i =i +9;
    break;
  case '0':
    break;
      
  }
  return i;
}


/*******************************************************************************
****** Print any Integer Number with any digit to CHARs on LCD function ********
*******************************************************************************/
/*      AS we get the result of any mathematical sign equation in the for of integer
  we need to print characters not an integer
  this function have 3 for loops
 1st loop ----> is for counting the number of digits in of the integer by using counter variable
 2nd loop ----> is for taking the reminder of the digit and put it at the last index that fits the integer digit until the last reminder that will be the first digit in the integer is saved in index 0
 3nd loop ----> is for printing the ascii code of each number + 48 and 48 is the offset from the ascii table to take the corresponding digit in character  */
void PrintIntToLCD(int answer)
{
  int array[20];
  int z = answer;
  int counter = 0;
  if(answer == 0)
  {
    LCD_Char('0');
  }
  for(int i = 0 ; i < 20 ; i++)
  {
    if(z == 0 )
    {
      i =100;
    }
    else{
    z  = z /10;
    counter++;
    }
  }
  z = answer;
  for(int i = counter - 1 ; i >= 0 ;i--)
  {
    array[i] = z % 10;
    z = z /10;
  }
  
  for(int i = 0 ; i < counter ; i++)
  {
    if(i>counter)
    {
      break;
    }
    LCD_Char(array[i]+48);
  }
  
}
