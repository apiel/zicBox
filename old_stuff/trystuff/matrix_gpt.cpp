/** Description:
This C++ program functions as a dedicated scanner for a small external button pad or keypad, often referred to as a "matrix." It is designed to continuously monitor the physical state of these buttons and report which ones are being pressed.

The core idea is to interact with the device’s physical connection points, known as GPIO (General Purpose Input/Output) pins. The code sets up a small grid structure:
1.  **Row Pins** are configured as "outputs" (senders).
2.  **Column Pins** are configured as "inputs" (receivers).

During the **Setup Phase**, the program initializes these six pins (2 rows and 3 columns) to prepare them for communication.

In the **Reading Phase**, the program systematically scans the grid. It activates one row pin at a time and then instantly checks the status of all column pins. If a button connecting that specific row and column is pressed, the electrical circuit is completed, and the program records a change in the signal.

The results are organized into a 2x3 table, or matrix, where each position corresponds to a physical button. This table of values (representing "pressed" or "not pressed") is then printed to the screen. The entire process repeats automatically every 100 milliseconds, creating a rapid, continuous monitor of the physical device state.

sha: a04f1e5559e52799196a23e4180d43eb714bafb2f20b1bd487e7ef5c0a908926 
*/
// g++ -o matrix matrix.cpp -lrt

#include "../helpers/gpio.h"

#include <iostream>
#include <vector>
#include <thread>
#include <chrono>

const uint8_t rowPins[] = { 5, 26 };
const uint8_t colPins[] = { 6, 13, 19 };
const uint8_t numRows = 2;
const uint8_t numCols = 3;

void setupPins() {
    for (uint8_t i = 0; i < numRows; i++) {
        gpioSetMode(rowPins[i], GPIO_OUTPUT);
        clearGpio(rowPins[i]);
    }
    for (uint8_t j = 0; j < numCols; j++) {
        gpioSetMode(colPins[j], GPIO_INPUT);
        gpioSetPullUp(colPins[j]);
    }
}

void readMatrix() {
    std::vector<std::vector<uint8_t>> matrix(numRows, std::vector<uint8_t>(numCols, 0));

    for (uint8_t i = 0; i < numRows; i++) {
        setGpio(rowPins[i]);
        for (uint8_t j = 0; j < numCols; j++) {
            matrix[i][j] = gpioRead(colPins[j]);
        }
        clearGpio(rowPins[i]);
    }

    for (uint8_t i = 0; i < numRows; i++) {
        for (uint8_t j = 0; j < numCols; j++) {
            std::cout << (int)matrix[i][j] << " ";
        }
        std::cout << "  ";
    }
    std::cout << std::endl;
}

int main() {
    if (initGpio() < 0) {
        std::cerr << "Failed to initialize GPIO" << std::endl;
        return -1;
    }

    setupPins();

    while (true) {
        readMatrix();
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }

    return 0;
}
