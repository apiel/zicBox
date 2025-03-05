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

void scanMatrix()
{
    for (int row = 0; row < 2; ++row) {
        // std::cout << "------ Scanning row " << row + 1 << std::endl;
        gpioWrite(rowPins[row], 0); // Set current row low
        for (int col = 0; col < 3; ++col) {
            if (gpioRead(colPins[col]) == 0) {
                // std::cout << "Button " << row * 3 + col + 1 << " pressed" << std::endl;
                std::cout << "0 ";
                // usleep(200000); // Debounce delay
            } else {
                std::cout << "1 ";
            }
            usleep(100);
        }
        gpioWrite(rowPins[row], 1); // Set row high again
    }
    std::cout << std::endl;

    // for (int row = 0; row < 2; ++row) {
    //     std::cout << "------ Scanning row " << row + 1 << std::endl;
    //     gpioWrite(rowPins[row], 0); // Set current row low
    //     for (int col = 0; col < 3; ++col) {
    //         if (gpioRead(colPins[col]) == 0) {
    //             std::cout << "Button " << row * 3 + col + 1 << " pressed" << std::endl;
    //             usleep(200000); // Debounce delay
    //         }
    //     }
    //     gpioWrite(rowPins[row], 1); // Set row high again
    // }
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
