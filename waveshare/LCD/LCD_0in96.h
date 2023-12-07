/*****************************************************************************
* | File      	:	LCD_0in96.h
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
#ifndef __LCD_DRIVER_H
#define __LCD_DRIVER_H

#include "DEV_Config.h"

#define LCD_0IN96_WIDTH   160 //LCD width
#define LCD_0IN96_HEIGHT  80 //LCD height

#define LCD_0IN96_SetBacklight(Value) DEV_SetBacklight(Value) 

#define LCD_0IN96_CS_0	LCD_CS_0	 
#define LCD_0IN96_CS_1	LCD_CS_1	
     
#define LCD_0IN96_RST_0	LCD_RST_0	
#define LCD_0IN96_RST_1	LCD_RST_1	
     
#define LCD_0IN96_DC_0	LCD_DC_0	
#define LCD_0IN96_DC_1	LCD_DC_1	
      
#define LCD_0IN96_BL_0	LCD_BL_0	
#define LCD_0IN96_BL_1	LCD_BL_1	

void LCD_0IN96_Init(void);
void LCD_0IN96_Clear(UWORD Color);
void LCD_0IN96_Display(UWORD *Image);
void LCD_0IN96_DisplayWindows(UWORD Xstart, UWORD Ystart, UWORD Xend, UWORD Yend, UWORD *Image);
void LCD_0IN96_DrawPaint(UWORD x, UWORD y, UWORD Color);
void Handler_0IN96_LCD(int signo);

void LCD_0IN96_SetBackLight(UWORD Value);
void LCD_0IN96_ClearWindow(UWORD Xstart, UWORD Ystart, UWORD Xend, UWORD Yend,UWORD color);
void LCD_0IN96_SetCursor(UWORD X, UWORD Y);
void LCD_0IN96_SetWindow(UWORD Xstart, UWORD Ystart, UWORD Xend, UWORD  Yend);

#endif
