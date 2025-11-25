/** Description:
This C++ program is designed as an embedded systems application for interacting directly with hardware, specifically focusing on reading the input from a small electronic button grid, known as a matrix keypad (likely a 2x3 setup).

**How the Program Works:**

1.  **Preparation (Setup):** The program first establishes communication with the computer’s physical input/output connections (GPIO pins). It divides the wires into two roles: "row" wires, which are set as outputs to send control signals, and "column" wires, which are set as inputs to receive readings. This configuration is essential for monitoring multiple buttons using the fewest possible wires.

2.  **The Scanning Process:** The main task involves a highly structured check. The program systematically activates one "row" wire at a time. While that single row is active, it quickly monitors the status of all "column" wires. If a button pressed at the intersection of the active row and a column completes an electrical circuit, the program instantly detects a change in the electrical signal on that column wire.

3.  **Continuous Monitoring:** After the initial setup, the program enters an endless loop, continuously repeating the entire scanning process multiple times per second. This ensures that any button press is detected immediately.

4.  **Reporting:** For every scan, the program prints the status of the entire button grid to the console. A "0" is outputted to indicate a detected button press, while a "1" signifies that the button is currently released, providing a real-time map of the keypad's state.

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
