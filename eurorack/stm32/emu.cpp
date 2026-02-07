#include <SFML/Graphics.hpp>
#include <alsa/asoundlib.h>
#include <iostream>
#include <thread>
#include <atomic>
#include <vector>
#include <string>

#include "Core.h"
#include "draw/fonts/PoppinsLight_12.h"

// --- Constants ---
const int SCREEN_W = 160;
const int SCREEN_H = 80;
const int SCALE = 4;

// --- SFML & Draw Bridge ---
// This emulates your ST7735.h behavior: it provides a framebuffer for DrawPrimitives
class EmuDisplay : public DrawPrimitives {
public:
    // Framebuffer matching your ST7735.h logic (16-bit RGB565)
    uint16_t buffer[SCREEN_W * SCREEN_H];

    EmuDisplay() : DrawPrimitives(PoppinsLight_12, SCREEN_W) {
        std::memset(buffer, 0, sizeof(buffer));
    }

    // This is what Core.h calls via DrawPrimitives
    void pixel(Point position, Color color) override {
        if (position.x < 0 || position.x >= SCREEN_W || position.y < 0 || position.y >= SCREEN_H) 
            return;
        
        // Convert RGB888 to RGB565 (matching your rgb565 function)
        uint16_t r = (color.r & 0xF8) << 8;
        uint16_t g = (color.g & 0xFC) << 3;
        uint16_t b = (color.b >> 3);
        uint16_t rawColor = r | g | b;

        // Store in buffer (Note: we don't need the SPI byte-swap (<<8 | >>8) 
        // here because we aren't sending to an ST7735 hardware chip)
        buffer[position.y * SCREEN_W + position.x] = rawColor;
    }

    // Helper for SFML to read the buffer
    sf::Color getSfColor(int x, int y) {
        uint16_t p = buffer[y * SCREEN_W + x];
        // Extract RGB565 to RGB888
        uint8_t r = (p >> 11) & 0x1F;
        uint8_t g = (p >> 5) & 0x3F;
        uint8_t b = p & 0x1F;
        return sf::Color((r * 255) / 31, (g * 255) / 63, (b * 255) / 31);
    }
};

// --- ALSA Audio Implementation ---
class AlsaAudio {
    snd_pcm_t* handle;
    std::thread audioThread;
    std::atomic<bool> running{true};

public:
    AlsaAudio() {
        if (snd_pcm_open(&handle, "default", SND_PCM_STREAM_PLAYBACK, 0) < 0) {
            std::cerr << "ALSA: Cannot open device" << std::endl;
            exit(1);
        }
        snd_pcm_set_params(handle, SND_PCM_FORMAT_FLOAT_LE, SND_PCM_ACCESS_RW_INTERLEAVED,
                           1, (unsigned int)SAMPLE_RATE, 1, 50000);
    }

    ~AlsaAudio() {
        running = false;
        if (audioThread.joinable()) audioThread.join();
        snd_pcm_close(handle);
    }

    void start(Core& core) {
        audioThread = std::thread([this, &core]() {
            float audioBuf[256];
            while (running) {
                for (int i = 0; i < 256; ++i) audioBuf[i] = core.sample();
                if (snd_pcm_writei(handle, audioBuf, 256) < 0) snd_pcm_prepare(handle);
            }
        });
    }
};

// --- Main Emulator Loop ---
int main() {
    sf::RenderWindow window(sf::VideoMode(SCREEN_W * SCALE, SCREEN_H * SCALE), "STM32 Emulator");
    window.setFramerateLimit(60);

    EmuDisplay display;
    Core core(display);
    AlsaAudio audio;

    // SFML Texture and Sprite for rendering the framebuffer
    sf::Texture texture;
    texture.create(SCREEN_W, SCREEN_H);
    sf::Sprite sprite(texture);
    sprite.setScale(SCALE, SCALE);

    // Pixel array for SFML texture updates (RGBA8888)
    std::vector<sf::Uint8> sfPixels(SCREEN_W * SCREEN_H * 4);

    audio.start(core);

    while (window.isOpen()) {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed) window.close();

            // Mouse Wheel -> Encoder
            if (event.type == sf::Event::MouseWheelScrolled) {
                core.onEncoder(event.mouseWheelScroll.delta > 0 ? 1 : -1);
            }

            // A, S, D, F -> Encoder Button
            if (event.type == sf::Event::KeyPressed) {
                if (event.key.code == sf::Keyboard::A || event.key.code == sf::Keyboard::S ||
                    event.key.code == sf::Keyboard::D || event.key.code == sf::Keyboard::F) {
                    core.encBtn();
                }
            }
        }

        // 1. Core Logic & Draw calls to display.buffer
        core.render();

        // 2. Convert display.buffer (RGB565) to SFML Texture (RGBA8888)
        for (int y = 0; y < SCREEN_H; ++y) {
            for (int x = 0; x < SCREEN_W; ++x) {
                sf::Color c = display.getSfColor(x, y);
                int idx = (y * SCREEN_W + x) * 4;
                sfPixels[idx]     = c.r;
                sfPixels[idx + 1] = c.g;
                sfPixels[idx + 2] = c.b;
                sfPixels[idx + 3] = 255; // Alpha
            }
        }
        texture.update(sfPixels.data());

        // 3. Render to Window
        window.clear();
        window.draw(sprite);
        window.display();
    }

    return 0;
}