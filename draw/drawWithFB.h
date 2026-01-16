/** Description:
This C++ class, `DrawWithFB`, is a specialized tool designed for low-level graphics display, primarily targeting systems running the Linux operating system. It provides a highly efficient way to draw images directly onto the computer monitor without relying on standard graphical systems or window managers.

**How it Works:**

The core mechanism involves accessing the Linux **Framebuffer** device. The framebuffer is the designated region of memory that holds the picture currently displayed on the screen.

1.  **Initialization:** The class starts by opening the connection to the physical screen hardware, typically found at a system location like `/dev/fb0`. It queries the system to determine crucial details, such as the screen’s exact resolution and how many bits are used for each color pixel. Most importantly, it uses a technique called *memory mapping* to create a direct, shared link between the program's internal variables and the physical memory of the display hardware.
2.  **Drawing:** The class holds the image data it wants to display in an internal buffer. The `render` function then efficiently copies this internal image data straight into the linked framebuffer memory. This instant transfer bypasses typical operating system overhead, resulting in very fast updates. The system also handles the necessary translation of standard colors into the specific color format required by the screen hardware.
3.  **Cleanup:** When the drawing operations are complete and the program closes, it safely disconnects the direct memory link and closes the hardware connection, ensuring system stability.

In summary, this class offers a fast, direct-access method for rendering graphics, making it suitable for specialized or embedded applications where performance and minimal software layers are essential.

sha: 20d443dd0a4ca2e54ada04a895b44a7504830822308fdfe7ee180928595a78cb 
*/
#pragma once

// to remove blinking cursor
// sudo sh -c 'setterm --cursor off --blank force --clear > /dev/tty1'

#include "./draw.h"

#include <fcntl.h>
#include <linux/fb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <unistd.h>

#include <arpa/inet.h> // htons

class DrawWithFB : public Draw {
protected:
    uint8_t* fbp = nullptr;
    struct fb_var_screeninfo vinfo;
    struct fb_fix_screeninfo finfo;
    long screensize = 0;
    int fb = -1;
    int width = 0;
    int height = 0;

public:
    DrawWithFB(Styles& styles)
        : Draw(styles)
    {
    }

    ~DrawWithFB()
    {
        munmap(fbp, screensize);
        if (fb != -1) {
            close(fb);
        }
    }

    void init() override
    {
        logDebug("Initializing framebuffer /dev/fb0");
        fb = open("/dev/fb0", O_RDWR);
        if (fb == -1) {
            logError("Error opening framebuffer /dev/fb0");
            return;
        }

        ioctl(fb, FBIOGET_FSCREENINFO, &finfo);
        ioctl(fb, FBIOGET_VSCREENINFO, &vinfo);

        screensize = vinfo.yres_virtual * finfo.line_length;

        fbp = (uint8_t*)mmap(0, screensize, PROT_READ | PROT_WRITE, MAP_SHARED, fb, 0);
        if ((intptr_t)fbp == -1) {
            logError("Error mapping framebuffer /dev/fb0");
            close(fb);
            return;
        }

        width = vinfo.xres < styles.screen.w ? vinfo.xres : styles.screen.w;
        height = vinfo.yres < styles.screen.h ? vinfo.yres : styles.screen.h;

        logDebug("Framebuffer size: %dx%d buffer size: %dx%d", vinfo.xres, vinfo.yres, width, height);
    }

    void render() override
    {
        for (int y = 0; y < height; y++) {
            for (int x = 0; x < width; x++) {
                int location = y * finfo.line_length + x * (vinfo.bits_per_pixel / 8);

                Color color = screenBuffer[y][x];
                uint16_t rgb565 = ((color.r & 0xF8) << 8) | ((color.g & 0xFC) << 3) | (color.b >> 3);
                *((uint16_t*)(fbp + location)) = rgb565;
            }
        }
    }
};
