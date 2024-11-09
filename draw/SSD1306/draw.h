#ifndef _UI_DRAW_H_
#define _UI_DRAW_H_

#include "../../helpers/i2c.h"
#include "../../log.h"
#include "../../plugins/components/drawInterface.h"
#include <stdexcept>
#include <string>

#define SSD1306_I2C_ADDR 0x3c

#define SSD1306_COMM_CONTROL_BYTE 0x00
#define SSD1306_DATA_CONTROL_BYTE 0x40

#define SSD1306_COMM_DISPLAY_OFF 0xae
#define SSD1306_COMM_DISPLAY_ON 0xaf
#define SSD1306_COMM_HORIZ_NORM 0xa0
#define SSD1306_COMM_HORIZ_FLIP 0xa1
#define SSD1306_COMM_DISP_NORM 0xa6
#define SSD1306_COMM_MULTIPLEX 0xa8
#define SSD1306_COMM_VERT_OFFSET 0xd3
#define SSD1306_COMM_CLK_SET 0xd5
#define SSD1306_COMM_PRECHARGE 0xd9
#define SSD1306_COMM_COM_PIN 0xda
#define SSD1306_COMM_DESELECT_LV 0xdb
#define SSD1306_COMM_CONTRAST 0x81
#define SSD1306_COMM_DISABLE_SCROLL 0x2e
#define SSD1306_COMM_PAGE_NUMBER 0xb0
#define SSD1306_COMM_LOW_COLUMN 0x00
#define SSD1306_COMM_HIGH_COLUMN 0x10

#define SSD1306_COMM_START_LINE 0x40

#define SSD1306_COMM_CHARGE_PUMP 0x8d

#define SSD1306_COMM_SCAN_NORM 0xc0
#define SSD1306_COMM_SCAN_REVS 0xc8

#define SSD1306_COMM_MEMORY_MODE 0x20
#define SSD1306_COMM_SET_COL_ADDR 0x21
#define SSD1306_COMM_SET_PAGE_ADDR 0x22

#define SSD1306_HORI_MODE 0x00
#define SSD1306_VERT_MODE 0x01
#define SSD1306_PAGE_MODE 0x02

#define SSD1306_FONT_SMALL 0x00
#define SSD1306_FONT_NORMAL 0x01

#define SSD1306_128_64_LINES 64
#define SSD1306_128_32_LINES 32
#define SSD1306_64_48_LINES 48

#define SSD1306_128_64_COLUMNS 128
#define SSD1306_128_32_COLUMNS 128
#define SSD1306_64_48_COLUMNS 64

class Draw : public DrawInterface {
protected:
    I2c i2c;

    void initDisplay(uint8_t i2c_dev = 1)
    {
        if (!i2c.init(i2c_dev, SSD1306_I2C_ADDR)) {
            throw std::runtime_error("Could not initialize SSD1306 display.");
        }

        // test i2c connection
        uint8_t cmd = SSD1306_COMM_CONTROL_BYTE;
        uint8_t result = 0;
        i2c.send(&cmd, 1);
        i2c.pull(&result, 1);
        if (result == 0) {
            throw std::runtime_error("Connection to SSD1306 display failed.");
        }
    }

    void validateDisplaySize()
    {
        if (styles.screen.w != SSD1306_128_64_COLUMNS
            && styles.screen.w != SSD1306_128_32_COLUMNS
            && styles.screen.w != SSD1306_64_48_COLUMNS) {
            throw std::runtime_error("Unsupported display width " + std::to_string(styles.screen.w));
        }
        if (styles.screen.h != SSD1306_128_64_LINES
            && styles.screen.h != SSD1306_128_32_LINES
            && styles.screen.h != SSD1306_64_48_LINES) {
            throw std::runtime_error("Unsupported display height " + std::to_string(styles.screen.h));
        }
    }

    void setDisplayDefaultConfig()
    {
        uint8_t data_buf[] = {
            SSD1306_COMM_CONTROL_BYTE, // command control byte
            SSD1306_COMM_DISPLAY_OFF, // display off
            SSD1306_COMM_DISP_NORM, // Set Normal Display (default)
            SSD1306_COMM_CLK_SET, // SETDISPLAYCLOCKDIV
            0x80, // the suggested ratio 0x80
            SSD1306_COMM_MULTIPLEX, // SSD1306_SETMULTIPLEX
            (uint8_t)(styles.screen.h - 1), // height is 32 or 64 (always -1)
            SSD1306_COMM_VERT_OFFSET, // SETDISPLAYOFFSET
            0, // no offset
            SSD1306_COMM_START_LINE, // SETSTARTLINE
            SSD1306_COMM_CHARGE_PUMP, // CHARGEPUMP
            0x14, // turn on charge pump
            SSD1306_COMM_MEMORY_MODE, // MEMORYMODE
            SSD1306_PAGE_MODE, // page mode
            SSD1306_COMM_HORIZ_NORM, // SEGREMAP  Mirror screen horizontally (A0)
            SSD1306_COMM_SCAN_NORM, // COMSCANDEC Rotate screen vertically (C0)
            SSD1306_COMM_COM_PIN, // HARDWARE PIN
            styles.screen.h == SSD1306_128_32_LINES ? (uint8_t)0x02 : (uint8_t)0x12, // 0x02 for 32 lines, 0x12 for 64 lines or 48 lines
            SSD1306_COMM_CONTRAST, // SETCONTRAST
            0x7f, // default contract value
            SSD1306_COMM_PRECHARGE, // SETPRECHARGE
            0xf1, // default precharge value
            SSD1306_COMM_DESELECT_LV, // SETVCOMDETECT
            0x40, // default deselect value
            0xa4, // DISPLAYALLON_RESUME
            SSD1306_COMM_DISP_NORM, // NORMALDISPLAY
            SSD1306_COMM_DISPLAY_ON, // DISPLAY ON
            SSD1306_COMM_DISABLE_SCROLL, // Stop scroll
        };

        i2c.send(data_buf, sizeof(data_buf) / sizeof(data_buf[0]));
    }

