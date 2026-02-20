#pragma once

#include "stm32/platform.h"

#ifdef IS_STM32

void log(const char* message, ...) { };

#else

#include <stdarg.h>
#include <string>

void log(const char* message, ...)
{
    va_list args;
    va_start(args, message);
    printf("\033[35m[LOG]\033[0m ");
    vprintf(message, args);
    printf("\n");
    va_end(args);
    fflush(stdout);
}
#endif
