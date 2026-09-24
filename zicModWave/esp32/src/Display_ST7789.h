#pragma once
#include <Arduino.h>
#include <SPI.h>

#define LCD_WIDTH   320
#define LCD_HEIGHT  172

#define SPIFreq                        80000000
#define EXAMPLE_PIN_NUM_MISO           -1
#define EXAMPLE_PIN_NUM_MOSI           39
#define EXAMPLE_PIN_NUM_SCLK           38
#define EXAMPLE_PIN_NUM_LCD_CS         21
#define EXAMPLE_PIN_NUM_LCD_DC         45
#define EXAMPLE_PIN_NUM_LCD_RST        40
#define EXAMPLE_PIN_NUM_BK_LIGHT       46

#define Offset_X 0
#define Offset_Y 34

void LCD_Init(void);
void LCD_SetCursor(uint16_t Xstart, uint16_t Ystart, uint16_t Xend, uint16_t Yend);
void LCD_WriteData_nbyte(uint8_t* SetData, uint32_t Size);
void Backlight_Init(void);
void Set_Backlight(uint8_t Light);
