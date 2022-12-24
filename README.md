# Embedded-Application-3-Mode-
By using TM4C123GH6PM based on ARM Architecture, implementing a Hardware Application have 3 modes by using TIMER0, Systick Timer, Interrupt for PORTA and PORTF, GPIO, LCD, KEYPAD, Breadboard, wires and switches. 
Implemented Drivers and files: Keypad Driver- LCD DRIVER – GPIO DRIVER for each PORT – bitwise_operation.h file. 
Welcome screen for choosing 1- calc 2- Timer 3- Stopwatch 
  1st mode is Calculator {A  ‘+’ , B  ’-’, C   ‘/’ , D  ‘=’, * ’X’ }. The user enters the equation then pressing ‘=’ so the answer will be appeared. Automatically the calculator will restart to use it again after 5 sec presenting the last answer.  
2nd mode is Timer by using TIMER0. The user enters the targeted time to be count in 00:00 form. Then press PA3 button that gives an interrupt that start counting down till 00:00. The user can pause by pressing PF4 that gives an interrupt. 
3rd mode is Stopwatch by using Systick-timer. The must press PA3 button that gives an interrupt to start counting-up till 59:59. User can pause by pressing PF4 gives an interrupt to pause the system or can reset by pressing PF0 that gives interrupt while counting-up then it will reset then start automatically. 
And can pause then reset so the screen will be 00:00 waiting for start button. 
Note:  
From any mode you can interrupt to return to welcome screen by pressing PA2 that gives an interrupt.

Functions Flow 
Main () 
First we set INTERRUPT flag to no interrupt happened then switch case on STATE for 4 cases  
1st page is welcome page to print “welcome 1-calc 2-Timer 3-SW” then by default STATE is set to WelcomeHome. Then according to user input in WelcomeHome we change the STATE to the corresponding state then return to the main to do the needed function. 
2nd page is CalcPage to use a calculator, 3rd page is Timer which used to count-down from input by the user till 00:00 and the last page is Stopwatch which count from 00:00 till 59:59 


ISRs Functions

ISR_PortaHandler () 
We implement PA2 and PA3 as an interrupt. So, we need to check from MIS_R which pin caused the interrupt to set it’s corresponding pin in ICR_R Register that clears the pin in MIS_R. 
Note: we can’t clear MIS_R directly as it is a read only register. So, from datasheet we know that ICR_R is the interrupt clear register for MIS_R. 
We use PA2 for a global interrupt from any state to return to home to choose a new mode. So we need after knowing that it’s PA2 not any pin. To stop any variables or timers that counts from the current state and disable timers. Then set the corresponding pin in ICR_R to clear MIS_R. 
We use PA3 for a starting the timer or stopwatch by setting the pause variable to 0. So, no pause it means start counting. Then set the corresponding pin in ICR_R to clear MIS_R. 

ISR_PortfHandler () 
We use PF0 for resetting the stopwatch to 00:00 but we have two cases: 
1st case: while counting we reset so the stopwatch will reset to 00:00 and start counting automatically 
2nd case: while counting we pause then reset so the stopwatch will reset to 00:00 and wait to start button. 
Then set the corresponding pin in ICR_R to clear MIS_R. 

ISR_Timer0Handler () 
We use timer0 handler to only give a flag that a tick is happened for counting 1 sec in Timer Mode. 
Then set the corresponding pin in ICR_R to clear MIS_R. 

SysHandler ()  
We use Systick timer to only give a flag that a tick is happened for counting 1 sec in Stopwatch Mode. 
No need to clear interrupt flag as it cleared automatically by Hardware. 

Welcome_Home () 
In this function we polling on keypad until giving a right input as 1 or 2 or 3. Otherwise will neglect. 
Note: keypad_read() returning by default ‘!’ character to prevent stuck on polling in keypad input until the user press any number on keypad. So, the targeted number is returned. 
While using stopwatch you can pause and return to welcomeHome. Then when you re-enter the stopwatch you have the option to start from the last counted time. Or reset and start from 00:00 

Calc_Page () 
 First we load a welcome menu for only one time by using a stop_welcome variable as a lock to prevent re-entering this if condition. 
We think about Calculator as : hold_in_x “sign” hold_in_y = hold_in_answer 
so we need to enter x variable then pressing any kind of sign then we will change the state from x to y. to start taking the variable in y until pressing ‘=’ so we will be in case Z 
we do the same method here. As getting to this case is meaning to start calculating the targeted sign. So we use a switch case to apply the equation and store it in take_in_answer. 
Then sending take_in_asnwer that is integer to PrintIntLCD(). This function is responsible for changing the integer to string to print it on LCD. 

Timer_Page () 	 
First we initialize PD3 to use it when timer0 timeout. This pin toggle a led for 3 sec 
Then we print a “Loading. . .” then printing “00:00” to start taking an input from the user. 
This while loop is responsible for taking 4 input from the user as “X1_T0   X0_T0      :    Y1_T0   Y0_T0”. 
CharToNumber() is to change the character to integer to store it as an integer in each corresponding variable to start increment or decrement the targeted variable 
PrintNow () is for printing an integer by changing it to character and print it. 
Then start using the timer by pressing start PA3 button or pausing for any time 
You  can press PA2 for returning to WelcomeHome. 

SW_Page () 
We print a “Loading. . .” then printing “00:00” and waiting to start button 
Our stopwatch designed as “X1    X0    :    Y1    Y0”. 
CharToNumber() is to change the character to integer to store it as an integer in each corresponding variable to start increment or decrement the targeted variable 
PrintNow () is for printing an integer by changing it to character and print it. 
Then start using the timer by pressing start PA3 button  
Or pause by PF4  
OR reset by PF0  
All work by interrupt 
You  can press PA2 for returning to WelcomeHome. 


Functions Helped Us 
Change_state () 
This fucntion is for selecting from the welcome screen a state to start using this choosing state by taking input at welcome function of 1- calc 2- Timer 3- Stopwatch 

PrintIntToLCD () 
AS we get the result of any mathematical sign equation in the for of integer we need to print characters not an integer  
This function have 3 for loops 
 1st loop ----> is for counting the number of digits in of the integer by using counter variable 
 2nd loop ----> is for taking the reminder of the digit and put it at the last index that fits the integer digit until the last reminder that will be the first digit in the integer is saved in index 0 
 3nd loop ----> is for printing the ascii code of each number + 48 and 48 is the offset from the ascii table to take the corresponding digit in character 
