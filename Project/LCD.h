#ifndef __LCD_H__
#define __LCD_H__

// Assuming on same RS,EN,Data Lines on same port
// This FUnctions use whole port pins
/*...........................COnfigs...............................................*/

#define LCD_Port GPIO_PORTB_DATA_R			/* Define LCD data port */
#define LCD_RS_PIN P0				/* Define Register Select pin */
#define LCD_EN_PIN P2 				/* Define Enable signal pin */

#define uint8 unsigned char 

/*.........................Ports_by_ADDRESSESS.............. ................*/
#define PORTA_Direct GPIO_PORTA_DATA_R
#define PORTB_Direct GPIO_PORTB_DATA_R
#define PORTC_Direct GPIO_PORTC_DATA_R
#define PORTD_Direct GPIO_PORTD_DATA_R
#define PORTE_Direct GPIO_PORTE_DATA_R
#define PORTF_Direct GPIO_PORTF_DATA_R

#define ClearScreen 0x01
#define ReturnHome 0x02
#define SecondLine 0xC0
#define FirstLine 0x80
#define ShiftCursorLeft 0x10
#define ShiftCursorRight 0x14
#define BlinkingStop 0x0C
#define BlinkingStart 0x0F

/*.................................FUncs.............*/
void LCD_Command( uint8 cmnd );


void LCD_Init (void);
void LCD_Char(uint8 data);

void LCD_String(uint8 str[]);

#endif