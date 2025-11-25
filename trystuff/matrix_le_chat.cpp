/** Description:
This C++ program is designed to monitor and read input from a physical hardware component, specifically a small matrix keypad or grid of buttons, likely connected to a single-board computer like a Raspberry Pi.

The primary goal is to determine which buttons are being pressed efficiently, using fewer connection points (GPIO pins) than the total number of buttons.

### How the Program Works

1.  **Setup:** The program first initializes the specialized connection points (GPIO pins) on the computer. It designates some pins as "Row Outputs" (to send electrical signals) and others as "Column Inputs" (to receive signals).

2.  **Matrix Scanning:** The core function uses a technique called matrix scanning. It systematically checks the entire button grid in a rapid cycle:
    *   It activates one "Row Output" at a time by briefly setting its voltage low.
    *   While that row is active, it quickly checks the status of all "Column Inputs."
    *   If a button is pressed, it bridges the gap between the active row and a specific column, allowing the input pin to detect the signal change.

3.  **Reporting:** This scanning process runs continuously. For every button in the 2x3 grid, the program prints its current status to the screen: typically displaying "0" if the button is currently pressed and "1" if it is not. This ensures the output reflects the real-time state of the keypad.

sha: 5327d518372f5b7549626659e5ebf35089fcbe1c557d3fa4c455e6e92213c24d 
*/
// g++ -o matrix matrix.cpp -lrt

#include <iostream>
#include <unistd.h> // for usleep

#include "../helpers/gpio.h"

// Define row and column pins
const uint8_t rowPins[] = { 5, 26 };
const uint8_t colPins[] = { 6, 13, 19 };

void setup()
{
    // Initialize GPIO
    if (initGpio() < 0) {
        std::cerr << "Failed to initialize GPIO" << std::endl;
        exit(1);
    }

    // Set row pins as outputs
    for (uint8_t pin : rowPins) {
        gpioSetMode(pin, GPIO_OUTPUT);
        gpioWrite(pin, 1); // Set rows high initially
    }

    // Set column pins as inputs with pull-ups
    for (uint8_t pin : colPins) {
        gpioSetMode(pin, GPIO_INPUT);
        gpioSetPullUp(pin);
    }
}

// void scanMatrix()
// {
//     for (int row = 0; row < 2; ++row) {
//         // std::cout << "------ Scanning row " << row + 1 << std::endl;
//         gpioWrite(rowPins[row], 0); // Set current row low
//         for (int col = 0; col < 3; ++col) {
//             if (gpioRead(colPins[col]) == 0) {
//                 // std::cout << "Button " << row * 3 + col + 1 << " pressed" << std::endl;
//                 std::cout << "0 ";
//                 // usleep(200000); // Debounce delay
//             } else {
//                 std::cout << "1 ";
//             }
//             usleep(100);
//         }
//         gpioWrite(rowPins[row], 1); // Set row high again
//     }
//     std::cout << std::endl;
// }

void scanMatrix() {
    // Set all rows high initially
    for (uint8_t pin : rowPins) {
        gpioWrite(pin, 1);
    }

    // Read the state of each column
    for (int row = 0; row < 2; ++row) {
        gpioWrite(rowPins[row], 0); // Set current row low
        for (int col = 0; col < 3; ++col) {
            if (gpioRead(colPins[col]) == 0) {
                std::cout << "0 ";
            } else {
                std::cout << "1 ";
            }
            usleep(100); // Short delay between reads
        }
        gpioWrite(rowPins[row], 1); // Set row high again
        std::cout << "  "; // Separator between rows
    }
    std::cout << std::endl;
}

int main()
{
    setup();
    while (true) {
        scanMatrix();
        usleep(100000); // Short delay between scans
    }
    return 0;
}
