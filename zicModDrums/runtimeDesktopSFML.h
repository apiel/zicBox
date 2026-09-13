#pragma once

#include <SFML/Graphics.hpp>
#include <atomic>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <string>

#include "displayView.h"
#include "zicApp.h"

inline void runDesktopSFML(Draw& d, ZicApp& app, DisplayView& displayView, std::atomic<bool>& keepRunning)
{
    const int SCALE = 2; // 2x scale for crisp desktop rendering (640x340)
    const int WIN_W = DisplayView::SCREEN_W * SCALE;
    const int WIN_H = DisplayView::SCREEN_H * SCALE;

    // Check for screenshot environment variable (headless screenshot generation)
    const char* screenshotEnv = std::getenv("ZIC_SCREENSHOT");
    if (screenshotEnv != nullptr) {
        std::string basePath(screenshotEnv);
        sf::Image screenshot;
        std::vector<sf::Uint8> pixelBuf(WIN_W * WIN_H * 4, 255);

        for (int viewIdx = 0; viewIdx < NUM_VIEWS; ++viewIdx) {
            app.setView(viewIdx);
            d.setScreenSize({ DisplayView::SCREEN_W, DisplayView::SCREEN_H });
            displayView.render(d, app);

            // Scale to 2x for screenshot export
            for (int y = 0; y < WIN_H; ++y) {
                int srcY = y / SCALE;
                for (int x = 0; x < WIN_W; ++x) {
                    int srcX = x / SCALE;
                    Color c = d.screenBuffer[srcY][srcX];
                    int dstIdx = (y * WIN_W + x) * 4;
                    pixelBuf[dstIdx + 0] = c.r;
                    pixelBuf[dstIdx + 1] = c.g;
                    pixelBuf[dstIdx + 2] = c.b;
                    pixelBuf[dstIdx + 3] = c.a;
                }
            }
            screenshot.create(WIN_W, WIN_H, pixelBuf.data());
            std::string filePath = basePath + "_" + std::to_string(viewIdx) + ".png";
            screenshot.saveToFile(filePath);
            std::cout << "[Screenshot] Exported: " << filePath << std::endl;
        }
        return;
    }

    sf::RenderWindow window(sf::VideoMode(WIN_W, WIN_H), "zicModDrums - LilyGO T-Display-S3 Simulator (320x170)");
    window.setFramerateLimit(60);

    sf::Texture screenTexture;
    screenTexture.create(WIN_W, WIN_H);
    sf::Sprite screenSprite(screenTexture);
    std::vector<sf::Uint8> pixelBuffer(WIN_W * WIN_H * 4, 255);

    d.setScreenSize({ DisplayView::SCREEN_W, DisplayView::SCREEN_H });

    while (window.isOpen() && keepRunning) {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed) {
                window.close();
                keepRunning = false;
            } else if (event.type == sf::Event::KeyPressed) {
                int padIdx = -1;
                switch (event.key.code) {
                    case sf::Keyboard::Num1: padIdx = 0; break;
                    case sf::Keyboard::Num2: padIdx = 1; break;
                    case sf::Keyboard::Num3: padIdx = 2; break;
                    case sf::Keyboard::Num4: padIdx = 3; break;

                    case sf::Keyboard::Q: padIdx = 4; break;
                    case sf::Keyboard::W: padIdx = 5; break;
                    case sf::Keyboard::E: padIdx = 6; break;
                    case sf::Keyboard::R: padIdx = 7; break;

                    case sf::Keyboard::A: padIdx = 8; break;
                    case sf::Keyboard::S: padIdx = 9; break;
                    case sf::Keyboard::D: padIdx = 10; break;
                    case sf::Keyboard::F: padIdx = 11; break;

                    case sf::Keyboard::Z: padIdx = 12; break;
                    case sf::Keyboard::X: padIdx = 13; break;
                    case sf::Keyboard::C: padIdx = 14; break;
                    case sf::Keyboard::V: padIdx = 15; break;

                    case sf::Keyboard::Left: app.prevView(); break;
                    case sf::Keyboard::Right: app.nextView(); break;
                    case sf::Keyboard::Space: app.brain.isPlaying = !app.brain.isPlaying; break;

                    case sf::Keyboard::Up:
                        app.brain.selectedTrack = (app.brain.selectedTrack + 7) % SequenceBrain::NUM_TRACKS;
                        break;

                    case sf::Keyboard::Down:
                        app.brain.selectedTrack = (app.brain.selectedTrack + 1) % SequenceBrain::NUM_TRACKS;
                        break;

                    default: break;
                }
                if (padIdx >= 0) {
                    app.handlePadPress(padIdx, true);
                }
            } else if (event.type == sf::Event::MouseButtonPressed) {
                if (event.mouseButton.button == sf::Mouse::Left) {
                    int simX = event.mouseButton.x / SCALE;
                    int simY = event.mouseButton.y / SCALE;
                    displayView.handleTouchDown(simX, simY);
                }
            } else if (event.type == sf::Event::MouseMoved) {
                if (sf::Mouse::isButtonPressed(sf::Mouse::Left)) {
                    int simX = event.mouseMove.x / SCALE;
                    int simY = event.mouseMove.y / SCALE;
                    displayView.handleTouchMove(simX, simY);
                }
            } else if (event.type == sf::Event::MouseButtonReleased) {
                if (event.mouseButton.button == sf::Mouse::Left) {
                    int simX = event.mouseButton.x / SCALE;
                    int simY = event.mouseButton.y / SCALE;
                    displayView.handleTouchUp(simX, simY, app);
                }
            }
        }

        // Render UI canvas
        displayView.render(d, app);

        // Scale to 2x window buffer
        for (int y = 0; y < WIN_H; ++y) {
            int srcY = y / SCALE;
            for (int x = 0; x < WIN_W; ++x) {
                int srcX = x / SCALE;
                Color c = d.screenBuffer[srcY][srcX];
                int dstIdx = (y * WIN_W + x) * 4;
                pixelBuffer[dstIdx + 0] = c.r;
                pixelBuffer[dstIdx + 1] = c.g;
                pixelBuffer[dstIdx + 2] = c.b;
                pixelBuffer[dstIdx + 3] = c.a;
            }
        }

        screenTexture.update(pixelBuffer.data());
        window.clear();
        window.draw(screenSprite);
        window.display();
    }
}
