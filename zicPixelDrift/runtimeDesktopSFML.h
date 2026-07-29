#pragma once

#include <SFML/Graphics.hpp>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <memory>
#include <string>
#include <vector>

#include "draw/draw.h"
#include "kickBody.h"
#include "mixer.h"
#include "sequencer.h"
#include "ui.h"

#ifndef BUFFER_SIZE
#define BUFFER_SIZE 4096
#endif

extern std::atomic<bool> keep_running;

void runDesktopSFML(Draw& d, UiPixelDrift& ui, bool& needFullRedraw)
{
    const int SCREEN_W = 320;
    const int SCREEN_H = 176;

    const char* screenshotEnv = std::getenv("ZIC_SCREENSHOT");
    if (screenshotEnv != nullptr) {
        std::string basePath(screenshotEnv);
        std::vector<sf::Uint8> screenshotPixels(SCREEN_W * SCREEN_H * 4);

        for (int viewIdx = 0; viewIdx < VIEW_COUNT; ++viewIdx) {
            ui.currentView = static_cast<ViewState>(viewIdx);
            needFullRedraw = true;

            d.setScreenSize({ SCREEN_W, SCREEN_H });

            if (ui.drawUI(d, SCREEN_W, SCREEN_H, needFullRedraw)) {
                for (unsigned y = 0; y < SCREEN_H; y++) {
                    std::memcpy(&screenshotPixels[y * SCREEN_W * 4], d.screenBuffer[y], SCREEN_W * 4);
                }
            }

            sf::Image screenshot;
            screenshot.create(SCREEN_W, SCREEN_H, screenshotPixels.data());
            screenshot.saveToFile(basePath + "_" + std::to_string(viewIdx) + ".png");
        }
        return;
    }

    const int WINDOW_SCALE = 2;

    sf::RenderWindow window(sf::VideoMode(SCREEN_W * WINDOW_SCALE, SCREEN_H * WINDOW_SCALE), "zicPixelDrift (Desktop Emulation)");
    window.setFramerateLimit(60);
    window.setKeyRepeatEnabled(false);

    sf::Texture screenTexture;
    screenTexture.create(BUFFER_SIZE, BUFFER_SIZE);
    sf::Sprite screenSprite(screenTexture);
    screenSprite.setScale((float)WINDOW_SCALE, (float)WINDOW_SCALE);

    std::vector<sf::Uint8> pixelBuffer(BUFFER_SIZE * BUFFER_SIZE * 4, 255);

    d.setScreenSize({ SCREEN_W, SCREEN_H });

    while (window.isOpen() && keep_running) {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed) {
                window.close();
                keep_running = false;
            } else if (event.type == sf::Event::KeyPressed) {
                char k = 0;
                if (event.key.code == sf::Keyboard::Q) k = 'q';
                else if (event.key.code == sf::Keyboard::W) k = 'w';
                else if (event.key.code == sf::Keyboard::E) k = 'e';
                else if (event.key.code == sf::Keyboard::R) k = 'r';
                else if (event.key.code == sf::Keyboard::F) k = 'f';

                if (k != 0) {
                    ui.handleViewKey(k, needFullRedraw);
                }

                // Performance pads A S D / Z X C V
                char perfKey = 0;
                if (event.key.code == sf::Keyboard::A) perfKey = 'a';
                else if (event.key.code == sf::Keyboard::S) perfKey = 's';
                else if (event.key.code == sf::Keyboard::D) perfKey = 'd';
                else if (event.key.code == sf::Keyboard::Z) perfKey = 'z';
                else if (event.key.code == sf::Keyboard::X) perfKey = 'x';
                else if (event.key.code == sf::Keyboard::C) perfKey = 'c';
                else if (event.key.code == sf::Keyboard::V) perfKey = 'v';

                if (perfKey != 0) {
                    ui.handlePerformancePad(perfKey, true, needFullRedraw);
                }

                // Keyboard Encoder emulation (Keys 1..4 + Up/Down arrows)
                if (event.key.code >= sf::Keyboard::Num1 && event.key.code <= sf::Keyboard::Num4) {
                    int encIdx = event.key.code - sf::Keyboard::Num1;
                    int dir = sf::Keyboard::isKeyPressed(sf::Keyboard::LShift) ? -1 : 1;
                    ui.handleEncoderTurn(encIdx, dir, needFullRedraw);
                }
            } else if (event.type == sf::Event::KeyReleased) {
                char perfKey = 0;
                if (event.key.code == sf::Keyboard::A) perfKey = 'a';
                else if (event.key.code == sf::Keyboard::S) perfKey = 's';
                else if (event.key.code == sf::Keyboard::D) perfKey = 'd';
                else if (event.key.code == sf::Keyboard::Z) perfKey = 'z';
                else if (event.key.code == sf::Keyboard::X) perfKey = 'x';
                else if (event.key.code == sf::Keyboard::C) perfKey = 'c';
                else if (event.key.code == sf::Keyboard::V) perfKey = 'v';

                if (perfKey != 0) {
                    ui.handlePerformancePad(perfKey, false, needFullRedraw);
                }
            } else if (event.type == sf::Event::MouseMoved) {
                int mx = event.mouseMove.x / WINDOW_SCALE;
                int my = event.mouseMove.y / WINDOW_SCALE;

                // Track active encoder hover card
                int colW = SCREEN_W / 4;
                if (my >= 12 && my <= 46 && mx >= 0 && mx < SCREEN_W) {
                    ui.activeEncoderHover = std::clamp(mx / colW, 0, 3);
                } else {
                    ui.activeEncoderHover = -1;
                }
            } else if (event.type == sf::Event::MouseWheelScrolled) {
                int mx = event.mouseWheelScroll.x / WINDOW_SCALE;
                int my = event.mouseWheelScroll.y / WINDOW_SCALE;
                int colW = SCREEN_W / 4;
                int encIdx = mx / colW;
                if (encIdx >= 0 && encIdx < 4 && my <= 48) {
                    int dir = (event.mouseWheelScroll.delta > 0) ? 1 : -1;
                    ui.handleEncoderTurn(encIdx, dir, needFullRedraw);
                }
            }
        }

        // Draw Frame Buffer
        if (ui.drawUI(d, SCREEN_W, SCREEN_H, needFullRedraw)) {
            for (unsigned y = 0; y < SCREEN_H; y++) {
                std::memcpy(&pixelBuffer[y * BUFFER_SIZE * 4], d.screenBuffer[y], SCREEN_W * 4);
            }
            screenTexture.update(pixelBuffer.data());
        }

        window.clear();
        window.draw(screenSprite);
        window.display();
    }
}
