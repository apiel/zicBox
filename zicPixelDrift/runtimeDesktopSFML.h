#pragma once

#include <SFML/Graphics.hpp>
#include <cstdlib>
#include <iostream>
#include <memory>
#include <string>

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

                if (k != 0) {
                    ui.handleViewKey(k, needFullRedraw);
                }

                // Performance pads A S D F / Z X C V
                char perfKey = 0;
                if (event.key.code == sf::Keyboard::A) perfKey = 'a';
                else if (event.key.code == sf::Keyboard::S) perfKey = 's';
                else if (event.key.code == sf::Keyboard::D) perfKey = 'd';
                else if (event.key.code == sf::Keyboard::F) perfKey = 'f';
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
            } else if (event.type == sf::Event::MouseMoved) {
                int mx = event.mouseMove.x / WINDOW_SCALE;
                int my = event.mouseMove.y / WINDOW_SCALE;

                // Track active encoder hover card
                int colW = (SCREEN_W - 20) / 4;
                if (my >= 35 && my <= 130 && mx >= 10 && mx <= SCREEN_W - 10) {
                    ui.activeEncoderHover = (mx - 10) / colW;
                } else {
                    ui.activeEncoderHover = -1;
                }
            } else if (event.type == sf::Event::MouseWheelScrolled) {
                int mx = event.mouseWheelScroll.x / WINDOW_SCALE;
                int colW = (SCREEN_W - 20) / 4;
                int encIdx = (mx - 10) / colW;
                if (encIdx >= 0 && encIdx < 4) {
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
