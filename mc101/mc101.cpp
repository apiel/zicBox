#include <alsa/asoundlib.h>
#include <iostream>
#include <vector>
#include <csignal>

// Global flag to handle clean exits (Ctrl+C)
bool keep_running = true;
void signalHandler(int signum) {
    keep_running = false;
}

void parseMidi(const unsigned char* buffer, ssize_t length) {
    for (ssize_t i = 0; i < length; ++i) {
        unsigned char status = buffer[i];

        // We only care about Status Bytes (bit 7 is set)
        if (!(status & 0x80)) continue;

        unsigned char eventType = status & 0xF0;
        unsigned char channel = (status & 0x0F) + 1;

        if (eventType == 0x90) { // Note On
            unsigned char note = buffer[++i];
            unsigned char velocity = buffer[++i];
            if (velocity > 0) {
                std::cout << "[CH " << (int)channel << "] Note ON:  " << (int)note 
                          << " | Velocity: " << (int)velocity << std::endl;
            } else {
                std::cout << "[CH " << (int)channel << "] Note OFF: " << (int)note << std::endl;
            }
        } 
        else if (eventType == 0x80) { // Note Off
            unsigned char note = buffer[++i];
            unsigned char velocity = buffer[++i]; // Ignore velocity for off usually
            std::cout << "[CH " << (int)channel << "] Note OFF: " << (int)note << std::endl;
        }
        else if (eventType == 0xB0) { // Control Change (Knobs/Sliders)
            unsigned char controller = buffer[++i];
            unsigned char value = buffer[++i];
            std::cout << "[CH " << (int)channel << "] CC: " << (int)controller 
                      << " | Value: " << (int)value << std::endl;
        }
    }
}

int main() {
    // Setup signal handler for Ctrl+C
    signal(SIGINT, signalHandler);

    const char* device_id = "hw:3,0";
    snd_rawmidi_t* midi_in = nullptr;
    int err;

    // 1. Attempt to open the MC-101
    // SND_RAWMIDI_SYNC ensures we don't drop bytes if we're slightly slow
    err = snd_rawmidi_open(&midi_in, nullptr, device_id, 0);
    
    if (err < 0) {
        std::cerr << "FATAL: Could not find MC-101 at " << device_id << std::endl;
        std::cerr << "ALSA Error: " << snd_strerror(err) << std::endl;
        return 1; // Exit as requested
    }

    std::cout << "--- Connected to MC-101 (" << device_id << ") ---" << std::endl;
    std::cout << "Waiting for MIDI input... (Press Ctrl+C to stop)" << std::endl;

    // 2. Main Listening Loop
    unsigned char buffer[256]; 
    while (keep_running) {
        // This call blocks until at least 1 byte is available
        ssize_t bytesRead = snd_rawmidi_read(midi_in, buffer, sizeof(buffer));
        
        if (bytesRead < 0) {
            if (bytesRead != -EAGAIN && bytesRead != -EINTR) {
                std::cerr << "Read error: " << snd_strerror(bytesRead) << std::endl;
                break;
            }
            continue;
        }

        if (bytesRead > 0) {
            parseMidi(buffer, bytesRead);
        }
    }

    // 3. Cleanup
    std::cout << "\nClosing connection..." << std::endl;
    snd_rawmidi_close(midi_in);
    
    return 0;
}