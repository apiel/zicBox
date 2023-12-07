/*****************************************************************************
* | File      	:	LCD_2IN_Driver.c
* | Author      :   Waveshare team
* | Function    :   LCD driver
* | Info        :
*----------------
* |	This version:   V1.0
* | Date        :   2018-12-18
* | Info        :   
#
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documnetation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to  whom the Software is
# furished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice shall be included in
# all copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS OR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
# THE SOFTWARE.
#
******************************************************************************/
#include "LCD_2inch.h"
#include <string.h>
#include <stdlib.h>		//itoa()
/*******************************************************************************
function:
	Hardware reset
*******************************************************************************/
static void LCD_2IN_Reset(void)
{
	DEV_Digital_Write(LCD_CS, 1);
	DEV_Delay_ms(100);
	DEV_Digital_Write(LCD_RST, 0);
	DEV_Delay_ms(100);
	DEV_Digital_Write(LCD_RST, 1);
	DEV_Delay_ms(100);
}

/*******************************************************************************
function:
		Write data and commands
*******************************************************************************/
static void LCD_2IN_Write_Command(UBYTE data)	 
{	
	DEV_Digital_Write(LCD_CS, 0);
	DEV_Digital_Write(LCD_DC, 0);
	DEV_SPI_WriteByte(data);
}

static void LCD_2IN_WriteData_Byte(UBYTE data) 
{	
	DEV_Digital_Write(LCD_CS, 0);
	DEV_Digital_Write(LCD_DC, 1);
	DEV_SPI_WriteByte(data);  
	DEV_Digital_Write(LCD_CS,1);
}  

void LCD_2IN_WriteData_Word(UWORD data)
{
	DEV_Digital_Write(LCD_CS, 0);
	DEV_Digital_Write(LCD_DC, 1);
	DEV_SPI_WriteByte((data>>8) & 0xff);
	DEV_SPI_WriteByte(data);
	DEV_Digital_Write(LCD_CS, 1);
}	  


/******************************************************************************
function:	
		Common register initialization
******************************************************************************/
void LCD_2IN_Init(void)
{
	LCD_2IN_Reset();

	LCD_2IN_Write_Command(0x36);
	LCD_2IN_WriteData_Byte(0x00); 

	LCD_2IN_Write_Command(0x3A); 
	LCD_2IN_WriteData_Byte(0x05);

	LCD_2IN_Write_Command(0x21); 

	LCD_2IN_Write_Command(0x2A);
	LCD_2IN_WriteData_Byte(0x00);
	LCD_2IN_WriteData_Byte(0x00);
	LCD_2IN_WriteData_Byte(0x01);
	LCD_2IN_WriteData_Byte(0x3F);

	LCD_2IN_Write_Command(0x2B);
	LCD_2IN_WriteData_Byte(0x00);
	LCD_2IN_WriteData_Byte(0x00);
	LCD_2IN_WriteData_Byte(0x00);
	LCD_2IN_WriteData_Byte(0xEF);

	LCD_2IN_Write_Command(0xB2);
	LCD_2IN_WriteData_Byte(0x0C);
	LCD_2IN_WriteData_Byte(0x0C);
	LCD_2IN_WriteData_Byte(0x00);
	LCD_2IN_WriteData_Byte(0x33);
	LCD_2IN_WriteData_Byte(0x33);

	LCD_2IN_Write_Command(0xB7);
	LCD_2IN_WriteData_Byte(0x35); 

	LCD_2IN_Write_Command(0xBB);
	LCD_2IN_WriteData_Byte(0x1F);

	LCD_2IN_Write_Command(0xC0);
	LCD_2IN_WriteData_Byte(0x2C);

	LCD_2IN_Write_Command(0xC2);
	LCD_2IN_WriteData_Byte(0x01);

	LCD_2IN_Write_Command(0xC3);
	LCD_2IN_WriteData_Byte(0x12);   

	LCD_2IN_Write_Command(0xC4);
	LCD_2IN_WriteData_Byte(0x20);

	LCD_2IN_Write_Command(0xC6);
	LCD_2IN_WriteData_Byte(0x0F); 

	LCD_2IN_Write_Command(0xD0);
	LCD_2IN_WriteData_Byte(0xA4);
	LCD_2IN_WriteData_Byte(0xA1);

	LCD_2IN_Write_Command(0xE0);
	LCD_2IN_WriteData_Byte(0xD0);
	LCD_2IN_WriteData_Byte(0x08);
	LCD_2IN_WriteData_Byte(0x11);
	LCD_2IN_WriteData_Byte(0x08);
	LCD_2IN_WriteData_Byte(0x0C);
	LCD_2IN_WriteData_Byte(0x15);
	LCD_2IN_WriteData_Byte(0x39);
	LCD_2IN_WriteData_Byte(0x33);
	LCD_2IN_WriteData_Byte(0x50);
	LCD_2IN_WriteData_Byte(0x36);
	LCD_2IN_WriteData_Byte(0x13);
	LCD_2IN_WriteData_Byte(0x14);
	LCD_2IN_WriteData_Byte(0x29);
	LCD_2IN_WriteData_Byte(0x2D);

	LCD_2IN_Write_Command(0xE1);
	LCD_2IN_WriteData_Byte(0xD0);
	LCD_2IN_WriteData_Byte(0x08);
	LCD_2IN_WriteData_Byte(0x10);
	LCD_2IN_WriteData_Byte(0x08);
	LCD_2IN_WriteData_Byte(0x06);
	LCD_2IN_WriteData_Byte(0x06);
	LCD_2IN_WriteData_Byte(0x39);
	LCD_2IN_WriteData_Byte(0x44);
	LCD_2IN_WriteData_Byte(0x51);
	LCD_2IN_WriteData_Byte(0x0B);
	LCD_2IN_WriteData_Byte(0x16);
	LCD_2IN_WriteData_Byte(0x14);
	LCD_2IN_WriteData_Byte(0x2F);
	LCD_2IN_WriteData_Byte(0x31);
	LCD_2IN_Write_Command(0x21);

	LCD_2IN_Write_Command(0x11);

	LCD_2IN_Write_Command(0x29);
}

