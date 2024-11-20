#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <fcntl.h>
#include <unistd.h>
#include <time.h>
#include <sys/time.h>
#include <sys/ioctl.h>
#include <linux/types.h>
#include <linux/spi/spidev.h>
#include <wiringPi.h>
#include <arpa/inet.h>

/* Test the Adafuit 7789 display.  Initialize, clear to four different colors
 * then blank and exit.
 *
 * Use kernel SPI bit-banging.  This app assumes the display is wired this way:
 *
 * PIN   Name   PI
 * ---   -----  -------------------
 *  1    Vi     3v3 (pin 1)
 *  2    3V     (n/c)
 *  3    G      Ground (pin 6)
 *  4    CL     SPI0 CLK (GPIO 11) (pin 23) - SPI clock
 *  5    SO     SPI0 MISO (GPIO 9) (pin 21) - Not used by this app
 *  6    SI     SPI0 MOSI (GPIO 10) (pin 19) - SPI data out (to display)
 *  7    TC     SPI0 CE0 (GPIO 8) (pin 24) TFT chip select
 *  8    RT     (n/c) - Reset line
 *  9    DC     GPIO 25 (pin 22).  Data/Command switch
 * 10    CC     SPI0 CE1 (GPIO 7) (pin 26) SD card chip select
 * 11    BL     (n/c) - Ground this to disable the backlight
 */

static void pabort(const char *s)
{
    perror(s);
    abort();
}

/*
 * The GPIO pins we're directly manipulating
 */
enum {
    DC = 17
} pin;

/*
 * Some convenient 565 color constants
 */
enum {
    BLACK = 0x0000,
    RED   = 0xF800,
    GREEN = 0x07E0,
    BLUE  = 0x001F,
    WHITE = 0xFFFF
} color;

/*
 * The ST7789 commands I'm using
 */
enum {
    SWRESET = 0x01,    // Software reset
    SLPOUT  = 0x11,    // Sleep out
    NORON   = 0x13,    // Normal display mode on
    INVON   = 0x21,    // Inverse video mode on
    DISPON  = 0x29,    // Display on
    CASET   = 0x2A,    // Column address set
    RASET   = 0x2B,    // Row address set
    RAMWR   = 0x2C,    // RAM Write
    MADCTL  = 0x36,    // Memory and data access control
    COLMOD  = 0x3A     // Color mode (interface pixel format)
} commands;

/*
 * Set a pin's voltage.  Zero-value = low, non-zero = high
 */
void set_pin(uint8_t pin, uint8_t value)
{
    digitalWrite(pin, value);
}

void init_gpio(void)
{
    // Initialize GPIO library using Broadcom pin numbers
    wiringPiSetupGpio();

    // Set all the pins we're using to output mode
    pinMode(DC,   OUTPUT);
}

int init_spi(void)
{
    int fd;
    int ret;
    uint8_t mode;
    uint8_t bits;
    uint32_t speed;

    fd = open("/dev/spidev0.0", O_RDWR);
    if (fd < 0)
        pabort("Can't open device");

    // Normal/default SPI mode
    //
    mode = 0;
    ret = ioctl(fd, SPI_IOC_WR_MODE, &mode);
    if (ret == -1)
        pabort("Can't set SPI write mode");
    ret = ioctl(fd, SPI_IOC_RD_MODE, &mode);
    if (ret == -1)
        pabort("Can't get SPI read mode");
    printf("SPI mode: %d\n", mode);

    // Bits per word
    bits = 8;
    ret = ioctl(fd, SPI_IOC_WR_BITS_PER_WORD, &bits);
    if (ret == -1)
        pabort("Can't set bits");
    ret = ioctl(fd, SPI_IOC_RD_BITS_PER_WORD, &bits);
    if (ret == -1)
        pabort("Can't get bits\n");
    printf("Bits per word: %d\n", bits);

    // Max speed (in Hz)
    speed = 16000000;     // 16 MHz
    ret = ioctl(fd, SPI_IOC_WR_MAX_SPEED_HZ, &speed);
    if (ret == -1)
        pabort("Can't set max speed");
    ret = ioctl(fd, SPI_IOC_RD_MAX_SPEED_HZ, &speed);
    if (ret == -1)
        pabort("Can't get max speed\n");
    printf("Max speed: %ld\n", speed);

    return fd;
}

void command_mode(void)
{
    set_pin(DC, 0);
}

void data_mode(void)
{
    set_pin(DC, 1);
}

void write_buffer(int fd, uint8_t *buffer, size_t len)
{
    int                     ret;
    uint8_t                *pos;
    size_t                  remain;
    size_t                  send_len;
    struct spi_ioc_transfer tr = {0};

    // There's a limit of about 4K per ioctl here, so send everything in
    // 4K chunks

#define SPI_BUFFER_MAX (4095)

    pos = buffer;
    remain = len;

    while (remain)
    {
        send_len = (remain > SPI_BUFFER_MAX) ? SPI_BUFFER_MAX : remain;

        tr.tx_buf = (unsigned long)pos;
        tr.len    = send_len;

        ret = ioctl(fd, SPI_IOC_MESSAGE(1), &tr);
        if (ret < 1)
            pabort("Can't write SPI buffer");

        pos += send_len;
        remain -= send_len;
    }
}

void write_command(int fd, uint8_t value)
{
    command_mode();
    write_buffer(fd, &value, 1);
}

void write_data(int fd, uint8_t value)
{
    data_mode();
    write_buffer(fd, &value, 1);
}

