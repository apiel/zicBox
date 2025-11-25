/** Description:
This program is a specialized C++ application designed to monitor the status of a small electronic keypad or button grid, specifically configured as a 2x3 matrix, connected to the physical input/output pins (GPIO) of a computer or microcontroller.

### How the Program Works

**1. Setup and Initialization:**
The code begins by configuring the specific physical pins used for the rows and columns of the grid. It initializes the hardware system, ensuring these pins are ready to be used as either voltage drivers (outputs) or readers (inputs).

**2. Robust Matrix Scanning:**
The primary function of the program is to determine exactly which buttons in the 2x3 grid are currently pressed. It uses a clever technique known as matrix scanning. To check for a connection, the program sequentially applies voltage to one set of lines (e.g., Row 1) and listens on the perpendicular set of lines (the Columns). If voltage is detected on a column, it means the button at that intersection is pressed.

To ensure accuracy and prevent incorrect readings when multiple buttons are held down (a problem called "ghosting"), the program performs two full scans:
*   **Scan 1:** Rows are used as drivers (outputs), and columns are used as readers (inputs).
*   **Scan 2:** Columns are used as drivers (outputs), and rows are used as readers (inputs).

Only when both independent scans confirm a connection at a specific point is that button definitively registered as pressed.

**3. Continuous Monitoring and Output:**
The program runs in an endless loop. It continuously performs the dual scan, processes the combined results into a 2x3 status map (where '1' means pressed and '0' means released), and prints this map to the console in real-time. A short pause is included in the loop to stabilize readings and prevent the program from using too much processing power.

In essence, this code provides a robust, real-time interface for reading physical inputs from a structured electronic component.

sha: f5c3f6af33442646827392995944a57ee57a576cc0373686d6b2fd631182c5fc 
*/
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