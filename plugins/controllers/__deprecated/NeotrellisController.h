/** Description:
This software component, the `NeotrellisController`, serves as a specialized communications driver for the Adafruit Neotrellis M4 device, which functions as a grid of illuminated buttons.

Its primary role is to act as a bridge, translating physical interactions on the keypad into digital signals for an application, and vice versa.

### How It Works

1.  **Serial Connection:** The controller establishes a direct data link—typically using a USB cable that simulates a serial port. The code opens this connection and meticulously configures all the technical rules for communication (like speed, data size, and error handling) to ensure accurate and stable transfer between the computer and the device.

2.  **Background Listener:** To detect input instantly, the controller starts a dedicated listener that runs continuously in the background. This listener constantly monitors the serial connection. When a button is pressed or released on the Neotrellis, the device sends coded messages. The listener captures these messages and alerts the main application immediately. Key presses and releases are identified by specific starting characters followed by the unique ID of the key.

3.  **Visual Control:** The controller also allows the application to send data back to the device. Functions are included that transmit commands to change the colors of the individual keys or buttons, enabling the software to provide dynamic visual feedback to the user.

In summary, this component manages all low-level serial communication, dedicating resources to constantly monitor user input while providing methods to control the visual output of the physical keypad. When its job is done, it safely closes the connection and stops the background listener.

sha: 27e4337d0c990e7cc2fd02bd4401cb813b072ab28497870e02a61530e79ad47a 
*/
#ifndef _NEO_CONTROLLER_H_
#define _NEO_CONTROLLER_H_

#include "keypadInterface.h"

#include <stdio.h>
#include <string.h>

#include <errno.h> // Error integer and strerror() function
#include <fcntl.h> // Contains file controls like O_RDWR
#include <termios.h> // Contains POSIX terminal control definitions
#include <unistd.h> // write(), read(), close()

#include <thread>

/*md
## NeotrellisController

<img src="https://raw.githubusercontent.com/apiel/zicBox/main/plugins/controllers/NeotrellisController.png" />

Keypad interface using [Adafruit Neotrellis M4](https://learn.adafruit.com/adafruit-neotrellis-m4/overview).
*/
class NeotrellisController : public KeypadInterface {
protected:
    struct termios tty;
    int port;
    std::thread readThread;
    bool loop = false;

    void setAttributes()
    {
        tty.c_cflag &= ~PARENB; // Clear parity bit, disabling parity (most common)
        tty.c_cflag &= ~CSTOPB; // Clear stop field, only one stop bit used in communication (most common)
        tty.c_cflag &= ~CSIZE; // Clear all bits that set the data size
        tty.c_cflag |= CS8; // 8 bits per byte (most common)
        //   tty.c_cflag &= ~CRTSCTS; // Disable RTS/CTS hardware flow control (most common)
        tty.c_cflag |= CREAD | CLOCAL; // Turn on READ & ignore ctrl lines (CLOCAL = 1)

        tty.c_lflag &= ~ICANON;
        tty.c_lflag &= ~ECHO; // Disable echo
        tty.c_lflag &= ~ECHOE; // Disable erasure
        tty.c_lflag &= ~ECHONL; // Disable new-line echo
        tty.c_lflag &= ~ISIG; // Disable interpretation of INTR, QUIT and SUSP
        tty.c_iflag &= ~(IXON | IXOFF | IXANY); // Turn off s/w flow ctrl
        tty.c_iflag &= ~(IGNBRK | BRKINT | PARMRK | ISTRIP | INLCR | IGNCR | ICRNL); // Disable any special handling of received bytes

        tty.c_oflag &= ~OPOST; // Prevent special interpretation of output bytes (e.g. newline chars)
        tty.c_oflag &= ~ONLCR; // Prevent conversion of newline to carriage return/line feed

        tty.c_cc[VMIN] = 1;
        tty.c_cc[VTIME] = 0;
    }

    void readLoop()
    {
        loop = true;
        while (loop) {
            uint8_t byte;
            int bytesRead = read(port, &byte, 1);
            if (bytesRead) {
                if (byte == '$') {
                    uint8_t key;
                    read(port, &key, 1);
                    // printf("press key %d\n", key);
                    onKey(id, key, 1);
                } else if (byte == '!') {
                    uint8_t key;
                    read(port, &key, 1);
                    // printf("release key %d\n", key);
                    onKey(id, key, 0);
                }
            }
        }
    }

    void openSerial(char* path)
    {
        port = open(path, O_RDWR | O_NOCTTY);

        // Read in existing settings, and handle any error
        if (tcgetattr(port, &tty) != 0) {
            printf("Error %i from tcgetattr: %s\n", errno, strerror(errno));
            return;
        }

        setAttributes();
        cfsetispeed(&tty, B115200);
        cfsetospeed(&tty, B115200);
        // cfsetispeed(&tty, B921600);
        // cfsetospeed(&tty, B921600);

        if (tcsetattr(port, TCSANOW, &tty) != 0) {
            printf("Error %i from tcsetattr: %s\n", errno, strerror(errno));
            return;
        }

        readThread = std::thread(&NeotrellisController::readLoop, this);
        pthread_setname_np(readThread.native_handle(), "neotrellis");
    }

public:
    NeotrellisController(Props& props, uint16_t id)
        : KeypadInterface(props, id)
    {
    }

    ~NeotrellisController()
    {
        if (loop && readThread.joinable()) {
            loop = false;
            readThread.join();
        }
        if (port >= 0) {
            close(port);
        }
    }

    bool config(char* key, char* value)
    {
        /*md - `SERIAL: /dev/ttyACM0` open connection using serial port */
        if (strcmp(key, "SERIAL") == 0) {
            openSerial(value);
            return true;
        }
        return KeypadInterface::config(key, value);
    }

    void setKeyColor(int id, uint8_t color)
    {
        // uint8_t command = '#';
        // write(port, &command, 1);
        // uint8_t key = id;
        // write(port, &key, 1);
        // write(port, &color, 1);

        uint8_t msg[3];
        msg[0] = '#';
        msg[1] = id;
        msg[2] = color;
        write(port, msg, 3);
    }
    
    void setButton(int id, uint8_t color)
    {
        // uint8_t command = '%';
        // write(port, &command, 1);
        // uint8_t key = id;
        // write(port, &key, 1);
        // write(port, &color, 1);

        uint8_t msg[3];
        msg[0] = '%';
        msg[1] = id;
        msg[2] = color;
        write(port, msg, 3);
    }
};

#endif