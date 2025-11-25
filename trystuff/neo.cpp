/** Description:
This C++ program serves as a diagnostic tool and demonstration for interacting with a NeoTrellis device, which is a physical interface featuring a grid of buttons and multi-color LEDs.

The code's primary purpose is to establish communication, confirm hardware functionality, and continuously monitor user input.

**How the Code Works:**

1.  **Initialization and Callback:** The program first sets up the connection to the NeoTrellis device. It defines a critical instruction, known as a "callback," which specifies what should happen the instant a button is pressed or released. In this case, the instruction is to print the button number and its current status (pressed or released) to the screen.

2.  **Key Activation:** It then configures every button on the grid to actively detect both the moment a key is pressed down and the moment it is let go.

3.  **Visual Demonstration:** Before listening for user input, the program executes a brief visual test. It loops through all the buttons on the Trellis, sequentially lighting each one up in a bright purple color, creating a rapid visual wave. It then performs the same cycle, turning them all off. This confirms that the LED control system is working correctly.

4.  **Continuous Monitoring:** Finally, the application enters an endless loop where it constantly polls the device. This repeated checking ensures that the program can immediately detect and respond to any button press or release by executing the initial printing instruction defined during setup.

sha: e52e7dee3d3cb2e2cae6835246c83bf2af8861c389867b4e6ff21fb07293f891 
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
