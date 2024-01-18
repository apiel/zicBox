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

<img src="https://raw.githubusercontent.com/apiel/zicBox/main/plugins/components/NeotrellisController.png" />

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
                    onKeypad(key, 1);
                } else if (byte == '!') {
                    uint8_t key;
                    read(port, &key, 1);
                    // printf("release key %d\n", key);
                    onKeypad(key, 0);
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
    }

public:
    NeotrellisController(Props& props)
        : KeypadInterface(props)
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