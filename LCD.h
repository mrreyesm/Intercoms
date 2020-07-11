#ifndef LCD_H
#define	LCD_H

#ifdef	__cplusplus
extern "C" {
#endif
#ifdef	__cplusplus
}
#endif

/* C access to assembler insructions */	
#define	CLRWDT()	__asm(" clrwdt")
#define	ClrWdt()	__asm(" clrwdt")

/****************************************************************/
/* Built-in delay routine					*/
/****************************************************************/
#pragma intrinsic(_delay)
extern __nonreentrant void _delay(unsigned long);
#pragma intrinsic(_delaywdt)
extern __nonreentrant void _delaywdt(unsigned long);
#pragma intrinsic(_delay3)
extern __nonreentrant void _delay3(unsigned char);
// NOTE: To use the macros below, YOU must have previously defined _XTAL_FREQ
#define __delay_us(x) _delay((unsigned long)((x)*(_XTAL_FREQ/4000000.0)))
#define __delay_ms(x) _delay((unsigned long)((x)*(_XTAL_FREQ/4000.0)))
#define __delaywdt_us(x) _delaywdt((unsigned long)((x)*(_XTAL_FREQ/4000000.0)))
#define __delaywdt_ms(x) _delaywdt((unsigned long)((x)*(_XTAL_FREQ/4000.0)))

//  NECESSARY LIBS TO USE LCD
#include <stdlib.h>  
#include <stdio.h>
#include <string.h>
#include <stddef.h>
#include <math.h>
#include <string.h>
#include <ctype.h>

void LCDport(char a);
void LCDcmd(char a);
void LCDsc(unsigned char a, unsigned char b);
void LCDclr(void);
void LCDin(void); 
void LCDputc(unsigned char a);
void LCDputs(char *a);
void LCDsr(void);
void LCDsl(void);
void LCDcoff(void);

// Command set for Hitachi 44780U LCD display controller
#define LCD_CLEAR           0x01
#define LCD_HOME            0x02
#define LCD_CURSOR_BACK     0x10
#define LCD_CURSOR_FWD      0x14
#define LCD_PAN_LEFT        0x18
#define LCD_PAN_RIGHT       0x1C
#define LCD_CURSOR_OFF      0x0C
#define LCD_CURSOR_ON       0x0E
#define LCD_CURSOR_BLINK    0x0F
#define LCD_CURSOR_LINE2    0xC0

//  PIN DISTRIBUTION OF LCD
#define RS LATAbits.LATA0
#define EN LATAbits.LATA1
#define D4 LATAbits.LATA2
#define D5 LATAbits.LATA3
#define D6 LATAbits.LATA4
#define D7 LATAbits.LATA5

void LCDport(char a)
{
	if(a & 1)   D4 = 1;
	else		D4 = 0;
	if(a & 2)	D5 = 1;
	else		D5 = 0;
	if(a & 4)	D6 = 1;
	else		D6 = 0;
	if(a & 8)	D7 = 1;
	else		D7 = 0;
}

void LCDcmd(char a)
{
	RS = 0;             // => RS = 0
	LCDport(a);
	EN  = 1;             // => E = 1
        __delay_ms(4);
        EN  = 0;             // => E = 0
}

void LCDsc(unsigned char a, unsigned char b) {
	unsigned char temp,z,y;
	if(a == 1) {
        temp = (unsigned char)(0x80 + b - 1);
		z = (unsigned char)(temp>>4);
		y = (unsigned char)(temp & 0x0F);
		LCDcmd(z);
		LCDcmd(y);
	} else if(a == 2) {
		temp = (unsigned char)(0xC0 + b - 1);
		z = (unsigned char)(temp>>4);
		y = (unsigned char)(temp & 0x0F);
		LCDcmd(z);
		LCDcmd(y);
	}
}

 void LCDclr(void) {
	LCDcmd(0);
	LCDcmd(1);
    LCDsc(1,1);
}

 void LCDin(void) 
{
  LCDport(0x00);
  __delay_ms(20);
  LCDcmd(0x03);
  __delay_ms(5);
  LCDcmd(0x03);
  __delay_ms(11);
  LCDcmd(0x03);
  LCDcmd(0x02);
  LCDcmd(0x02);
  LCDcmd(0x08);
  LCDcmd(0x00);
  LCDcmd(0x0F);
  LCDcmd(0x00);
  LCDcmd(0x06);
}

void LCDputc(unsigned char a)
{
   char temp,y;
   temp = (unsigned char)(a&0x0F);
   y = (unsigned char)(a&0xF0);
   RS = 1;             // => RS = 1
   LCDport((unsigned char)(y>>4));             //Data transfer
   EN = 1;
   __delay_us(40);
   EN = 0;
   LCDport(temp);
   EN = 1;
   __delay_us(40);
   EN = 0;
}

void LCDputs(char *a)
{ 
 	int i;
	for(i=0;a[i]!='\0';i++)
	   LCDputc(a[i]);
}

void LCDsr(void) //Shift Right
{
	LCDcmd(0x01);
	LCDcmd(0x0C);
}

void LCDsl() //Shift Left
{
	LCDcmd(0x01);
	LCDcmd(0x08);
}

void LCDcoff() //Cursor OFF
{
    LCDcmd(0x00);
    LCDcmd(0x0C);  
}

void LCDcon() //Cursor OFF
{
    LCDcmd(0x00);
    LCDcmd(0x0F);  
}

#endif	
