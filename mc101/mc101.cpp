#include <alsa/asoundlib.h>
#include <iostream>
#include <vector>
#include <cmath>
#include <atomic>
#include <thread>

static constexpr uint32_t SAMPLE_RATE = 44100;
static constexpr uint32_t CHUNK_FRAMES = 256; 

// Atomic variables to communicate between threads safely
std::atomic<double> currentFreq{0.0};
std::atomic<bool> keep_running{true};

// --- AUDIO THREAD ---
void audioThread(snd_pcm_t* pcm) {
    std::vector<int16_t> buffer(CHUNK_FRAMES);
    uint32_t phase_index = 0;

    while (keep_running) {
        double freq = currentFreq.load();
        
        for (uint32_t i = 0; i < CHUNK_FRAMES; ++i) {
            if (freq > 0) {
                // Smooth square wave generation
                double t = (double)phase_index / SAMPLE_RATE;
                buffer[i] = (sin(2.0 * M_PI * freq * t) > 0) ? 8000 : -8000;
                phase_index++;
            } else {
                buffer[i] = 0; // Silence
                phase_index = 0;
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
            if (cardName.find("MC-101") != std::string::npos) return "hw:" + std::to_string(card) + ",0";
        }
    }
    return "";
}

int main() {
    std::string midi_id = findMC101();
    if (midi_id.empty()) { std::cerr << "MC-101 not found!" << std::endl; return 1; }

    // 1. Setup Audio
    snd_pcm_t* pcm_handle;
    snd_pcm_open(&pcm_handle, "default", SND_PCM_STREAM_PLAYBACK, 0);
    snd_pcm_set_params(pcm_handle, SND_PCM_FORMAT_S16_LE, SND_PCM_ACCESS_RW_INTERLEAVED, 1, SAMPLE_RATE, 1, 20000);

    // 2. Setup MIDI
    snd_rawmidi_t* midi_handle;
    snd_rawmidi_open(&midi_handle, nullptr, midi_id.c_str(), 0);

    // 3. Start Audio Thread
    std::thread server(audioThread, pcm_handle);

    std::cout << "Engine Running. Found MC-101 at " << midi_id << std::endl;

    // 4. Main Loop (MIDI Listener)
    unsigned char midiBuf[32];
    while (keep_running) {
        ssize_t n = snd_rawmidi_read(midi_handle, midiBuf, sizeof(midiBuf));
        for (int i = 0; i < n; i++) {
            if ((midiBuf[i] & 0xF0) == 0x90) { // Note On
                int note = midiBuf[i+1];
                int vel = midiBuf[i+2];
                if (vel > 0) currentFreq = 440.0 * pow(2.0, (note - 69.0) / 12.0);
                else currentFreq = 0.0;
            } 
            else if ((midiBuf[i] & 0xF0) == 0x80) { // Note Off
                currentFreq = 0.0;
            }
        }
    }

    server.join();
    snd_rawmidi_close(midi_handle);
    snd_pcm_close(pcm_handle);
    return 0;
}