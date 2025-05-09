// g++ fb.cpp -o fb

#include <fcntl.h>
#include <linux/fb.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <cstdint>

int main() {
    int fb = open("/dev/fb0", O_RDWR);
    if (fb == -1) {
        perror("Error opening framebuffer");
        return 1;
    }

    struct fb_var_screeninfo vinfo;
    struct fb_fix_screeninfo finfo;

    ioctl(fb, FBIOGET_FSCREENINFO, &finfo);
    ioctl(fb, FBIOGET_VSCREENINFO, &vinfo);

    long screensize = vinfo.yres_virtual * finfo.line_length;

    uint8_t* fbp = (uint8_t*) mmap(0, screensize, PROT_READ | PROT_WRITE, MAP_SHARED, fb, 0);
    if ((intptr_t)fbp == -1) {
        perror("Error mapping framebuffer");
        close(fb);
        return 1;
    }

    // Fill with solid red (assuming 16bpp RGB565)
    for (int y = 0; y < vinfo.yres; y++) {
        for (int x = 0; x < vinfo.xres; x++) {
            int location = y * finfo.line_length + x * (vinfo.bits_per_pixel / 8);
            // Red in RGB565 is 0xF800
            *((uint16_t*)(fbp + location)) = 0xF809;
        }
    }

    munmap(fbp, screensize);
    close(fb);
    return 0;
}
