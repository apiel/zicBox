#ifndef _DEF_H
#define _DEF_H

#ifdef PIGPIO
// sudo apt-get install libpigpio-dev
#include <pigpio.h>
#endif

#include <lo/lo.h>

#include <stdint.h>
#include <vector>
#include <stdio.h>
#include <unistd.h>
#include <cstring>

#define ENCODER_COUNT 12
#define BUTTON_COUNT 0

bool debug = false;
lo_address oscClient = lo_address_new(nullptr, "8888");

#endif