    void oledPosition(uint8_t x, uint8_t page)
    {
        uint8_t data_buf[4];
        data_buf[0] = SSD1306_COMM_CONTROL_BYTE;
        data_buf[1] = SSD1306_COMM_PAGE_NUMBER | (page & 0x0f);
        data_buf[2] = SSD1306_COMM_LOW_COLUMN | (x & 0x0f);
        data_buf[3] = SSD1306_COMM_HIGH_COLUMN | ((x >> 4) & 0x0f);
        i2c.send(data_buf, 4);
    }

    // The GDDRAM is a bit mapped static RAM holding the bit pattern to be displayed. The size of the RAM is
    // 128 x 64 bits and the RAM is divided into eight pages, from PAGE0 to PAGE7
    void oledClearPage(uint8_t page)
    {
        oledPosition(0, page);
        uint8_t data_buf[1 + styles.screen.w];
        data_buf[0] = SSD1306_DATA_CONTROL_BYTE;
        for (uint16_t i = 0; i < styles.screen.w; i++) {
            data_buf[i + 1] = 0x00;
        }
        i2c.send(data_buf, 1 + styles.screen.w);
    }

    uint8_t oledGetPageCount()
    {
        return styles.screen.h / 8;
    }

public:
    Draw(Styles& styles)
        : DrawInterface(styles)
    {
    }

    ~Draw()
    {
    }

    void init()
    {
        validateDisplaySize();
        initDisplay();
        setDisplayDefaultConfig();
        clear();
    }

    void renderNext()
    {
    }

    void triggerRendering()
    {
    }

    void render()
    {
    }

    int textCentered(Point position, std::string text, uint32_t size, DrawTextOptions options = {})
    {
        return 123;
    }

    int text(Point position, std::string text, uint32_t size, DrawTextOptions options = {})
    {
        return 123;
    }

    int textRight(Point position, std::string text, uint32_t size, DrawTextOptions options = {})
    {
        return 123;
    }

    void clear()
    {
        uint8_t pages = oledGetPageCount();
        for (uint8_t page = 0; page < pages; page++) {
            oledClearPage(page);
        }
    }

    void filledRect(Point position, Size size, DrawOptions options = {})
    {
    }

    void rect(Point position, Size size, DrawOptions options = {})
    {
    }

    void filledRect(Point position, Size size, uint8_t radius, DrawOptions options = {})
    {
        filledRect({ position.x + radius, position.y }, { size.w - 2 * radius, size.h }, options);
        filledRect({ position.x, position.y + radius }, { size.w, size.h - 2 * radius }, options);
        filledPie({ position.x + radius, position.y + radius }, radius, 180, 270, options);
        filledPie({ position.x + size.w - radius, position.y + radius }, radius, 270, 360, options);
        filledPie({ position.x + radius, position.y + size.h - radius }, radius, 90, 180, options);
        filledPie({ position.x + size.w - radius, position.y + size.h - radius }, radius, 0, 90, options);
    }

    void rect(Point position, Size size, uint8_t radius, DrawOptions options = {})
    {
        arc({ position.x + radius, position.y + radius }, radius, 180, 270, options);
        line({ position.x, position.y + radius }, { position.x, position.y + size.h - radius }, options);
        arc({ position.x + size.w - radius, position.y + radius }, radius, 270, 360, options);
        line({ position.x + radius, position.y }, { position.x + size.w - radius, position.y }, options);
        arc({ position.x + radius, position.y + size.h - radius }, radius, 90, 180, options);
        line({ position.x + radius, position.y + size.h }, { position.x + size.w - radius, position.y + size.h }, options);
        arc({ position.x + size.w - radius, position.y + size.h - radius }, radius, 0, 90, options);
        line({ position.x + size.w, position.y + size.h - radius }, { position.x + size.w, position.y + radius }, options);
    }

    void arc(Point position, int radius, int startAngle, int endAngle, DrawOptions options = {})
    {
    }

    void filledPie(Point position, int radius, int startAngle, int endAngle, DrawOptions options = {})
    {
    }

    void filledEllipse(Point position, int radiusX, int radiusY, DrawOptions options = {})
    {
    }

    void ellipse(Point position, int radiusX, int radiusY, DrawOptions options = {})
    {
    }

    void filledPolygon(std::vector<Point> points, DrawOptions options = {})
    {
    }

    void polygon(std::vector<Point> points, DrawOptions options = {})
    {
    }

    void aaline(Point start, Point end, DrawOptions options = {})
    {
    }

    void aalines(std::vector<Point> points, DrawOptions options = {})
    {
    }

    void line(Point start, Point end, DrawOptions options = {})
    {
    }

    void lines(std::vector<Point> points, DrawOptions options = {})
    {
    }

    void pixel(Point position, DrawOptions options = {})
    {
    }

    void* setTextureRenderer(Size size)
    {
        return NULL;
    }

    void setMainRenderer()
    {
    }

    void destroyTexture(void* texture)
    {
    }

    void applyTexture(void* texture, Rect dest)
    {
    }

    Color darken(Color color, float amount)
    {
        return color;
    }

    Color lighten(Color color, float amount)
    {
        return color;
    }

    Color alpha(Color color, float amount)
    {
        return color;
    }

    Color getColor(char* color)
    {
        return styles.colors.white;
    }

    bool config(char* key, char* value)
    {
        return false;
    }
};

#endif