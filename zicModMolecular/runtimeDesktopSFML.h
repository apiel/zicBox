#pragma once

#include <SFML/Audio.hpp>
#include <SFML/Graphics.hpp>
#include <atomic>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <string>
#include <vector>

#include "displayView.h"
#include "zicApp.h"

class MolecularAudioStream : public sf::SoundStream {
public:
    ZicApp* app = nullptr;
    std::vector<float> floatBuffer;
    std::vector<sf::Int16> pcmBuffer;

    MolecularAudioStream()
    {
        initialize(1, static_cast<unsigned int>(AudioEngine::SAMPLE_RATE));
    }

    void setApp(ZicApp* a)
    {
        app = a;
    }

protected:
    bool onGetData(Chunk& data) override
    {
        const size_t numFrames = 512;
        if (floatBuffer.size() < numFrames) {
            floatBuffer.resize(numFrames, 0.0f);
            pcmBuffer.resize(numFrames, 0);
        }

        if (app) {
            app->audioEngine.renderAudioBlock(floatBuffer.data(), numFrames, app->nodes, app->connections);

            for (size_t i = 0; i < numFrames; ++i) {
                float sample = std::clamp(floatBuffer[i], -1.0f, 1.0f);
                pcmBuffer[i] = static_cast<sf::Int16>(sample * 32767.0f);
            }
        } else {
            std::fill(pcmBuffer.begin(), pcmBuffer.end(), 0);
        }

        data.samples = pcmBuffer.data();
        data.sampleCount = numFrames;
        return true;
    }

    void onSeek(sf::Time timeOffset) override {}
};

inline void runDesktopSFML(Draw& d, ZicApp& app, DisplayView& displayView, std::atomic<bool>& keepRunning)
{
    const int WIN_W = DisplayView::SCREEN_W; // 480
    const int WIN_H = DisplayView::SCREEN_H; // 800

    // Check for screenshot environment variable (headless screenshot generation)
    const char* screenshotEnv = std::getenv("ZIC_SCREENSHOT");
    if (screenshotEnv != nullptr) {
        std::string basePath(screenshotEnv);
        sf::Image screenshot;
        std::vector<sf::Uint8> pixelBuf(WIN_W * WIN_H * 4, 255);

        d.setScreenSize({ DisplayView::SCREEN_W, DisplayView::SCREEN_H });
        displayView.render(d, app);

        for (int y = 0; y < WIN_H; ++y) {
            for (int x = 0; x < WIN_W; ++x) {
                Color c = d.screenBuffer[y][x];
                int dstIdx = (y * WIN_W + x) * 4;
                pixelBuf[dstIdx + 0] = c.r;
                pixelBuf[dstIdx + 1] = c.a > 0 ? c.g : 0;
                pixelBuf[dstIdx + 2] = c.b;
                pixelBuf[dstIdx + 3] = 255;
            }
        }
        screenshot.create(WIN_W, WIN_H, pixelBuf.data());
        std::string filePath = basePath + "_portrait.png";
        screenshot.saveToFile(filePath);
        std::cout << "[Screenshot] Exported: " << filePath << std::endl;
        return;
    }

    sf::RenderWindow window(sf::VideoMode(WIN_W, WIN_H), "zicModMolecular - 480x800 Portrait Touch Simulator");
    window.setFramerateLimit(60);

    sf::Texture screenTexture;
    screenTexture.create(WIN_W, WIN_H);
    sf::Sprite screenSprite(screenTexture);
    std::vector<sf::Uint8> pixelBuffer(WIN_W * WIN_H * 4, 255);

    d.setScreenSize({ DisplayView::SCREEN_W, DisplayView::SCREEN_H });

    // Start SFML Audio Stream
    MolecularAudioStream audioStream;
    audioStream.setApp(&app);
    audioStream.play();

    bool isMouseDown = false;

    while (window.isOpen() && keepRunning) {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed) {
                window.close();
                keepRunning = false;
            } else if (event.type == sf::Event::MouseButtonPressed) {
                if (event.mouseButton.button == sf::Mouse::Left) {
                    isMouseDown = true;
                    app.handleTouchDown(event.mouseButton.x, event.mouseButton.y);
                }
            } else if (event.type == sf::Event::MouseMoved) {
                if (isMouseDown) {
                    app.handleTouchMove(event.mouseMove.x, event.mouseMove.y);
                }
            } else if (event.type == sf::Event::MouseButtonReleased) {
                if (event.mouseButton.button == sf::Mouse::Left) {
                    isMouseDown = false;
                    app.handleTouchUp();
                }
            } else if (event.type == sf::Event::KeyPressed) {
                if (event.key.code == sf::Keyboard::Escape) {
                    window.close();
                    keepRunning = false;
                } else if (event.key.code == sf::Keyboard::Space) {
                    app.initDefaultPatch();
                }
            }
        }

        // Render scene
        displayView.render(d, app);

        // Copy screen buffer into SFML pixel buffer
        for (int y = 0; y < WIN_H; ++y) {
            for (int x = 0; x < WIN_W; ++x) {
                Color c = d.screenBuffer[y][x];
                int dstIdx = (y * WIN_W + x) * 4;
                pixelBuffer[dstIdx + 0] = c.r;
                pixelBuffer[dstIdx + 1] = c.g;
                pixelBuffer[dstIdx + 2] = c.b;
                pixelBuffer[dstIdx + 3] = 255;
            }
        }

        screenTexture.update(pixelBuffer.data());
        window.clear();
        window.draw(screenSprite);
        window.display();
    }

    audioStream.stop();
}
