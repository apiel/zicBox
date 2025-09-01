#pragma once

// Can be use without to be root,
// but GPIO_TFT_RESET_PIN make it bugs...

#include <fcntl.h> // open, O_RDWR, O_SYNC
#include <stdio.h> // printf, stderr

#include "helpers/gpio.h"

#include <cmath>
#include <errno.h>
#include <iostream>
#include <linux/spi/spidev.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>

class Spi {
protected:
    int fd = -1;

    uint8_t gpioDataControl;

    // uint32_t speed = 16000000; // 16 MHz;
    // uint32_t speed = 2500000;

    void write_buffer(uint8_t* buffer, size_t len)
    {
        int ret;
        uint8_t* pos;
        size_t remain;
        size_t send_len;
        struct spi_ioc_transfer tr = { 0 };

        // There's a limit of about 4K per ioctl here, so send everything in
        // 4K chunks

#define SPI_BUFFER_MAX (4095)

        pos = buffer;
        remain = len;

        while (remain) {
            send_len = (remain > SPI_BUFFER_MAX) ? SPI_BUFFER_MAX : remain;

            tr.tx_buf = (unsigned long)pos;
            tr.len = send_len;

            ret = ioctl(fd, SPI_IOC_MESSAGE(1), &tr);
            if (ret < 1) {
                // printf("! %s\n", strerror(errno));
                printf("!");
                return;
            }

            pos += send_len;
            remain -= send_len;
        }
    }

public:
    Spi(uint8_t gpioDataControl)
        : gpioDataControl(gpioDataControl)
    {
    }

    int init()
    {
        int ret;
        uint8_t mode;
        uint8_t bits;

        fd = open("/dev/spidev0.0", O_RDWR);
        if (fd < 0) {
            printf("Can't open device");
            return -1;
        }

        mode = 0;
        ret = ioctl(fd, SPI_IOC_WR_MODE, &mode);
        if (ret == -1) {
            printf("Can't set SPI write mode");
            return -1;
        }
        ret = ioctl(fd, SPI_IOC_RD_MODE, &mode);
        if (ret == -1) {
            printf("Can't get SPI read mode");
            return -1;
        }
        // Bits per word
        bits = 8;
        ret = ioctl(fd, SPI_IOC_WR_BITS_PER_WORD, &bits);
        if (ret == -1) {
            printf("Can't set bits");
            return -1;
        }
        ret = ioctl(fd, SPI_IOC_RD_BITS_PER_WORD, &bits);
        if (ret == -1) {
            printf("Can't get bits\n");
            return -1;
        }

        return fd;
    }

    void sendCmd(uint8_t cmd, uint8_t* payload, uint32_t payloadSize)
    {
        // An SPI transfer to the display always starts with one control (command) byte, followed by N data bytes.
        gpioWrite(gpioDataControl, 0);
        write_buffer(&cmd, 1);

        if (payloadSize > 0) {
            gpioWrite(gpioDataControl, 1);
            write_buffer(payload, payloadSize);
        }
    }

    int setSpeed(uint32_t speed)
    {
        int ret = ioctl(fd, SPI_IOC_WR_MAX_SPEED_HZ, &speed);
        if (ret == -1) {
            printf("Can't set max speed");
            return -1;
        }
        ret = ioctl(fd, SPI_IOC_RD_MAX_SPEED_HZ, &speed);
        if (ret == -1) {
            printf("Can't get max speed\n");
            return -1;
        }
        return 0;
    }
};
