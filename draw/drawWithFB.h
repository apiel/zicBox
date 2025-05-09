#pragma once

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

    // void quit() override
    // {
    // }

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
        for (int x = 0; x < width; x++) {
            for (int y = 0; y < height; y++) {
                int location = x * finfo.line_length + y * (vinfo.bits_per_pixel / 8);

                Color color = screenBuffer[y][x];
                uint16_t rgb565 = ((color.r & 0xF8) << 8) | ((color.g & 0xFC) << 3) | (color.b >> 3);
                *((uint16_t*)(fbp + location)) = rgb565;
            }
        }
    }

    // void config(nlohmann::json& config) override
    // {
    //     Draw::config(config);
    // }
};
