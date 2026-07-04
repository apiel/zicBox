/** Description:
This program acts as a sophisticated driver for a specific piece of hardware known as a NeoTrellis, which is typically a grid of buttons, each equipped with its own LED light.

The core function of the code is to initialize and monitor this device.

**How the Code Works:**

1.  **Setup and Listener:** The program first connects to the NeoTrellis hardware. It immediately defines a special instruction, called a "callback," that runs whenever a button is pressed or released. This instruction simply prints a message to the computer screen, confirming the key number and whether the action was "Pressed" or "Released."
2.  **Activation:** The code then activates all available buttons on the Trellis to listen for two distinct events: when the button is pushed down and when the finger is lifted up.
3.  **Visual Confirmation:** A short light show is executed to confirm the LEDs are working. The program sequentially lights up every key in purple, waits a moment, and then sequentially turns them all off.
4.  **Continuous Monitoring:** Finally, the program enters an endless loop where it constantly “reads” the device, checking for any user interaction. This continuous listening process ensures that the defined key-press instruction is instantly executed whenever a user touches the Trellis.

If any problem occurs during the hardware setup or operation, the program will safely catch the error and display a descriptive message.

sha: 669764214ae1e85e4fbdc8bba1ad803ec9b1d4d05aab3681f789ad1e479a8767 
*/
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
