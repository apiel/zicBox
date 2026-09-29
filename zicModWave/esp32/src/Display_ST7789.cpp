#include "Display_ST7789.h"
#include "driver/spi_master.h"

static spi_device_handle_t spi_handle = NULL;

void SPI_Init()
{
    spi_bus_config_t buscfg = {};
    buscfg.sclk_io_num = EXAMPLE_PIN_NUM_SCLK;
    buscfg.mosi_io_num = EXAMPLE_PIN_NUM_MOSI;
    buscfg.miso_io_num = -1;
    buscfg.quadwp_io_num = -1;
    buscfg.quadhd_io_num = -1;
    buscfg.max_transfer_sz = 320 * 172 * 2 + 64;

    ESP_ERROR_CHECK(spi_bus_initialize(SPI2_HOST, &buscfg, SPI_DMA_CH_AUTO));

    spi_device_interface_config_t devcfg = {};
    devcfg.clock_speed_hz = 80000000; // 80 MHz High-Speed Hardware SPI Clock
    devcfg.mode = 3;                  // SPI Mode 3
    devcfg.spics_io_num = EXAMPLE_PIN_NUM_LCD_CS;
    devcfg.queue_size = 7;

    ESP_ERROR_CHECK(spi_bus_add_device(SPI2_HOST, &devcfg, &spi_handle));
}

void LCD_WriteCommand(uint8_t Cmd)  
{ 
    digitalWrite(EXAMPLE_PIN_NUM_LCD_DC, LOW); 
    spi_transaction_t t = {};
    t.length = 8;
    t.tx_buffer = &Cmd;
    spi_device_polling_transmit(spi_handle, &t);
}

void LCD_WriteData(uint8_t Data) 
{ 
    digitalWrite(EXAMPLE_PIN_NUM_LCD_DC, HIGH); 
    spi_transaction_t t = {};
    t.length = 8;
    t.tx_buffer = &Data;
    spi_device_polling_transmit(spi_handle, &t);
}    

void LCD_WriteData_Word(uint16_t Data)
{
    uint16_t swapped = (Data >> 8) | (Data << 8);
    digitalWrite(EXAMPLE_PIN_NUM_LCD_DC, HIGH); 
    spi_transaction_t t = {};
    t.length = 16;
    t.tx_buffer = &swapped;
    spi_device_polling_transmit(spi_handle, &t);
}   

void LCD_WriteData_nbyte(uint8_t* SetData, uint32_t Size) 
{ 
    digitalWrite(EXAMPLE_PIN_NUM_LCD_DC, HIGH); 
    spi_transaction_t t = {};
    t.length = Size * 8;
    t.tx_buffer = SetData;
    spi_device_polling_transmit(spi_handle, &t);
} 

void LCD_Reset(void)
{
  digitalWrite(EXAMPLE_PIN_NUM_LCD_CS, LOW);       
  delay(50);
  digitalWrite(EXAMPLE_PIN_NUM_LCD_RST, LOW); 
  delay(50);
  digitalWrite(EXAMPLE_PIN_NUM_LCD_RST, HIGH); 
  delay(120);
  digitalWrite(EXAMPLE_PIN_NUM_LCD_CS, HIGH);
}

void Backlight_Init(void)
{
  pinMode(EXAMPLE_PIN_NUM_BK_LIGHT, OUTPUT);
  digitalWrite(EXAMPLE_PIN_NUM_BK_LIGHT, HIGH);
}

void Set_Backlight(uint8_t Light)
{
  pinMode(EXAMPLE_PIN_NUM_BK_LIGHT, OUTPUT);
  digitalWrite(EXAMPLE_PIN_NUM_BK_LIGHT, Light > 0 ? HIGH : LOW);
}

