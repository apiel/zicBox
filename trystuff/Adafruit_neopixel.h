#pragma once

#include "Adafruit_seesaw.h"

// RGB NeoPixel permutations; white and red offsets are always same
// Offset:         W          R          G          B
#define NEO_RGB ((0 << 6) | (0 << 4) | (1 << 2) | (2))
#define NEO_RBG ((0 << 6) | (0 << 4) | (2 << 2) | (1))
#define NEO_GRB ((1 << 6) | (1 << 4) | (0 << 2) | (2))
#define NEO_GBR ((2 << 6) | (2 << 4) | (0 << 2) | (1))
#define NEO_BRG ((1 << 6) | (1 << 4) | (2 << 2) | (0))
#define NEO_BGR ((2 << 6) | (2 << 4) | (1 << 2) | (0))

// RGBW NeoPixel permutations; all 4 offsets are distinct
// Offset:         W          R          G          B
#define NEO_WRGB ((0 << 6) | (1 << 4) | (2 << 2) | (3))
#define NEO_WRBG ((0 << 6) | (1 << 4) | (3 << 2) | (2))
#define NEO_WGRB ((0 << 6) | (2 << 4) | (1 << 2) | (3))
#define NEO_WGBR ((0 << 6) | (3 << 4) | (1 << 2) | (2))
#define NEO_WBRG ((0 << 6) | (2 << 4) | (3 << 2) | (1))
#define NEO_WBGR ((0 << 6) | (3 << 4) | (2 << 2) | (1))

#define NEO_RWGB ((1 << 6) | (0 << 4) | (2 << 2) | (3))
#define NEO_RWBG ((1 << 6) | (0 << 4) | (3 << 2) | (2))
#define NEO_RGWB ((2 << 6) | (0 << 4) | (1 << 2) | (3))
#define NEO_RGBW ((3 << 6) | (0 << 4) | (1 << 2) | (2))
#define NEO_RBWG ((2 << 6) | (0 << 4) | (3 << 2) | (1))
#define NEO_RBGW ((3 << 6) | (0 << 4) | (2 << 2) | (1))

#define NEO_GWRB ((1 << 6) | (2 << 4) | (0 << 2) | (3))
#define NEO_GWBR ((1 << 6) | (3 << 4) | (0 << 2) | (2))
#define NEO_GRWB ((2 << 6) | (1 << 4) | (0 << 2) | (3))
#define NEO_GRBW ((3 << 6) | (1 << 4) | (0 << 2) | (2))
#define NEO_GBWR ((2 << 6) | (3 << 4) | (0 << 2) | (1))
#define NEO_GBRW ((3 << 6) | (2 << 4) | (0 << 2) | (1))

#define NEO_BWRG ((1 << 6) | (2 << 4) | (3 << 2) | (0))
#define NEO_BWGR ((1 << 6) | (3 << 4) | (2 << 2) | (0))
#define NEO_BRWG ((2 << 6) | (1 << 4) | (3 << 2) | (0))
#define NEO_BRGW ((3 << 6) | (1 << 4) | (2 << 2) | (0))
#define NEO_BGWR ((2 << 6) | (3 << 4) | (1 << 2) | (0))
#define NEO_BGRW ((3 << 6) | (2 << 4) | (1 << 2) | (0))

// If 400 KHz support is enabled, the third parameter to the constructor
// requires a 16-bit value (in order to select 400 vs 800 KHz speed).
// If only 800 KHz is enabled (as is default on ATtiny), an 8-bit value
// is sufficient to encode pixel color order, saving some space.

#define NEO_KHZ800 0x0000 // 800 KHz datastream
#define NEO_KHZ400 0x0100 // 400 KHz datastream

typedef uint16_t neoPixelType;

/** Adafruit_NeoPixel-compatible 'wrapper' for LED control over seesaw
 */
class Adafruit_neopixel : public Adafruit_seesaw {

public:
    Adafruit_neopixel(uint16_t n, uint8_t p, neoPixelType t)
        : Adafruit_seesaw()
        , begun(false)
        , numLEDs(n)
        , pin(p)
        , brightness(0)
        , pixels(NULL)
        , endTime(0)
        , type(t)
    {
    }

    Adafruit_neopixel()
        : Adafruit_seesaw()
        ,
#ifdef NEO_KHZ400
        is800KHz(true)
        ,
#endif
        begun(false)
        , numLEDs(0)
        , numBytes(0)
        , pin(-1)
        , brightness(0)
        , pixels(NULL)
        , rOffset(1)
        , gOffset(0)
        , bOffset(2)
        , wOffset(1)
        , endTime(0)
    {
    }

    ~Adafruit_neopixel()
    {
        if (pixels)
            free(pixels);
    }

    void begin(const char* i2c_device = "/dev/i2c-1", uint8_t addr = 0x2E)
    {
        Adafruit_seesaw::begin(i2c_device, addr);
        // updateType(type);
        updateLength(numLEDs);
        setPin(pin);
    }

