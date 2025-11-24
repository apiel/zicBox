/** Description:
This C++ program serves as a low-level tool for direct screen manipulation on a Linux system, completely bypassing standard operating system graphics environments. It directly interacts with the digital screen memory, which Linux calls the “framebuffer.”

The program’s core function is to paint the entire screen a single, solid color.

Here is how it works:

1.  **Access and Information Gathering:** The program first opens the framebuffer device, treating it like a specialized file. It then sends specific commands to the device to retrieve detailed technical information, such as the exact screen resolution (width and height) and the layout of the color data (how many bits represent each pixel).

2.  **Memory Mapping:** The most important step is setting up “memory mapping.” This technique allows the program to treat the actual physical memory used by the screen hardware as if it were a normal block of data inside the running program. This grants extremely fast, direct read and write access to every single pixel.

3.  **Drawing:** Using the gathered resolution data, the program systematically loops through every pixel location on the screen. For each pixel, it calculates its exact position in the mapped memory and writes a pre-determined numerical value corresponding to a specific shade of red.

The result of this direct manipulation is that the entire display turns solid red. Finally, the program cleans up by releasing the memory access and closing the framebuffer connection.

sha: 9584e5dbfe341606100b39a12ea9c7102da3b9d425917eae8c49e8ace10bdabe 
*/
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
