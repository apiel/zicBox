#include <alsa/asoundlib.h>
#include <iostream>
#include <vector>
#include <cmath>
#include <atomic>
#include <thread>
#include <mutex> // Added for thread safety

#include "audio/engines/DrumKick2.h"

static constexpr uint32_t SAMPLE_RATE = 44100;
static constexpr uint32_t CHUNK_FRAMES = 256; 

std::atomic<bool> keep_running{true};
std::mutex engine_mutex; // Prevents audio glitches during NoteOn
DrumKick2 kick2(SAMPLE_RATE);

// --- AUDIO THREAD ---
void audioThread(snd_pcm_t* pcm) {
    std::vector<int16_t> buffer(CHUNK_FRAMES);

    while (keep_running) {
        {
            // Lock the mutex briefly to pull samples safely
            std::lock_guard<std::mutex> lock(engine_mutex);
            for (uint32_t i = 0; i < CHUNK_FRAMES; ++i) {
                float s = kick2.sample();
                
                // 1. Clamp the float to -1.0 ... 1.0 to prevent clipping noise
                if (s > 1.0f) s = 1.0f;
                if (s < -1.0f) s = -1.0f;

                // 2. Convert float (-1.0 to 1.0) to int16 (-32768 to 32767)
                buffer[i] = static_cast<int16_t>(s * 32767.0f);
            }
        }

        snd_pcm_sframes_t ret = snd_pcm_writei(pcm, buffer.data(), CHUNK_FRAMES);
        if (ret < 0) {
            snd_pcm_recover(pcm, static_cast<int>(ret), 0);
        }
    }
}

// --- SEARCH LOGIC ---
std::string findMC101() {
    int card = -1;
    while (snd_card_next(&card) >= 0 && card >= 0) {
        char* name;
        if (snd_card_get_name(card, &name) == 0) {
            std::string cardName(name);
            free(name);
            if (cardName.find("MC-101") != std::string::npos) 
                return "hw:" + std::to_string(card) + ",0";
        }
    }
    return "";
}

int main() {
    std::string midi_id = findMC101();
    if (midi_id.empty()) { std::cerr << "MC-101 not found!" << std::endl; return 1; }

    // 1. Setup Audio
    snd_pcm_t* pcm_handle;
    if (snd_pcm_open(&pcm_handle, "default", SND_PCM_STREAM_PLAYBACK, 0) < 0) return 1;
    snd_pcm_set_params(pcm_handle, SND_PCM_FORMAT_S16_LE, SND_PCM_ACCESS_RW_INTERLEAVED, 1, SAMPLE_RATE, 1, 20000);

    // 2. Setup MIDI
    snd_rawmidi_t* midi_handle;
    if (snd_rawmidi_open(&midi_handle, nullptr, midi_id.c_str(), 0) < 0) return 1;

    // 3. Start Audio Thread
    std::thread server(audioThread, pcm_handle);

    std::cout << "Kick2 Engine Ready. Listening for MIDI Note 38..." << std::endl;

    // 4. Main Loop (MIDI Listener)
    unsigned char midiBuf[32];
    while (keep_running) {
        ssize_t n = snd_rawmidi_read(midi_handle, midiBuf, sizeof(midiBuf));
        if (n < 0) continue;

        for (int i = 0; i < n; i++) {
            // Mask for Note On (ignore channel)
            if ((midiBuf[i] & 0xF0) == 0x90) {
                uint8_t note = midiBuf[i+1];
                float velocity = (float)midiBuf[i+2] / 127.0f;

                // Check for Note 38 and check velocity > 0 
                // (Some devices send NoteOn with 0 velocity instead of NoteOff)
                if (note == 38 && velocity > 0.0f) {
                    std::lock_guard<std::mutex> lock(engine_mutex);
                    kick2.noteOn(note, velocity);
                    std::cout << "DRUM TRIGGER: Note 38 | Vel: " << velocity << std::endl;
                }
                i += 2;
            }
        }
    }

    keep_running = false;
    server.join();
    snd_rawmidi_close(midi_handle);
    snd_pcm_close(pcm_handle);
    return 0;
}