    void updateLength(uint16_t n)
    {
        if (pixels)
            free(pixels); // Free existing data (if any)

        // Allocate new data -- note: ALL PIXELS ARE CLEARED
        numBytes = n * ((wOffset == rOffset) ? 3 : 4);
        if ((pixels = (uint8_t*)malloc(numBytes))) {
            memset(pixels, 0, numBytes);
            numLEDs = n;
        } else {
            numLEDs = numBytes = 0;
        }

        uint8_t buf[] = { (uint8_t)(numBytes >> 8), (uint8_t)(numBytes & 0xFF) };
        writeReg(SEESAW_NEOPIXEL_BASE, SEESAW_NEOPIXEL_BUF_LENGTH, buf, 2);
    }

    void show(void)
    {

        if (!pixels)
            return;

        // Data latch = 300+ microsecond pause in the output stream.  Rather than
        // put a delay at the end of the function, the ending time is noted and
        // the function will simply hold off (if needed) on issuing the
        // subsequent round of data until the latch time has elapsed.  This
        // allows the mainline code to start generating the next frame of data
        // rather than stalling for the latch.
        // while (!canShow())
        //     ;

        this->writeReg(SEESAW_NEOPIXEL_BASE, SEESAW_NEOPIXEL_SHOW, NULL, 0);

        // endTime = micros(); // Save EOD time for latch on next call
    }

    void setPin(uint8_t p)
    {
        this->write8(SEESAW_NEOPIXEL_BASE, SEESAW_NEOPIXEL_PIN, p);
        pin = p;
    }

    void setPixelColor(uint16_t n, uint8_t r, uint8_t g, uint8_t b)
    {
        if (n < numLEDs) {
            if (brightness) { // See notes in setBrightness()
                r = (r * brightness) >> 8;
                g = (g * brightness) >> 8;
                b = (b * brightness) >> 8;
            }
            uint8_t* p;
            if (wOffset == rOffset) { // Is an RGB-type strip
                p = &pixels[n * 3]; // 3 bytes per pixel
            } else { // Is a WRGB-type strip
                p = &pixels[n * 4]; // 4 bytes per pixel
                p[wOffset] = 0; // But only R,G,B passed -- set W to 0
            }
            p[rOffset] = r; // R,G,B always stored
            p[gOffset] = g;
            p[bOffset] = b;

            uint8_t len = (wOffset == rOffset ? 3 : 4);
            uint16_t offset = n * len;

            uint8_t writeBuf[6];
            writeBuf[0] = (offset >> 8);
            writeBuf[1] = offset;
            memcpy(&writeBuf[2], p, len);

            this->writeReg(SEESAW_NEOPIXEL_BASE, SEESAW_NEOPIXEL_BUF, writeBuf, len + 2);
        }
    }

    void setPixelColor(uint16_t n, uint8_t r, uint8_t g, uint8_t b, uint8_t w)
    {
        if (n < numLEDs) {
            if (brightness) { // See notes in setBrightness()
                r = (r * brightness) >> 8;
                g = (g * brightness) >> 8;
                b = (b * brightness) >> 8;
                w = (w * brightness) >> 8;
            }
            uint8_t* p;
            if (wOffset == rOffset) { // Is an RGB-type strip
                p = &pixels[n * 3]; // 3 bytes per pixel (ignore W)
            } else { // Is a WRGB-type strip
                p = &pixels[n * 4]; // 4 bytes per pixel
                p[wOffset] = w; // Store W
            }
            p[rOffset] = r; // Store R,G,B
            p[gOffset] = g;
            p[bOffset] = b;

            uint8_t len = (wOffset == rOffset ? 3 : 4);
            uint16_t offset = n * len;

            uint8_t writeBuf[6];
            writeBuf[0] = (offset >> 8);
            writeBuf[1] = offset;
            memcpy(&writeBuf[2], p, len);

            this->writeReg(SEESAW_NEOPIXEL_BASE, SEESAW_NEOPIXEL_BUF, writeBuf, len + 2);
        }
    }

    void setPixelColor(uint16_t n, uint32_t c)
    {
        if (n < numLEDs) {
            uint8_t *p, r = (uint8_t)(c >> 16), g = (uint8_t)(c >> 8), b = (uint8_t)c;
            if (brightness) { // See notes in setBrightness()
                r = (r * brightness) >> 8;
                g = (g * brightness) >> 8;
                b = (b * brightness) >> 8;
            }
            if (wOffset == rOffset) {
                p = &pixels[n * 3];
            } else {
                p = &pixels[n * 4];
                uint8_t w = (uint8_t)(c >> 24);
                p[wOffset] = brightness ? ((w * brightness) >> 8) : w;
            }
            p[rOffset] = r;
            p[gOffset] = g;
            p[bOffset] = b;

            uint8_t len = (wOffset == rOffset ? 3 : 4);
            uint16_t offset = n * len;

            uint8_t writeBuf[6];
            writeBuf[0] = (offset >> 8);
            writeBuf[1] = offset;
            memcpy(&writeBuf[2], p, len);

            this->writeReg(SEESAW_NEOPIXEL_BASE, SEESAW_NEOPIXEL_BUF, writeBuf, len + 2);
        }
    }

