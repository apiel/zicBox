#pragma once

#ifndef SAMPLE_RATE
// #define SAMPLE_RATE 44100
#define SAMPLE_RATE 48000
#endif

#define CHANNEL_MONO 1
#define CHANNEL_STEREO 2

#ifndef AUDIO_CHANNELS
#define AUDIO_CHANNELS CHANNEL_STEREO
// #define AUDIO_CHANNELS CHANNEL_MONO
#endif

#ifndef MAX_TRACKS
#define MAX_TRACKS 32
#endif

#ifndef CLOCK_TRACK
#define CLOCK_TRACK MAX_TRACKS
#endif

#ifndef STEP_TRACK
#define STEP_TRACK MAX_TRACKS-1
#endif

#ifndef TOTAL_TRACKS
#define TOTAL_TRACKS MAX_TRACKS+1
#endif

// To be deprecated?
#ifndef DEFAULT_MAX_STEPS
#define DEFAULT_MAX_STEPS 32
#endif

#ifndef WORKSPACE_FOLDER
#define WORKSPACE_FOLDER std::string("data/workspaces")
#endif

#ifndef AUDIO_FOLDER
#define AUDIO_FOLDER std::string("data/audio")
#endif