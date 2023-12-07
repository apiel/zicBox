/*****************************************************************************
* | File      	:   LCD_1in54.c
* | Author      :   Waveshare team
* | Function    :   Hardware underlying interface
* | Info        :
*                Used to shield the underlying layers of each master 
*                and enhance portability
*----------------
* |	This version:   V1.0
* | Date        :   2020-05-20
* | Info        :   Basic version
*
******************************************************************************/
#ifndef __LCD_1IN14_H
#define __LCD_1IN14_H	
	
#include "DEV_Config.h"
#include <stdint.h>

#include <stdlib.h>		//itoa()
#include <stdio.h>


#define LCD_1IN14_HEIGHT 240
#define LCD_1IN14_WIDTH 135


#define HORIZONTAL 0
#define VERTICAL   1

#define LCD_1IN14_SetBacklight(Value) DEV_SetBacklight(Value) 

#define LCD_1IN14_CS_0	LCD_CS_0	 
#define LCD_1IN14_CS_1	LCD_CS_1	
	                    
#define LCD_1IN14_RST_0	LCD_RST_0	
#define LCD_1IN14_RST_1	LCD_RST_1	
	                    
#define LCD_1IN14_DC_0	LCD_DC_0	
#define LCD_1IN14_DC_1	LCD_DC_1	
	                    
#define LCD_1IN14_BL_0	LCD_BL_0	
#define LCD_1IN14_BL_1	LCD_BL_1	

	
typedef struct{
	UWORD WIDTH;
	UWORD HEIGHT;
	UBYTE SCAN_DIR;
}LCD_1IN14_ATTRIBUTES;
extern LCD_1IN14_ATTRIBUTES LCD_1IN14;

/********************************************************************************
function:	
			Macro definition variable name
********************************************************************************/
void LCD_1IN14_Init(UBYTE Scan_dir);
void LCD_1IN14_Clear(UWORD Color);
void LCD_1IN14_Display(UWORD *Image);
void LCD_1IN14_DisplayWindows(UWORD Xstart, UWORD Ystart, UWORD Xend, UWORD Yend, UWORD *Image);
void LCD_1IN14_DisplayPoint(UWORD X, UWORD Y, UWORD Color);

void Handler_1IN14_LCD(int signo);
#endif