    void updateType(neoPixelType t)
    {
        bool oldThreeBytesPerPixel = (wOffset == rOffset); // false if RGBW

        wOffset = (t >> 6) & 0b11; // See notes in header file
        rOffset = (t >> 4) & 0b11; // regarding R/G/B/W offsets
        gOffset = (t >> 2) & 0b11;
        bOffset = t & 0b11;
        is800KHz = (t < 256); // 400 KHz flag is 1<<8

        this->write8(SEESAW_NEOPIXEL_BASE, SEESAW_NEOPIXEL_SPEED, is800KHz);

        // If bytes-per-pixel has changed (and pixel data was previously
        // allocated), re-allocate to new size.  Will clear any data.
        if (pixels) {
            bool newThreeBytesPerPixel = (wOffset == rOffset);
            if (newThreeBytesPerPixel != oldThreeBytesPerPixel)
                updateLength(numLEDs);
        }
    }

    static uint32_t Color(uint8_t r, uint8_t g, uint8_t b)
    {
        return ((uint32_t)r << 16) | ((uint32_t)g << 8) | b;
    }

    static uint32_t Color(uint8_t r, uint8_t g, uint8_t b, uint8_t w)
    {
        return ((uint32_t)w << 24) | ((uint32_t)r << 16) | ((uint32_t)g << 8) | b;
    }

    uint32_t getPixelColor(uint16_t n) const
    {
        if (n >= numLEDs)
            return 0; // Out of bounds, return no color.

        uint8_t* p;

        if (wOffset == rOffset) { // Is RGB-type device
            p = &pixels[n * 3];
            if (brightness) {
                // Stored color was decimated by setBrightness().  Returned value
                // attempts to scale back to an approximation of the original 24-bit
                // value used when setting the pixel color, but there will always be
                // some error -- those bits are simply gone.  Issue is most
                // pronounced at low brightness levels.
                return (((uint32_t)(p[rOffset] << 8) / brightness) << 16) | (((uint32_t)(p[gOffset] << 8) / brightness) << 8) | ((uint32_t)(p[bOffset] << 8) / brightness);
            } else {
                // No brightness adjustment has been made -- return 'raw' color
                return ((uint32_t)p[rOffset] << 16) | ((uint32_t)p[gOffset] << 8) | (uint32_t)p[bOffset];
            }
        } else { // Is RGBW-type device
            p = &pixels[n * 4];
            if (brightness) { // Return scaled color
                return (((uint32_t)(p[wOffset] << 8) / brightness) << 24) | (((uint32_t)(p[rOffset] << 8) / brightness) << 16) | (((uint32_t)(p[gOffset] << 8) / brightness) << 8) | ((uint32_t)(p[bOffset] << 8) / brightness);
            } else { // Return raw color
                return ((uint32_t)p[wOffset] << 24) | ((uint32_t)p[rOffset] << 16) | ((uint32_t)p[gOffset] << 8) | (uint32_t)p[bOffset];
            }
        }
    }

    uint8_t* getPixels(void) const { return pixels; }
    // uint8_t* getBrightness(void) const;
    int8_t getPin(void) { return pin; };
    uint16_t numPixels(void) const { return numLEDs; }

    void setBrightness(uint8_t b) { brightness = b; }
    void clear()
    {
        // Clear local pixel buffer
        memset(pixels, 0, numBytes);

        // Now clear the pixels on the seesaw
        uint8_t writeBuf[32];
        memset(writeBuf, 0, 32);
        for (uint8_t offset = 0; offset < numBytes; offset += 32 - 4) {
            writeBuf[0] = (offset >> 8);
            writeBuf[1] = offset;
            this->writeReg(SEESAW_NEOPIXEL_BASE, SEESAW_NEOPIXEL_BUF, writeBuf, 32);
        }
    }
    // inline bool canShow(void) { return (micros() - endTime) >= 300L; }

protected:
    bool is800KHz, // ...true if 800 KHz pixels
        begun; // true if begin() previously called
    uint16_t numLEDs, // Number of RGB LEDs in strip
        numBytes; // Size of 'pixels' buffer below (3 or 4 bytes/pixel)
    int8_t pin;
    uint8_t brightness,
        *pixels, // Holds LED color values (3 or 4 bytes each)
        rOffset, // Index of red byte within each 3- or 4-byte pixel
        gOffset, // Index of green byte
        bOffset, // Index of blue byte
        wOffset; // Index of white byte (same as rOffset if no white)
    uint32_t endTime; // Latch timing reference

    uint16_t type;
};
