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
