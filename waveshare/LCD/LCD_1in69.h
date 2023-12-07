/*****************************************************************************
* | File        :   LCD_1in69.h
* | Author      :   Waveshare team
* | Function    :   Hardware underlying interface
* | Info        :   Used to shield the underlying layers of each master and enhance portability
*----------------
* | This version:   V1.0
* | Date        :   2023-03-09
* | Info        :   Basic version
*
******************************************************************************/
#ifndef __LCD_1IN69_H
#define __LCD_1IN69_H   
    
#include "DEV_Config.h"
#include <stdint.h>

#include <stdlib.h>     //itoa()
#include <stdio.h>

#define LCD_1IN69_HEIGHT 280
#define LCD_1IN69_WIDTH 240

#define HORIZONTAL 0
#define VERTICAL   1

#define LCD_1IN69_SetBacklight(Value) DEV_SetBacklight(Value) 

#define LCD_1IN69_CS_0   LCD_CS_0     
#define LCD_1IN69_CS_1   LCD_CS_1    
                     
#define LCD_1IN69_RST_0  LCD_RST_0   
#define LCD_1IN69_RST_1  LCD_RST_1   
                      
#define LCD_1IN69_DC_0   LCD_DC_0    
#define LCD_1IN69_DC_1   LCD_DC_1    
                      
#define LCD_1IN69_BL_0   LCD_BL_0    
#define LCD_1IN69_BL_1   LCD_BL_1    

typedef struct{
    UWORD WIDTH;
    UWORD HEIGHT;
    UBYTE SCAN_DIR;
}LCD_1IN69_ATTRIBUTES;
extern LCD_1IN69_ATTRIBUTES LCD_1IN69;

/********************************************************************************
function:   Macro definition variable name
********************************************************************************/
void LCD_1IN69_Init(UBYTE Scan_dir);
void LCD_1IN69_Clear(UWORD Color);
void LCD_1IN69_Display(UWORD *Image);
void LCD_1IN69_DisplayWindows(UWORD Xstart, UWORD Ystart, UWORD Xend, UWORD Yend, UWORD *Image);
void LCD_1IN69_DrawPoint(UWORD X, UWORD Y, UWORD Color);
void Handler_1IN69_LCD(int signo);
#endif
