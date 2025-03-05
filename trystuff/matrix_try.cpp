// g++ -o matrix matrix.cpp -lrt

#include <iostream>
#include <unistd.h> // for usleep

#include "../helpers/gpio.h"

// Define row and column pins
const uint8_t rowPins[] = { 5, 26 };
const uint8_t colPins[] = { 6, 13, 19 };

void setRowAsOutput()
{
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

void setColsAsOutput() {
    // Set col pins as outputs
    for (uint8_t pin : colPins) {
        gpioSetMode(pin, GPIO_OUTPUT);
        gpioWrite(pin, 1); // Set columns high initially
    }

    // Set row pins as inputs with pull-ups
    for (uint8_t pin : rowPins) {
        gpioSetMode(pin, GPIO_INPUT);
        gpioSetPullUp(pin);
    }
}

bool btn[6] = {0, 0, 0, 0, 0, 0};

void scanRowMatrix() {
    // Set all rows high initially
    for (uint8_t pin : rowPins) {
        gpioWrite(pin, 1);
    }

    std::cout << "rows: ";
    // Read the state of each column
    for (int row = 0; row < 2; ++row) {
        // gpioWrite(rowPins[row], 0); // Set current row low
        for (int col = 0; col < 3; ++col) {
            if (gpioRead(colPins[col]) == 0) {
                // std::cout << "0 ";
            } else {
                // std::cout << "1 ";
                btn[row * 3 + col] = true;
            }
            usleep(100); // Short delay between reads
        }
        gpioWrite(rowPins[row], 1); // Set row high again
    }
    // std::cout << std::endl;
}

void scanColMatrix() {
    // Set all columns high initially
    for (uint8_t pin : colPins) {
        gpioWrite(pin, 1);
    }

    // std::cout << "cols: ";
    // Read the state of each row
    for (int col = 0; col < 3; ++col) {
        gpioWrite(colPins[col], 0); // Set current column low
        for (int row = 0; row < 2; ++row) {
            if (gpioRead(rowPins[row]) == 0) {
                // std::cout << "0 ";
            } else {
                // std::cout << "1 ";
                btn[row * 3 + col] = true;
            }
            usleep(100); // Short delay between reads
        }
        gpioWrite(colPins[col], 1); // Set column high again
    }
    // std::cout << std::endl;
}

int main()
{
        // Initialize GPIO
        if (initGpio() < 0) {
            std::cerr << "Failed to initialize GPIO" << std::endl;
            exit(1);
        }

    while (true) {
        btn[0] = false;
        btn[1] = false;
        btn[2] = false;
        btn[3] = false;
        btn[4] = false;
        btn[5] = false;

        setRowAsOutput();
        scanRowMatrix();
        setColsAsOutput();
        scanColMatrix();

        std::cout << btn[0] << " " << btn[1] << " " << btn[2] << " " << btn[3] << " " << btn[4] << " " << btn[5] << std::endl;

        usleep(100000); // Short delay between scans
    }
    return 0;
}
