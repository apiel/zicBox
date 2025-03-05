// g++ -o matrix matrix.cpp -lrt

#include "../helpers/gpio.h"
#include <unistd.h> // for usleep

const uint8_t rowPins[] = {5, 26};
const uint8_t colPins[] = {6, 13, 19};
const uint8_t numRows = sizeof(rowPins) / sizeof(rowPins[0]);
const uint8_t numCols = sizeof(colPins) / sizeof(colPins[0]);

void setupMatrix() {
    // Initialize GPIO
    if (initGpio() < 0) {
        fprintf(stderr, "Failed to initialize GPIO\n");
        exit(1);
    }
}

void scanMatrix(uint8_t matrix[numRows][numCols], bool rowsAsOutput) {
    if (rowsAsOutput) {
        // Set rows as output and columns as input
        for (uint8_t i = 0; i < numRows; i++) {
            gpioSetMode(rowPins[i], GPIO_OUTPUT);
            gpioWrite(rowPins[i], 0); // Set rows low initially
        }
        for (uint8_t i = 0; i < numCols; i++) {
            gpioSetMode(colPins[i], GPIO_INPUT);
            gpioSetPullUp(colPins[i]); // Enable pull-up resistors
        }

        // Scan rows
        for (uint8_t row = 0; row < numRows; row++) {
            gpioWrite(rowPins[row], 1); // Set current row high
            usleep(10); // Wait for signal to stabilize

            // Read columns
            for (uint8_t col = 0; col < numCols; col++) {
                matrix[row][col] = !gpioRead(colPins[col]); // Invert logic (pull-up resistors)
            }

            gpioWrite(rowPins[row], 0); // Set current row low
            usleep(10); // Wait for signal to stabilize
        }
    } else {
        // Set columns as output and rows as input
        for (uint8_t i = 0; i < numCols; i++) {
            gpioSetMode(colPins[i], GPIO_OUTPUT);
            gpioWrite(colPins[i], 0); // Set columns low initially
        }
        for (uint8_t i = 0; i < numRows; i++) {
            gpioSetMode(rowPins[i], GPIO_INPUT);
            gpioSetPullUp(rowPins[i]); // Enable pull-up resistors
        }

        // Scan columns
        for (uint8_t col = 0; col < numCols; col++) {
            gpioWrite(colPins[col], 1); // Set current column high
            usleep(10); // Wait for signal to stabilize

            // Read rows
            for (uint8_t row = 0; row < numRows; row++) {
                matrix[row][col] = !gpioRead(rowPins[row]); // Invert logic (pull-up resistors)
            }

            gpioWrite(colPins[col], 0); // Set current column low
            usleep(10); // Wait for signal to stabilize
        }
    }
}

void readMatrix(uint8_t matrix[numRows][numCols]) {
    uint8_t scan1[numRows][numCols] = {0};
    uint8_t scan2[numRows][numCols] = {0};

    // First scan: rows as output, columns as input
    scanMatrix(scan1, true);

    // Second scan: columns as output, rows as input
    scanMatrix(scan2, false);

    // Combine results
    for (uint8_t row = 0; row < numRows; row++) {
        for (uint8_t col = 0; col < numCols; col++) {
            matrix[row][col] = scan1[row][col] && scan2[row][col];
        }
    }
}

void printMatrix(uint8_t matrix[numRows][numCols]) {
    for (uint8_t row = 0; row < numRows; row++) {
        for (uint8_t col = 0; col < numCols; col++) {
            printf("%d ", matrix[row][col]);
        }
        printf("  ");
    }
    printf("\n");
}

int main() {
    uint8_t matrix[numRows][numCols] = {0};

    setupMatrix();

    while (1) {
        readMatrix(matrix);
        printMatrix(matrix);
        usleep(100000); // Sleep for 100ms to debounce and reduce CPU usage
    }

    return 0;
}