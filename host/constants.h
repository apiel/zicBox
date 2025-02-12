#pragma once

#ifndef SAMPLE_RATE
// #define SAMPLE_RATE 44100
#define SAMPLE_RATE 48000
#endif

#ifndef APP_CHANNELS
#define APP_CHANNELS 2
// #define APP_CHANNELS 1
#endif

#ifndef APP_AUDIO_CHUNK
#define APP_AUDIO_CHUNK 128
#endif

#ifndef APP_PRINT
#define APP_PRINT printf
#endif

#ifndef APP_INFO
#define APP_INFO APP_PRINT
#endif

#ifndef MAX_TRACKS
#define MAX_TRACKS 32
#endif

#ifndef CLOCK_TRACK
#define CLOCK_TRACK MAX_TRACKS
#endif

#ifndef TOTAL_TRACKS
#define TOTAL_TRACKS MAX_TRACKS+1
#endif

#ifndef MAX_STEPS
#define MAX_STEPS 32
#endif