void write_data16(int fd, uint16_t value)
{
    uint8_t buffer[2];

    buffer[0] = value >> 8;
    buffer[1] = value & 0xff;

    data_mode();
    write_buffer(fd, buffer, 2);
}

void write_data_buffer(int fd, uint8_t *buffer, size_t len)
{
    data_mode();
    write_buffer(fd, buffer, len);
}

/*
 * Repeatedly write an RGB 565 value
 */
void write_rgb565(int fd, uint16_t value, int count)
{
    static uint8_t buffer[240 * 240 * 2];
    
    uint8_t  val_hi;
    uint8_t  val_low;

    val_hi = value >> 8;

    for (int i=0; i<count; ++i)
    {
        buffer[i * 2] = val_hi;
        buffer[i * 2 + 1] = val_low;
    }

    data_mode();
    write_buffer(fd, buffer, count * 2);
}

/****************************
 *
 * ST7789 driver functions
 */
void init_display(int fd)
{
    write_command(fd, SWRESET);  // Software reset
    usleep(200000);
    write_command(fd, SLPOUT);   // Exit sleep mode
    usleep(200000);
    write_command(fd, COLMOD);   // Set color mode to RGB 16-bit 565
    write_data(fd, 0x55);
    usleep(10000);
    write_command(fd, MADCTL);   // Default memory order: left-right, top-bottom
    write_data(fd, 0);
    usleep(10000);
    write_command(fd, INVON);    // Hack - display invert
    usleep(10000);
    write_command(fd, NORON);    // Normal display mode
    usleep(10000);
    write_command(fd, DISPON);   // Display on
    usleep(200000);
}

void set_addr_window(int fd, uint16_t left, uint16_t top,
                     uint16_t right, uint16_t bottom)
{
    write_command(fd, CASET);
    write_data16(fd, left);
    write_data16(fd, right);
    write_command(fd, RASET);
    write_data16(fd, top);
    write_data16(fd, bottom);
}

void draw_pixel(int fd, uint16_t x, uint16_t y, uint16_t color)
{
    set_addr_window(fd, x, y, x, y);
    write_command(fd, RAMWR);
    write_rgb565(fd, color, 1);
}

void fill_rect(int fd, uint16_t left, uint16_t top,
               uint16_t right, uint16_t bottom, uint16_t color)
{
    uint16_t width = right - left + 1;
    uint16_t height = bottom - top + 1;

    set_addr_window(fd, left, top, right, bottom);
    write_command(fd, RAMWR);
    write_rgb565(fd, color, width * height);
}

void fill_screen(int fd, uint16_t color)
{
    fill_rect(fd, 0, 0, 239, 239, color);
}

void clear_screen(int fd)
{
    fill_screen(fd, BLACK);
}

void dump_bitmap(int fd, uint16_t left, uint16_t top, uint16_t right,
                 uint16_t bottom, void *buffer)
{
    uint16_t width = right - left + 1;
    uint16_t height = bottom - top + 1;

    set_addr_window(fd, left, top, right, bottom);
    write_command(fd, RAMWR);
    write_data_buffer(fd, (uint8_t *)buffer, width * height * 2);
}

/************************************
 *
 * Testing routines
 */

double time_delta(struct timeval *start, struct timeval *end)
{
    double s = start->tv_sec + (start->tv_usec / 1000000.0);
    double e = end->tv_sec + (end->tv_usec / 1000000.0);

    return e - s;
}

void time_display(int fd)
{
    struct timeval start, end;

    gettimeofday(&start, NULL);
    fill_screen(fd, GREEN);
    gettimeofday(&end, NULL);

    printf("Filled green in %.4f seconds\n", time_delta(&start, &end));
}

void draw_x(int fd)
{
    struct timeval start, end;

    gettimeofday(&start, NULL);
    for (int i=0; i<240; ++i)
    {
        draw_pixel(fd, i, i, RED);
        draw_pixel(fd, i, 240-i, RED);
        draw_pixel(fd, 0, i, GREEN);
        draw_pixel(fd, 239, i, GREEN);
        draw_pixel(fd, i, 0, WHITE);
        draw_pixel(fd, i, 239, WHITE);
    }
    gettimeofday(&end, NULL);
    printf("Drew shape in %.4f seconds\n", time_delta(&start, &end));
}

void draw_x_buffered(int fd)
{
    // One pixel per element.
    // Byte = x * 240 + y
    //
    uint16_t buffer[240 * 240];
    struct timeval start, end;

    gettimeofday(&start, NULL);
    
    memset(buffer, 0, 240*240*2);
    for (int i = 0; i<240; ++i)
    {
        buffer[i   * 240 + i]       = htons(GREEN);
        buffer[i   * 240 + (240-i)] = htons(GREEN);
        buffer[0   * 240 + i]       = htons(RED);
        buffer[239 * 240 + i]       = htons(RED);
        buffer[i   * 240 + 0]       = htons(BLUE);
        buffer[i   * 240 + 239]     = htons(BLUE);
    }

    dump_bitmap(fd, 0, 0, 239, 239, buffer);
    gettimeofday(&end, NULL);
    printf("Drew buffered shape in %.4f seconds\n", time_delta(&start, &end));
}

int main(int argc, char *argv[])
{
    int fd;

    init_gpio();
    fd = init_spi();

    init_display(fd);
    time_display(fd);
    sleep(3);
    clear_screen(fd);
    draw_x(fd);
    sleep(3);
    clear_screen(fd);
    draw_x_buffered(fd);
    
    return 0;
}
