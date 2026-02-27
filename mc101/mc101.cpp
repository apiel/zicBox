#include <alsa/asoundlib.h>
#include <iostream>
#include <vector>
#include <cmath>
#include <algorithm>

// Configuration
#define SAMPLE_RATE 44100
#define CHANNELS 1

// Helper to find the MC-101 hardware ID by name
std::string findMC101() {
    int card = -1;
    char *name;
    while (snd_card_next(&card) >= 0 && card >= 0) {
        snd_card_get_name(card, &name);
        std::string cardName(name);
        if (cardName.find("MC-101") != std::string::npos) {
            return "hw:" + std::to_string(card) + ",0";
        }
    }
    return "";
}

// Math for MIDI to Frequency
double midiToFreq(int note) {
    return 440.0 * pow(2.0, (note - 69.0) / 12.0);
}

// Simple Audio Generator (Square wave for better visibility/audibility)
void playBeep(snd_pcm_t* pcm, double freq) {
    int duration_samples = SAMPLE_RATE * 0.2; // 200ms beep
    std::vector<short> samples(duration_samples);
    
    for (int i = 0; i < duration_samples; i++) {
        // Square wave logic: if sine is positive, max volume; else, min volume
        double t = (double)i / SAMPLE_RATE;
        double value = sin(2.0 * M_PI * freq * t);
        samples[i] = (value > 0) ? 8000 : -8000; 
    }

    snd_pcm_prepare(pcm);
    snd_pcm_writei(pcm, samples.data(), duration_samples);
}

int main() {
    // 1. Find the MC-101
    std::string device_id = findMC101();
    if (device_id.empty()) {
        std::cerr << "Error: MC-101 not found. Is it plugged in and turned on?" << std::endl;
        return 1;
    }
    std::cout << "Found MC-101 at: " << device_id << std::endl;

    // 2. Open Audio Device
    snd_pcm_t* pcm_handle;
    if (snd_pcm_open(&pcm_handle, "default", SND_PCM_STREAM_PLAYBACK, 0) < 0) {
        std::cerr << "Could not open audio output." << std::endl;
        return 1;
    }
    snd_pcm_set_params(pcm_handle, SND_PCM_FORMAT_S16_LE, SND_PCM_ACCESS_RW_INTERLEAVED, 
                       CHANNELS, SAMPLE_RATE, 1, 500000);

    // 3. Open MIDI Device
    snd_rawmidi_t* midi_in;
    if (snd_rawmidi_open(&midi_in, nullptr, device_id.c_str(), 0) < 0) {
        std::cerr << "Could not open MIDI input." << std::endl;
        snd_pcm_close(pcm_handle);
        return 1;
    }

    std::cout << "Ready! Play a note on the MC-101 to hear a beep." << std::endl;

    // 4. Listen Loop
    unsigned char buffer[32];
    while (true) {
        ssize_t n = snd_rawmidi_read(midi_in, buffer, sizeof(buffer));
        if (n > 0) {
            for (int i = 0; i < n; i++) {
                // Look for Note On (0x90 to 0x9F)
                if ((buffer[i] & 0xF0) == 0x90) {
                    int note = buffer[i+1];
                    int vel  = buffer[i+2];
                    if (vel > 0) {
                        std::cout << "Note: " << note << " | Freq: " << midiToFreq(note) << "Hz" << std::endl;
                        playBeep(pcm_handle, midiToFreq(note));
                    }
                    i += 2;
                }
            }
        }
    }

    snd_rawmidi_close(midi_in);
    snd_pcm_close(pcm_handle);
    return 0;
}