#ifndef _TIMER_H_
#define _TIMER_H_

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

#endif
