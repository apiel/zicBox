#pragma once

#ifndef SAMPLE_RATE
// #define SAMPLE_RATE 44100
#define SAMPLE_RATE 48000
#endif

#ifndef APP_CHANNELS
#define APP_CHANNELS 2
// #define APP_CHANNELS 1
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

// To be deprecated?
#ifndef DEFAULT_MAX_STEPS
#define DEFAULT_MAX_STEPS 32
#endif

#ifndef CURRENT_REPO_FOLDER
#define CURRENT_REPO_FOLDER std::string("data/repositories/current")
#endif