#ifndef _UI_DRAW_H_
#define _UI_DRAW_H_

#include "../log.h"
#include "../plugins/components/drawInterface.h"
#include <stdexcept>
#include <string>

#include <fcntl.h>
#include <linux/i2c-dev.h>
#include <sys/ioctl.h>
#include <unistd.h>

#define SSD1306_I2C_ADDR 0x3c

#define SSD1306_COMM_CONTROL_BYTE 0x00
#define SSD1306_DATA_CONTROL_BYTE 0x40

#define SSD1306_COMM_DISPLAY_OFF 0xae
#define SSD1306_COMM_DISPLAY_ON 0xaf
#define SSD1306_COMM_HORIZ_NORM 0xa0
#define SSD1306_COMM_HORIZ_FLIP 0xa1
#define SSD1306_COMM_RESUME_RAM 0xa4
#define SSD1306_COMM_IGNORE_RAM 0xa5
#define SSD1306_COMM_DISP_NORM 0xa6
#define SSD1306_COMM_DISP_INVERSE 0xa7
#define SSD1306_COMM_MULTIPLEX 0xa8
#define SSD1306_COMM_VERT_OFFSET 0xd3
#define SSD1306_COMM_CLK_SET 0xd5
#define SSD1306_COMM_PRECHARGE 0xd9
#define SSD1306_COMM_COM_PIN 0xda
#define SSD1306_COMM_DESELECT_LV 0xdb
#define SSD1306_COMM_CONTRAST 0x81
#define SSD1306_COMM_DISABLE_SCROLL 0x2e
#define SSD1306_COMM_ENABLE_SCROLL 0x2f
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

class I2c {
public:
    int file = 0;

    bool init(int i2c, int dev_addr)
    {
        if (file == 0) {
            char filename[32];
            sprintf(filename, "/dev/i2c-%d", i2c); // I2C bus number passed
            file = open(filename, O_RDWR);
            if (file < 0) {
                file = 0;
                return false;
            }
            if (ioctl(file, I2C_SLAVE, dev_addr) < 0) { // set slave address
                close(file);
                file = 0;
                return false;
            }
            return true;
        }
        // assume done init already
        return true;
    }

    uint8_t end()
    {
        if (file != 0) {
            close(file);
            file = 0;
            return 0;
        }
        return 1;
    }

    uint8_t send(uint8_t* ptr, int16_t len)
    {
        if (file == 0 || ptr == 0 || len <= 0)
            return 1;

        write(file, ptr, len);
        return 0;
    }

    uint8_t pull(uint8_t* ptr, int16_t len)
    {
        if (file == 0 || ptr == 0 || len <= 0)
            return 1;

        read(file, ptr, len);
        return 0;
    }
};

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

    uint8_t setDisplayDefaultConfig(uint8_t oled_lines = SSD1306_128_64_LINES)
    {
        uint16_t i = 0;
        uint8_t data_buf[128];
        data_buf[i++] = SSD1306_COMM_CONTROL_BYTE; // command control byte
        data_buf[i++] = SSD1306_COMM_DISPLAY_OFF; // display off
        data_buf[i++] = SSD1306_COMM_DISP_NORM; // Set Normal Display (default)
        data_buf[i++] = SSD1306_COMM_CLK_SET; // SETDISPLAYCLOCKDIV
        data_buf[i++] = 0x80; // the suggested ratio 0x80
        data_buf[i++] = SSD1306_COMM_MULTIPLEX; // SSD1306_SETMULTIPLEX
        data_buf[i++] = oled_lines - 1; // height is 32 or 64 (always -1)
        data_buf[i++] = SSD1306_COMM_VERT_OFFSET; // SETDISPLAYOFFSET
        data_buf[i++] = 0; // no offset
        data_buf[i++] = SSD1306_COMM_START_LINE; // SETSTARTLINE
        data_buf[i++] = SSD1306_COMM_CHARGE_PUMP; // CHARGEPUMP
        data_buf[i++] = 0x14; // turn on charge pump
        data_buf[i++] = SSD1306_COMM_MEMORY_MODE; // MEMORYMODE
        data_buf[i++] = SSD1306_PAGE_MODE; // page mode
        data_buf[i++] = SSD1306_COMM_HORIZ_NORM; // SEGREMAP  Mirror screen horizontally (A0)
        data_buf[i++] = SSD1306_COMM_SCAN_NORM; // COMSCANDEC Rotate screen vertically (C0)
        data_buf[i++] = SSD1306_COMM_COM_PIN; // HARDWARE PIN
        if (oled_lines == SSD1306_128_32_LINES)
            data_buf[i++] = 0x02; // for 32 lines
        else
            data_buf[i++] = 0x12; // for 64 lines or 48 lines
        data_buf[i++] = SSD1306_COMM_CONTRAST; // SETCONTRAST
        data_buf[i++] = 0x7f; // default contract value
        data_buf[i++] = SSD1306_COMM_PRECHARGE; // SETPRECHARGE
        data_buf[i++] = 0xf1; // default precharge value
        data_buf[i++] = SSD1306_COMM_DESELECT_LV; // SETVCOMDETECT
        data_buf[i++] = 0x40; // default deselect value
        data_buf[i++] = SSD1306_COMM_RESUME_RAM; // DISPLAYALLON_RESUME
        data_buf[i++] = SSD1306_COMM_DISP_NORM; // NORMALDISPLAY
        data_buf[i++] = SSD1306_COMM_DISPLAY_ON; // DISPLAY ON
        data_buf[i++] = SSD1306_COMM_DISABLE_SCROLL; // Stop scroll

        return i2c.send(data_buf, i);
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
        initDisplay();
        setDisplayDefaultConfig();
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