

#include "../helpers/NeoTrellis.h"

int main()
{
    try {
        NeoTrellis trellis([](uint8_t num, bool pressed) {
            std::cout << (pressed ? "Pressed: " : "Released: ") << (int)num << std::endl;
        });

        trellis.begin();

        //activate all keys and set callbacks
        for (int i = 0; i < NEO_TRELLIS_NUM_KEYS; i++) {
            trellis.activateKey(i, NeoTrellis::SEESAW_KEYPAD_EDGE_RISING);
            trellis.activateKey(i, NeoTrellis::SEESAW_KEYPAD_EDGE_FALLING);
            // trellis.registerCallback(i, blink);
        }

        // trellis.setGlobalBrightness(0.5f);

        const NeoTrellis::Color OFF(0, 0, 0);
        const NeoTrellis::Color PURPLE(180, 0, 255);

        std::cout << "Starting LED cycle (Purple then Off)..." << std::endl;
        for (int i = 0; i < NEO_TRELLIS_NUM_KEYS; i++) {
            trellis.setPixelColor(i, PURPLE);
            trellis.show(); // Latch the colors to the LEDs
            std::this_thread::sleep_for(std::chrono::milliseconds(5));
        }

        for (int i = 0; i < NEO_TRELLIS_NUM_KEYS; i++) {
            trellis.setPixelColor(i, OFF);
            trellis.show(); // Latch the colors to the LEDs
            std::this_thread::sleep_for(std::chrono::milliseconds(5));
        }
        std::cout << "LED cycle complete." << std::endl;

        while (true) {
            trellis.read();
            std::this_thread::sleep_for(std::chrono::milliseconds(20));
        }

    } catch (const std::exception& e) {
        std::cerr << "Application Error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}
