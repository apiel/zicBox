#include <alsa/asoundlib.h>
#include <iostream>
#include <vector>
#include <cmath>

// Audio Settings
static constexpr uint32_t SAMPLE_RATE = 44100;
static constexpr uint32_t CHUNK_FRAMES = 128; // Small buffer for low latency
static constexpr int16_t AMPLITUDE = 10000;

// Find MC-101 by scanning card names
std::string findMC101() {
    int card = -1;
    while (snd_card_next(&card) >= 0 && card >= 0) {
        char* name;
        if (snd_card_get_name(card, &name) == 0) {
            std::string cardName(name);
            free(name);
            if (cardName.find("MC-101") != std::string::npos) {
                return "hw:" + std::to_string(card) + ",0";
            }
        }
    }
    return "";
}

// Write audio to ALSA and handle under-runs (XRUN)
void writeAudio(snd_pcm_t* pcm, const std::vector<int16_t>& buffer) {
    snd_pcm_sframes_t ret = snd_pcm_writei(pcm, buffer.data(), CHUNK_FRAMES);
    if (ret < 0) {
        snd_pcm_recover(pcm, static_cast<int>(ret), 0);
    }
}

int main() {
    // 1. Find Device
    std::string midi_id = findMC101();
    if (midi_id.empty()) {
        std::cerr << "Error: MC-101 not found!" << std::endl;
        return 1;
    }
    std::cout << "Found MC-101 at: " << midi_id << std::endl;

    // 2. Setup PCM (Audio Output)
    snd_pcm_t* pcm_handle;
    if (snd_pcm_open(&pcm_handle, "default", SND_PCM_STREAM_PLAYBACK, 0) < 0) return 1;
    
    snd_pcm_set_params(pcm_handle, SND_PCM_FORMAT_S16_LE, SND_PCM_ACCESS_RW_INTERLEAVED, 
                       1, SAMPLE_RATE, 1, 100000); // 100ms latency

    // 3. Setup RawMIDI (Input)
    snd_rawmidi_t* midi_handle;
    if (snd_rawmidi_open(&midi_handle, nullptr, midi_id.c_str(), 0) < 0) return 1;

    std::cout << "Listening for notes..." << std::endl;

    std::vector<int16_t> audioBuffer(CHUNK_FRAMES);
    unsigned char midiBuf[32];

    while (true) {
        ssize_t n = snd_rawmidi_read(midi_handle, midiBuf, sizeof(midiBuf));
        if (n > 0) {
            for (int i = 0; i < n; i++) {
                // Note On Detection
                if ((midiBuf[i] & 0xF0) == 0x90 && midiBuf[i+2] > 0) {
                    int note = midiBuf[i+1];
                    double freq = 440.0 * pow(2.0, (note - 69.0) / 12.0);
                    std::cout << "Playing Note: " << note << std::endl;

                    // Generate and play several chunks of a square wave
                    for (int chunk = 0; chunk < 20; ++chunk) { 
                        for (uint32_t f = 0; f < CHUNK_FRAMES; ++f) {
                            double t = (double)(chunk * CHUNK_FRAMES + f) / SAMPLE_RATE;
                            audioBuffer[f] = (sin(2.0 * M_PI * freq * t) > 0) ? AMPLITUDE : -AMPLITUDE;
                        }
                        writeAudio(pcm_handle, audioBuffer);
                    }
                    i += 2;
                }
            }
        }
    }

    snd_rawmidi_close(midi_handle);
    snd_pcm_close(pcm_handle);
    return 0;
}
