// Adafruit_NeoTrellis t_array[] = {Adafruit_NeoTrellis(0x2E)};

// pass this matrix to the Adafruit_M4MultiTrellis_8x8 object

#include <Adafruit_Keypad.h>
#include <Adafruit_NeoPixel.h>
#include <Adafruit_NeoPixel_ZeroDMA.h>
#include <Adafruit_NeoTrellis.h>
#include <Adafruit_NeoTrellisM4.h>

Adafruit_NeoTrellis trellis;
Adafruit_NeoTrellisM4 trellisM4 = Adafruit_NeoTrellisM4();

uint32_t baseColors[] = {
    0xffffff,
    0x3761a1,
    0x3791a1,
    0x00ffef,
    0x00b300,
    0x9dfe86,
    0xff8d99,
    0xff0000,
    0xff0077,
    0xff7700,
    0xd09ff5,
    0xa19ffc,
    0x800080,
    0xa7194b,
    0xffff00,
    0x964B00,
};

const uint8_t baseColorCount = sizeof(baseColors) / sizeof(uint32_t);
const uint8_t colorCount = baseColorCount * 10;

const uint8_t colorOff = 254;

uint32_t colors[255] = {0};

const uint8_t keyCount = 48;
uint8_t button[keyCount] = {colorOff};
uint8_t buttonScale = 3;

uint32_t applyBrightness(uint32_t color, uint8_t brightness)
{
    uint8_t r = (uint8_t)(color >> 16), g = (uint8_t)(color >> 8), b = (uint8_t)color;
    if (brightness)
    {
        r = (r * brightness) >> 8;
        g = (g * brightness) >> 8;
        b = (b * brightness) >> 8;
    }
    // return (uint32_t)r << 16 | (uint32_t)g << 8 | b;
    return Adafruit_NeoPixel::Color(r, g, b);
}

void initColors()
{
    for (uint8_t i = 0; i < 255; i++)
    {
        colors[i] = 0;
    }

    for (uint8_t i = 0; i < keyCount; i++)
    {
        button[i] = colorOff;
    }

    const float brightnessRatio = 255.0 / 10.0;

    for (uint8_t i = 0; i < baseColorCount; i++)
    {
        for (uint8_t j = 0; j < 10; j++)
        {
            uint8_t brightness = (j % 10) * brightnessRatio + 5;
            colors[i * 10 + j] = applyBrightness(baseColors[i], brightness);
        }
    }

    colors[colorOff] = 0; // Let's ensure that 255 is 0
}

void setColor(uint8_t key, uint32_t color)
{
    uint8_t column = key % 12;
    uint8_t row = key / 12;
    if (column < 8)
    {
        trellisM4.setPixelColor(column + row * 8, color);
    }
    else
    {
        trellis.pixels.setPixelColor(column - 8 + row * 4, color);
        trellis.pixels.show();
    }
}

void onKeyPressed(uint8_t key)
{
    if (button[key] != colorOff)
    {
        setColor(key, colors[button[key] + buttonScale]);
    }

    Serial.write('$'); // 0x24
    Serial.write(key);
    Serial.write('\n');

    // Serial.printf("$%c\n", key);
}

void onKeyReleased(uint8_t key)
{
    Serial.write('!'); // 0x21
    Serial.write(key);
    Serial.write('\n');

    // Serial.printf("!%c\n", key);

    if (button[key] != colorOff)
    {
        setColor(key, colors[button[key]]);
    }
}

// define a callback for key presses
TrellisCallback callback(keyEvent evt)
{
    uint8_t key = evt.bit.NUM + (int)(evt.bit.NUM / 4.0 + 1) * 8;
    if (evt.bit.EDGE == SEESAW_KEYPAD_EDGE_RISING)
    {
        onKeyPressed(key);
    }
    else if (evt.bit.EDGE == SEESAW_KEYPAD_EDGE_FALLING)
    {
        onKeyReleased(key);
    }

    trellis.pixels.show();

    return 0;
}

void setup()
{
    Serial.begin(115200);

    trellisM4.begin();

    if (!trellis.begin())
    {
        // Serial.println("failed to start trellis");
    }

    // activate all keys and set callbacks
    for (int i = 0; i < NEO_TRELLIS_NUM_KEYS; i++)
    {
        trellis.activateKey(i, SEESAW_KEYPAD_EDGE_RISING);
        trellis.activateKey(i, SEESAW_KEYPAD_EDGE_FALLING);
        trellis.registerCallback(i, callback);
    }

    initColors();
}

unsigned long last = 0;
void loop()
{
    trellis.read();

    trellisM4.tick();
    while (trellisM4.available())
    {
        keypadEvent e = trellisM4.read();
        uint8_t key = e.bit.KEY + (int)(e.bit.KEY / 8.0) * 4;
        if (e.bit.EVENT == KEY_JUST_PRESSED)
        {
            onKeyPressed(key);
        }
        else if (e.bit.EVENT == KEY_JUST_RELEASED)
        {
            onKeyReleased(key);
        }
    }

    while (Serial.available())
    {
        uint8_t cmd = Serial.read();
        if (cmd == 0x23) // equivalent to '#' command to set color
        {
            uint8_t key = Serial.read();
            uint32_t color = colors[Serial.read()];
            // Serial.printf("key %d color: rgb #%x\n", key, color);
            setColor(key, color);
        }
        else if (cmd == 0x25) // equivalent to '%' command to enable button mode
        {
            uint8_t key = Serial.read();
            uint8_t colorIndex = Serial.read();
            button[key] = colorIndex;
            setColor(key, colors[colorIndex]);
        }
    }

    delay(10);

    // if (millis() - last > 2000)
    // {
    //     last = millis();
    //     Serial.printf("loop: %d\n", last);
    // }
}
