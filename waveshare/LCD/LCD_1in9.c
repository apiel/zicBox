/*****************************************************************************
* | File        :   LCD_1in9.c
* | Author      :   Waveshare team
* | Function    :   Hardware underlying interface
* | Info        :   Used to shield the underlying layers of each master and enhance portability
*----------------
* | This version:   V1.0
* | Date        :   2022-11-18
* | Info        :   Basic version
 *
 ******************************************************************************/
#include "LCD_1in9.h"
#include "DEV_Config.h"

#include <stdlib.h> //itoa()
#include <stdio.h>

LCD_1IN9_ATTRIBUTES LCD_1IN9;

/******************************************************************************
function :  Hardware reset
parameter:
******************************************************************************/
static void LCD_1IN9_Reset(void)
{
    LCD_1IN9_RST_1;
    DEV_Delay_ms(100);
    LCD_1IN9_RST_0;
    DEV_Delay_ms(100);
    LCD_1IN9_RST_1;
    DEV_Delay_ms(100);
}

/******************************************************************************
function :  send command
parameter:
     Reg : Command register
******************************************************************************/
static void LCD_1IN9_SendCommand(UBYTE Reg)
{
    LCD_1IN9_DC_0;
    // LCD_1IN9_CS_0;
    DEV_SPI_WriteByte(Reg);
    //LCD_1IN9_CS_1;
}

/******************************************************************************
function :  send data
parameter:
    Data : Write data
******************************************************************************/
static void LCD_1IN9_SendData_8Bit(UBYTE Data)
{
    LCD_1IN9_DC_1;
    //LCD_1IN9_CS_0;
    DEV_SPI_WriteByte(Data);
    //LCD_1IN9_CS_1;
}

/******************************************************************************
function :  send data
parameter:
    Data : Write data
******************************************************************************/
static void LCD_1IN9_SendData_16Bit(UWORD Data)
{
    LCD_1IN9_DC_1;
    // LCD_1IN9_CS_0;
    DEV_SPI_WriteByte((Data >> 8) & 0xFF);
    DEV_SPI_WriteByte(Data & 0xFF);
    // LCD_1IN9_CS_1;
}