void LCD_Init(void)
{
  pinMode(EXAMPLE_PIN_NUM_LCD_CS, OUTPUT);
  pinMode(EXAMPLE_PIN_NUM_LCD_DC, OUTPUT);
  pinMode(EXAMPLE_PIN_NUM_LCD_RST, OUTPUT); 
  Backlight_Init();
  SPI_Init();

  LCD_Reset();

  // Official Waveshare JD9853 Initialization Operations
  LCD_WriteCommand(0x11);
  delay(120);

  LCD_WriteCommand(0xDF); LCD_WriteData(0x98); LCD_WriteData(0x53);
  LCD_WriteCommand(0xB2); LCD_WriteData(0x23);

  LCD_WriteCommand(0xB7); LCD_WriteData(0x00); LCD_WriteData(0x47); LCD_WriteData(0x00); LCD_WriteData(0x6F);

  LCD_WriteCommand(0xBB); LCD_WriteData(0x1C); LCD_WriteData(0x1A); LCD_WriteData(0x55); LCD_WriteData(0x73); LCD_WriteData(0x63); LCD_WriteData(0xF0);

  LCD_WriteCommand(0xC0); LCD_WriteData(0x44); LCD_WriteData(0xA4);
  LCD_WriteCommand(0xC1); LCD_WriteData(0x16);

  LCD_WriteCommand(0xC3); LCD_WriteData(0x7D); LCD_WriteData(0x07); LCD_WriteData(0x14); LCD_WriteData(0x06); LCD_WriteData(0xCF); LCD_WriteData(0x71); LCD_WriteData(0x72); LCD_WriteData(0x77);

  LCD_WriteCommand(0xC4); LCD_WriteData(0x00); LCD_WriteData(0x00); LCD_WriteData(0xA0); LCD_WriteData(0x79); LCD_WriteData(0x0B); LCD_WriteData(0x0A); LCD_WriteData(0x16); LCD_WriteData(0x79); LCD_WriteData(0x0B); LCD_WriteData(0x0A); LCD_WriteData(0x16); LCD_WriteData(0x82);

  LCD_WriteCommand(0xC8);
  static const uint8_t c8_data[32] = {
    0x3F, 0x32, 0x29, 0x29, 0x27, 0x2B, 0x27, 0x28, 0x28, 0x26, 0x25, 0x17, 0x12, 0x0D, 0x04, 0x00,
    0x3F, 0x32, 0x29, 0x29, 0x27, 0x2B, 0x27, 0x28, 0x28, 0x26, 0x25, 0x17, 0x12, 0x0D, 0x04, 0x00
  };
  for (int i = 0; i < 32; i++) LCD_WriteData(c8_data[i]);

  LCD_WriteCommand(0xD0); LCD_WriteData(0x04); LCD_WriteData(0x06); LCD_WriteData(0x6B); LCD_WriteData(0x0F); LCD_WriteData(0x00);

  LCD_WriteCommand(0xD7); LCD_WriteData(0x00); LCD_WriteData(0x30);
  LCD_WriteCommand(0xE6); LCD_WriteData(0x14);
  LCD_WriteCommand(0xDE); LCD_WriteData(0x01);

  LCD_WriteCommand(0xB7); LCD_WriteData(0x03); LCD_WriteData(0x13); LCD_WriteData(0xEF); LCD_WriteData(0x35); LCD_WriteData(0x35);

  LCD_WriteCommand(0xC1); LCD_WriteData(0x14); LCD_WriteData(0x15); LCD_WriteData(0xC0);

  LCD_WriteCommand(0xC2); LCD_WriteData(0x06); LCD_WriteData(0x3A);
  LCD_WriteCommand(0xC4); LCD_WriteData(0x72); LCD_WriteData(0x12);
  LCD_WriteCommand(0xBE); LCD_WriteData(0x00);
  LCD_WriteCommand(0xDE); LCD_WriteData(0x02);

  LCD_WriteCommand(0xE5); LCD_WriteData(0x00); LCD_WriteData(0x02); LCD_WriteData(0x00);
  LCD_WriteCommand(0xE5); LCD_WriteData(0x01); LCD_WriteData(0x02); LCD_WriteData(0x00);

  LCD_WriteCommand(0xDE); LCD_WriteData(0x00);
  LCD_WriteCommand(0x35); LCD_WriteData(0x00);
  LCD_WriteCommand(0x3A); LCD_WriteData(0x05);

  LCD_WriteCommand(0x2A); LCD_WriteData(0x00); LCD_WriteData(0x22); LCD_WriteData(0x00); LCD_WriteData(0xCD);
  LCD_WriteCommand(0x2B); LCD_WriteData(0x00); LCD_WriteData(0x00); LCD_WriteData(0x01); LCD_WriteData(0x3F);

  LCD_WriteCommand(0xDE); LCD_WriteData(0x02);
  LCD_WriteCommand(0xE5); LCD_WriteData(0x00); LCD_WriteData(0x02); LCD_WriteData(0x00);
  LCD_WriteCommand(0xDE); LCD_WriteData(0x00);

  // MADCTL: 0x60 (Landscape 320x172 rotated 180 deg)
  LCD_WriteCommand(0x36);
  LCD_WriteData(0x60);

  LCD_WriteCommand(0x21); // Display Inversion ON
  delay(10);
  LCD_WriteCommand(0x29); // Display ON
}

void LCD_SetCursor(uint16_t Xstart, uint16_t Ystart, uint16_t Xend, uint16_t Yend)
{ 
  uint16_t x0 = Xstart + Offset_X;
  uint16_t x1 = Xend + Offset_X;
  uint16_t y0 = Ystart + Offset_Y;
  uint16_t y1 = Yend + Offset_Y;

  // set the X coordinates (0x2A)
  LCD_WriteCommand(0x2A);
  LCD_WriteData(x0 >> 8);
  LCD_WriteData(x0 & 0xFF);
  LCD_WriteData(x1 >> 8);
  LCD_WriteData(x1 & 0xFF);
  
  // set the Y coordinates (0x2B)
  LCD_WriteCommand(0x2B);
  LCD_WriteData(y0 >> 8);
  LCD_WriteData(y0 & 0xFF);
  LCD_WriteData(y1 >> 8);
  LCD_WriteData(y1 & 0xFF);

  LCD_WriteCommand(0x2C);
}
