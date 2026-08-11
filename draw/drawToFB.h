#pragma once

#include "draw.h"
#include "log.h"

#include <fcntl.h>
#include <linux/fb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <unistd.h>
#include <string>

class DrawToFB {
protected:
    Draw& draw;
    uint8_t* fbp = nullptr;
    struct fb_var_screeninfo vinfo {};
    struct fb_fix_screeninfo finfo {};
    long screensize = 0;
    int fb = -1;
    int width = 0;
    int height = 0;
    std::string device = "/dev/fb0";

public:
    DrawToFB(Draw& draw, const std::string& device = "/dev/fb0")
        : draw(draw)
        , device(device)
    {
        logInfo("Constructing DrawToFB for device %s", device.c_str());
    }

    ~DrawToFB()
    {
        if (fbp && fbp != MAP_FAILED) {
            munmap(fbp, screensize);
        }
        if (fb != -1) {
            close(fb);
        }
    }

    void setDevice(const std::string& dev) { device = dev; }

    void init()
    {
        logDebug("Initializing framebuffer %s", device.c_str());
        fb = open(device.c_str(), O_RDWR);
        if (fb == -1) {
            logError("Error opening framebuffer %s", device.c_str());
            return;
        }

        if (ioctl(fb, FBIOGET_FSCREENINFO, &finfo) == -1) {
            logError("Error reading fixed screen information from %s", device.c_str());
            close(fb);
            fb = -1;
            return;
        }

        if (ioctl(fb, FBIOGET_VSCREENINFO, &vinfo) == -1) {
            logError("Error reading variable screen information from %s", device.c_str());
            close(fb);
            fb = -1;
            return;
        }

        screensize = vinfo.yres_virtual * finfo.line_length;

        fbp = (uint8_t*)mmap(0, screensize, PROT_READ | PROT_WRITE, MAP_SHARED, fb, 0);
        if ((intptr_t)fbp == -1 || fbp == MAP_FAILED) {
            logError("Error mapping framebuffer %s", device.c_str());
            close(fb);
            fb = -1;
            fbp = nullptr;
            return;
        }

        width = vinfo.xres < (uint32_t)draw.styles.screen.w ? vinfo.xres : draw.styles.screen.w;
        height = vinfo.yres < (uint32_t)draw.styles.screen.h ? vinfo.yres : draw.styles.screen.h;

        logDebug("Framebuffer size: %dx%d (%dbpp), buffer size: %dx%d", vinfo.xres, vinfo.yres, vinfo.bits_per_pixel, width, height);
    }

    void render()
    {
        if (!fbp) return;

        if (vinfo.bits_per_pixel == 16) {
            for (int y = 0; y < height; y++) {
                for (int x = 0; x < width; x++) {
                    int location = y * finfo.line_length + x * 2;
                    Color color = draw.screenBuffer[y][x];
                    uint16_t rgb565 = ((color.r & 0xF8) << 8) | ((color.g & 0xFC) << 3) | (color.b >> 3);
                    *((uint16_t*)(fbp + location)) = rgb565;
                }
            }
        } else if (vinfo.bits_per_pixel == 32) {
            for (int y = 0; y < height; y++) {
                for (int x = 0; x < width; x++) {
                    int location = y * finfo.line_length + x * 4;
                    Color color = draw.screenBuffer[y][x];
                    uint32_t pixelVal = 0;
                    if (vinfo.red.length > 0) {
                        pixelVal = ((color.r >> (8 - vinfo.red.length)) << vinfo.red.offset)
                                 | ((color.g >> (8 - vinfo.green.length)) << vinfo.green.offset)
                                 | ((color.b >> (8 - vinfo.blue.length)) << vinfo.blue.offset);
                    } else {
                        // Fallback ARGB8888
                        pixelVal = (color.r << 16) | (color.g << 8) | color.b;
                    }
                    *((uint32_t*)(fbp + location)) = pixelVal;
                }
            }
        } else if (vinfo.bits_per_pixel == 24) {
            for (int y = 0; y < height; y++) {
                for (int x = 0; x < width; x++) {
                    int location = y * finfo.line_length + x * 3;
                    Color color = draw.screenBuffer[y][x];
                    if (vinfo.red.offset == 0) {
                        fbp[location]     = color.r;
                        fbp[location + 1] = color.g;
                        fbp[location + 2] = color.b;
                    } else {
                        fbp[location]     = color.b;
                        fbp[location + 1] = color.g;
                        fbp[location + 2] = color.r;
                    }
                }
            }
        } else {
            // Default 16-bit RGB565 fallback for unusual bpp
            for (int y = 0; y < height; y++) {
                for (int x = 0; x < width; x++) {
                    int location = y * finfo.line_length + x * (vinfo.bits_per_pixel / 8);
                    Color color = draw.screenBuffer[y][x];
                    uint16_t rgb565 = ((color.r & 0xF8) << 8) | ((color.g & 0xFC) << 3) | (color.b >> 3);
                    *((uint16_t*)(fbp + location)) = rgb565;
                }
            }
        }
    }
};