/******************************************************************************
function :  Initialize the lcd register
parameter:
******************************************************************************/
static void LCD_1IN9_InitReg(void)
{
    LCD_1IN9_SendCommand(0x36);
    LCD_1IN9_SendData_8Bit(0x00);

    LCD_1IN9_SendCommand(0x3A);
    LCD_1IN9_SendData_8Bit(0x55);

    LCD_1IN9_SendCommand(0xB2);
    LCD_1IN9_SendData_8Bit(0x0C);
    LCD_1IN9_SendData_8Bit(0x0C);
    LCD_1IN9_SendData_8Bit(0x00);
    LCD_1IN9_SendData_8Bit(0x33);
    LCD_1IN9_SendData_8Bit(0x33);

    LCD_1IN9_SendCommand(0xB7);
    LCD_1IN9_SendData_8Bit(0x35);

    LCD_1IN9_SendCommand(0xBB);
    LCD_1IN9_SendData_8Bit(0x13);

    LCD_1IN9_SendCommand(0xC0);
    LCD_1IN9_SendData_8Bit(0x2C);

    LCD_1IN9_SendCommand(0xC2);
    LCD_1IN9_SendData_8Bit(0x01);

    LCD_1IN9_SendCommand(0xC3);
    LCD_1IN9_SendData_8Bit(0x0B);

    LCD_1IN9_SendCommand(0xC4);
    LCD_1IN9_SendData_8Bit(0x20);

    LCD_1IN9_SendCommand(0xC6);
    LCD_1IN9_SendData_8Bit(0x0F);

    LCD_1IN9_SendCommand(0xD0);
    LCD_1IN9_SendData_8Bit(0xA4);
    LCD_1IN9_SendData_8Bit(0xA1);

    LCD_1IN9_SendCommand(0xE0);
    LCD_1IN9_SendData_8Bit(0x00);
    LCD_1IN9_SendData_8Bit(0x03);
    LCD_1IN9_SendData_8Bit(0x07);
    LCD_1IN9_SendData_8Bit(0x08);
    LCD_1IN9_SendData_8Bit(0x07);
    LCD_1IN9_SendData_8Bit(0x15);
    LCD_1IN9_SendData_8Bit(0x2A);
    LCD_1IN9_SendData_8Bit(0x44);
    LCD_1IN9_SendData_8Bit(0x42);
    LCD_1IN9_SendData_8Bit(0x0A);
    LCD_1IN9_SendData_8Bit(0x17);
    LCD_1IN9_SendData_8Bit(0x18);
    LCD_1IN9_SendData_8Bit(0x25);
    LCD_1IN9_SendData_8Bit(0x27);

    LCD_1IN9_SendCommand(0xE1);
    LCD_1IN9_SendData_8Bit(0x00);
    LCD_1IN9_SendData_8Bit(0x03);
    LCD_1IN9_SendData_8Bit(0x08);
    LCD_1IN9_SendData_8Bit(0x07);
    LCD_1IN9_SendData_8Bit(0x07);
    LCD_1IN9_SendData_8Bit(0x23);
    LCD_1IN9_SendData_8Bit(0x2A);
    LCD_1IN9_SendData_8Bit(0x43);
    LCD_1IN9_SendData_8Bit(0x42);
    LCD_1IN9_SendData_8Bit(0x09);
    LCD_1IN9_SendData_8Bit(0x18);
    LCD_1IN9_SendData_8Bit(0x17);
    LCD_1IN9_SendData_8Bit(0x25);
    LCD_1IN9_SendData_8Bit(0x27);

    LCD_1IN9_SendCommand(0x21);

    LCD_1IN9_SendCommand(0x11);
    DEV_Delay_ms(120);
    LCD_1IN9_SendCommand(0x29);
}

/********************************************************************************
function:   Set the resolution and scanning method of the screen
parameter:
        Scan_dir:   Scan direction
********************************************************************************/
static void LCD_1IN9_SetAttributes(UBYTE Scan_dir)
{
    // Get the screen scan direction
    LCD_1IN9.SCAN_DIR = Scan_dir;
    UBYTE MemoryAccessReg = 0x00;

    // Get GRAM and LCD width and height
    if (Scan_dir == HORIZONTAL) {
        LCD_1IN9.HEIGHT = LCD_1IN9_WIDTH;
        LCD_1IN9.WIDTH = LCD_1IN9_HEIGHT;
        MemoryAccessReg = 0X00;
    }
    else {
        LCD_1IN9.HEIGHT = LCD_1IN9_HEIGHT;
        LCD_1IN9.WIDTH = LCD_1IN9_WIDTH;      
        MemoryAccessReg = 0X70;
    }

    // Set the read / write scan direction of the frame memory
    LCD_1IN9_SendCommand(0x36); // MX, MY, RGB mode
    LCD_1IN9_SendData_8Bit(MemoryAccessReg); // 0x08 set RGB
}

/********************************************************************************
function :  Initialize the lcd
parameter:
********************************************************************************/
void LCD_1IN9_Init(UBYTE Scan_dir)
{
    // Hardware reset
    LCD_1IN9_Reset();

    // Set the resolution and scanning method of the screen
    LCD_1IN9_SetAttributes(Scan_dir);

    // Set the initialization register
    LCD_1IN9_InitReg();
}

