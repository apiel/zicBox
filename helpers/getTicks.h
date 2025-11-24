/** Description:
This header file serves as a robust, flexible utility for accurately measuring elapsed time within an application. Its primary purpose is to define a single, simple function (`getTicks`) that returns the system's time, typically measured in milliseconds, since the program or system initialized.

The crucial design feature is its adaptability, achieved through a conditional structure:

1.  **The Smart Switch:** The file first checks if the program is set up to use the popular SDL (Simple DirectMedia Layer) library for timing.

2.  **Scenario A (SDL Used):** If the SDL timer flag is active, the code utilizes SDL’s highly accurate, cross-platform 64-bit timing function. This is often the preferred, easy-to-use method for games and multimedia applications.

3.  **Scenario B (Standard System Time):** If the SDL flag is not active, the code falls back to standard operating system (POSIX) functions. It consults the system's monotonic clock—a clock guaranteed to always move forward, even if the user manually changes the date and time. It gathers the time in both seconds and very tiny nanosecond units, then performs a calculation to convert this information into a total count of milliseconds.

In either scenario, the result is an extremely large, positive number (a 64-bit integer) representing the total elapsed time, ensuring precision and preventing timer overflow issues that older systems might face. This abstraction allows the main program to always call the same function regardless of the underlying timing technology.

sha: 8d951594f4d1ed7a3718dad3f629baa54ca052019c875a96cbfe3be5226c409c 
*/
#pragma once

#include <cstdint>

#ifdef USE_SDL_TIMER
#include <SDL2/SDL.h>
static uint64_t getTicks()
{
    return SDL_GetTicks64();
}
#else
#include <time.h>
static uint64_t getTicks()
{
    struct timespec ts;

    clock_gettime(CLOCK_MONOTONIC, &ts);

    return (uint64_t)(ts.tv_nsec / 1000000) + ((uint64_t)ts.tv_sec * 1000ull);
}
#endif