/******************************************************************************
function:	Set the cursor position
parameter	:
	  Xstart: 	Start UWORD x coordinate
	  Ystart:	Start UWORD y coordinate
	  Xend  :	End UWORD coordinates
	  Yend  :	End UWORD coordinatesen
******************************************************************************/
void LCD_2IN_SetWindow(UWORD Xstart, UWORD Ystart, UWORD Xend, UWORD  Yend)
{ 
	LCD_2IN_Write_Command(0x2a);
	LCD_2IN_WriteData_Byte(Xstart >>8);
	LCD_2IN_WriteData_Byte(Xstart & 0xff);
	LCD_2IN_WriteData_Byte((Xend - 1) >> 8);
	LCD_2IN_WriteData_Byte((Xend - 1) & 0xff);

	LCD_2IN_Write_Command(0x2b);
	LCD_2IN_WriteData_Byte(Ystart >>8);
	LCD_2IN_WriteData_Byte(Ystart & 0xff);
	LCD_2IN_WriteData_Byte((Yend - 1) >> 8);
	LCD_2IN_WriteData_Byte((Yend - 1) & 0xff);

	LCD_2IN_Write_Command(0x2C);
}

/******************************************************************************
function:	Settings window
parameter	:
	  Xstart: 	Start UWORD x coordinate
	  Ystart:	Start UWORD y coordinate

******************************************************************************/
void LCD_2IN_SetCursor(UWORD X, UWORD Y)
{ 
	LCD_2IN_Write_Command(0x2a);
	LCD_2IN_WriteData_Byte(X >> 8);
	LCD_2IN_WriteData_Byte(X);
	LCD_2IN_WriteData_Byte(X >> 8);
	LCD_2IN_WriteData_Byte(X);

	LCD_2IN_Write_Command(0x2b);
	LCD_2IN_WriteData_Byte(Y >> 8);
	LCD_2IN_WriteData_Byte(Y);
	LCD_2IN_WriteData_Byte(Y >> 8);
	LCD_2IN_WriteData_Byte(Y);

	LCD_2IN_Write_Command(0x2C);
}

/******************************************************************************
function:	Clear screen function, refresh the screen to a certain color
parameter	:
	  Color :		The color you want to clear all the screen
******************************************************************************/
void LCD_2IN_Clear(UWORD Color)
{
	UWORD i;
	UWORD image[LCD_2IN_WIDTH];
	for(i=0;i<LCD_2IN_WIDTH;i++){
		image[i] = Color>>8 | (Color&0xff)<<8;
	}
	UBYTE *p = (UBYTE *)(image);
	LCD_2IN_SetWindow(0, 0, LCD_2IN_WIDTH, LCD_2IN_HEIGHT);
	DEV_Digital_Write(LCD_DC, 1);
	for(i = 0; i < LCD_2IN_HEIGHT; i++){
		DEV_SPI_Write_nByte(p,LCD_2IN_WIDTH*2);
	}
}

/******************************************************************************
function:	Refresh a certain area to the same color
parameter	:
	  Xstart: Start UWORD x coordinate
	  Ystart:	Start UWORD y coordinate
	  Xend  :	End UWORD coordinates
	  Yend  :	End UWORD coordinates
	  color :	Set the color
******************************************************************************/
void LCD_2IN_ClearWindow(UWORD Xstart, UWORD Ystart, UWORD Xend, UWORD Yend,UWORD color)
{          
	UWORD i,j; 
	LCD_2IN_SetWindow(Xstart, Ystart, Xend-1,Yend-1);
	for(i = Ystart; i <= Yend-1; i++){
		for(j = Xstart; j <= Xend-1; j++){
			LCD_2IN_WriteData_Word(color);
		}
	}
}

/******************************************************************************
function: Show a picture
parameter	:
		image: Picture buffer
******************************************************************************/
void LCD_2IN_Display(UBYTE *image)
{
	UWORD i;
	LCD_2IN_SetWindow(0, 0, LCD_2IN_WIDTH, LCD_2IN_HEIGHT);
	DEV_Digital_Write(LCD_DC, 1);
	for(i = 0; i < LCD_2IN_HEIGHT; i++){
		DEV_SPI_Write_nByte((UBYTE *)image+LCD_2IN_WIDTH*2*i,LCD_2IN_WIDTH*2);
	}
}

/******************************************************************************
function: Draw a point
parameter	:
	    X	: 	Set the X coordinate
	    Y	:	Set the Y coordinate
	  Color :	Set the color
******************************************************************************/
void LCD_2IN_DrawPaint(UWORD x, UWORD y, UWORD Color)
{
	LCD_2IN_SetCursor(x, y);
	LCD_2IN_WriteData_Word(Color); 	    
}

void  Handler_2IN_LCD(int signo)
{
    //System Exit
    printf("\r\nHandler:Program stop\r\n");     
    DEV_ModuleExit();
	exit(0);
}