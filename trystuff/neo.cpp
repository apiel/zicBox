#include "Adafruit_NeoTrellis.h"

Adafruit_NeoTrellis trellis;

TrellisCallback blink(keyEvent evt)
{
    // Check is the pad pressed?
    if (evt.bit.EDGE == SEESAW_KEYPAD_EDGE_RISING) {
        // trellis.pixels.setPixelColor(evt.bit.NUM, 0xFF0000); //on rising
        std::cout << "Pressed: " << (int)evt.bit.NUM << std::endl;
    } else if (evt.bit.EDGE == SEESAW_KEYPAD_EDGE_FALLING) {
        // or is the pad released?
        // trellis.pixels.setPixelColor(evt.bit.NUM, 0); //off falling
        std::cout << "Released: " << (int)evt.bit.NUM << std::endl;
    }

    // Turn on/off the neopixels!
    // trellis.pixels.show();

    return 0;
}

uint32_t Wheel(byte WheelPos)
{
    // if (WheelPos < 85) {
    //     return trellis.pixels.Color(WheelPos * 3, 255 - WheelPos * 3, 0);
    // } else if (WheelPos < 170) {
    //     WheelPos -= 85;
    //     return trellis.pixels.Color(255 - WheelPos * 3, 0, WheelPos * 3);
    // } else {
    //     WheelPos -= 170;
    //     return trellis.pixels.Color(0, WheelPos * 3, 255 - WheelPos * 3);
    // }
    return 0;
}

int main()
{
    try {

        trellis.begin();

        //activate all keys and set callbacks
        for (int i = 0; i < NEO_TRELLIS_NUM_KEYS; i++) {
            trellis.activateKey(i, SEESAW_KEYPAD_EDGE_RISING);
            trellis.activateKey(i, SEESAW_KEYPAD_EDGE_FALLING);
            trellis.registerCallback(i, blink);
        }

        // //do a little animation to show we're on
        // for (uint16_t i = 0; i < trellis.pixels.numPixels(); i++) {
        //     trellis.pixels.setPixelColor(i, 0x00FF00);
        //     trellis.pixels.show();
        //     // delay(50);
        //     std::this_thread::sleep_for(std::chrono::milliseconds(50));
        // }
        // for (uint16_t i = 0; i < trellis.pixels.numPixels(); i++) {
        //     trellis.pixels.setPixelColor(i, 0x000000);
        //     trellis.pixels.show();
        //     // delay(50);
        //     std::this_thread::sleep_for(std::chrono::milliseconds(50));
        // }

        while (true) {
            trellis.read(); // interrupt management does all the work! :)
            std::this_thread::sleep_for(std::chrono::milliseconds(20)); // Polling rate (20ms recommended by Adafruit)
        }

    } catch (const std::exception& e) {
        std::cerr << "Application Error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}
