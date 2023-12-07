#ifndef __LCD_1IN8_DRIVER_H
#define __LCD_1IN8_DRIVER_H

#include <stdint.h>
#include "DEV_Config.h"
#include "../Fonts/fonts.h"

#define	COLOR				uint16_t		//The variable type of the color (unsigned short) 
#define	POINT				uint16_t		//The type of coordinate (unsigned short) 
#define	LENGTH				uint16_t		//The type of coordinate (unsigned short) 

/********************************************************************************
function:
		Define the full screen height length of the display
********************************************************************************/


#define LCD_1IN8_WIDTH  160  //LCD width
#define LCD_1IN8_HEIGHT  128 //LCD height
#define LCD_1IN8_X	 2
#define LCD_1IN8_Y	 1

#define LCD_1IN8_SetBacklight(Value) DEV_SetBacklight(Value) 

#define LCD_1IN8_CS_0	LCD_CS_0	 
#define LCD_1IN8_CS_1	LCD_CS_1	
	                  
#define LCD_1IN8_RST_0	LCD_RST_0	
#define LCD_1IN8_RST_1	LCD_RST_1	
	                 
#define LCD_1IN8_DC_0	LCD_DC_0	
#define LCD_1IN8_DC_1	LCD_DC_1	
	                  
#define LCD_1IN8_BL_0	LCD_BL_0	
#define LCD_1IN8_BL_1	LCD_BL_1	


/********************************************************************************
function:
			scanning method
********************************************************************************/
typedef enum {
    L2R_U2D  = 0,				//The display interface is displayed , left to right, up to down
    L2R_D2U  ,
    R2L_U2D  ,
    R2L_D2U  ,

    U2D_L2R  ,
    U2D_R2L  ,
    D2U_L2R  ,
    D2U_R2L  ,
} LCD_1IN8_SCAN_DIR;
#define SCAN_DIR_DFT  U2D_R2L  	//Default scan direction = L2R_U2D

/********************************************************************************
function:
	Defines the total number of rows in the display area
********************************************************************************/

typedef struct {
    LENGTH 				LCD_1IN8_Dis_Column;	//COLUMN
    LENGTH 				LCD_1IN8_Dis_Page;		//PAGE
    LCD_1IN8_SCAN_DIR 	LCD_1IN8_Scan_Dir;
    POINT 				LCD_1IN8_X_Adjust;		//LCD x actual display position calibration
    POINT 				LCD_1IN8_Y_Adjust;		//LCD y actual display position calibration
} LCD_1IN8_DIS;


void LCD_1IN8_Init( LCD_1IN8_SCAN_DIR Lcd_ScanDir );
void LCD_1IN8_Clear(COLOR  Color);
void LCD_1IN8_Display(UWORD *Image);
void LCD_1IN8_DisplayWindows(UWORD Xstart, UWORD Ystart, UWORD Xend, UWORD Yend, UWORD *Image);
void LCD_1IN8_SetPointlColor ( POINT Xpoint, POINT Ypoint, COLOR Color );
void Handler_1IN8_LCD(int signo);


void LCD_1IN8_SetGramScanWay(LCD_1IN8_SCAN_DIR Scan_dir);
void LCD_1IN8_SetWindows( POINT Xstart, POINT Ystart, POINT Xend, POINT Yend );
void LCD_1IN8_SetCursor ( POINT Xpoint, POINT Ypoint );
void LCD_1IN8_SetColor( COLOR Color ,POINT Xpoint, POINT Ypoint);
void LCD_1IN8_SetArealColor ( POINT Xstart, POINT Ystart, POINT Xend, POINT Yend,COLOR  Color);


#endif