/********************************************************************************
function:   Sets the start position and size of the display area
parameter:
        Xstart  :   X direction Start coordinates
        Ystart  :   Y direction Start coordinates
        Xend    :   X direction end coordinates
        Yend    :   Y direction end coordinates
********************************************************************************/
void LCD_1IN9_SetWindows(UWORD Xstart, UWORD Ystart, UWORD Xend, UWORD Yend)
{    
    if (LCD_1IN9.SCAN_DIR == HORIZONTAL) { 
        // set the X coordinates
        LCD_1IN9_SendCommand(0x2A);
        LCD_1IN9_SendData_8Bit((Xstart + 0x23) >> 8);
        LCD_1IN9_SendData_8Bit(Xstart + 0x23);
        LCD_1IN9_SendData_8Bit((Xend + 0x22) >> 8);
        LCD_1IN9_SendData_8Bit(Xend + 0x22);

        // set the Y coordinates
        LCD_1IN9_SendCommand(0x2B);
        LCD_1IN9_SendData_8Bit(Ystart >> 8);
        LCD_1IN9_SendData_8Bit(Ystart);
        LCD_1IN9_SendData_8Bit((Yend - 1) >> 8);
        LCD_1IN9_SendData_8Bit(Yend - 1);
    }
    else { 
        // set the X coordinates
        LCD_1IN9_SendCommand(0x2A);
        LCD_1IN9_SendData_8Bit((Xstart+1) >> 8);
        LCD_1IN9_SendData_8Bit(Xstart+1);
        LCD_1IN9_SendData_8Bit(Xend >> 8);
        LCD_1IN9_SendData_8Bit(Xend);

        // set the Y coordinates
        LCD_1IN9_SendCommand(0x2B);
        LCD_1IN9_SendData_8Bit((Ystart + 0x22) >> 8);
        LCD_1IN9_SendData_8Bit(Ystart + 0x22);
        LCD_1IN9_SendData_8Bit((Yend - 1 + 0x22) >> 8);
        LCD_1IN9_SendData_8Bit(Yend - 1 + 0x22);
    }
    LCD_1IN9_SendCommand(0x2C);   
}

/******************************************************************************
function :  Clear screen
parameter:
******************************************************************************/
void LCD_1IN9_Clear(UWORD Color)
{
    UWORD j;
    UWORD Image[LCD_1IN9_WIDTH*LCD_1IN9_HEIGHT];

    for (j=0; j<LCD_1IN9_HEIGHT*LCD_1IN9_WIDTH; j++) {
        Image[j] = Color;
    }

    LCD_1IN9_SetWindows(0, 0, LCD_1IN9.HEIGHT, LCD_1IN9.WIDTH);
    LCD_1IN9_DC_1;
    for (j=0; j<LCD_1IN9.HEIGHT; j++) {
        DEV_SPI_Write_nByte((uint8_t *)&Image[j * LCD_1IN9.WIDTH], LCD_1IN9.WIDTH * 2);
    }

}

/******************************************************************************
function :  Sends the image buffer in RAM to displays
parameter:
******************************************************************************/
void LCD_1IN9_Display(UWORD *Image)
{
    UWORD j;
    
    LCD_1IN9_SetWindows(0, 0, LCD_1IN9.HEIGHT, LCD_1IN9.WIDTH);
    LCD_1IN9_DC_1;
    for (j=0; j<LCD_1IN9.HEIGHT; j++) {
        DEV_SPI_Write_nByte((uint8_t *)&Image[j * LCD_1IN9.WIDTH], LCD_1IN9.WIDTH * 2);
    }
}

void LCD_1IN9_DisplayWindows(UWORD Xstart, UWORD Ystart, UWORD Xend, UWORD Yend, UWORD *Image)
{
    // display
    UDOUBLE Addr = 0;
    UWORD j;
    
    LCD_1IN9_SetWindows(Xstart, Ystart, Xend, Yend);
    LCD_1IN9_DC_1;
    
    for (j=Ystart; j<Yend-1; j++) {
        Addr = Xstart + j * LCD_1IN9.HEIGHT;
        DEV_SPI_Write_nByte((uint8_t *)&Image[Addr], (Xend-Xstart) * 2);
    }
}

void LCD_1IN9_DrawPoint(UWORD X, UWORD Y, UWORD Color)
{
    LCD_1IN9_SetWindows(X, Y, X, Y);
    LCD_1IN9_SendData_16Bit(Color);
}

void Handler_1IN9_LCD(int signo)
{
    // System Exit
    printf("\r\nHandler:Program stop\r\n");
    DEV_ModuleExit();
    exit(0);
}