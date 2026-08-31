#pragma once

#include <SFML/Graphics.hpp>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <memory>
#include <string>
#include <vector>

#include "draw/draw.h"
#include "studio.h"
#include "ui.h"

#ifndef BUFFER_SIZE
#define BUFFER_SIZE 4096
#endif

extern std::atomic<bool> keep_running;

void runDesktopSFML(Draw& d, UiPixMini& ui, bool& needFullRedraw)
{
    const int SCREEN_W = 240;
    const int SCREEN_H = 320;

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

        ui.isShuttingDown = true;
        ui.renderedGoodbye = false;
        if (ui.drawUI(d, SCREEN_W, SCREEN_H, needFullRedraw)) {
            for (unsigned y = 0; y < SCREEN_H; y++) {
                std::memcpy(&screenshotPixels[y * SCREEN_W * 4], d.screenBuffer[y], SCREEN_W * 4);
            }
            sf::Image screenshot;
            screenshot.create(SCREEN_W, SCREEN_H, screenshotPixels.data());
            screenshot.saveToFile(basePath + "_shutdown.png");
        }
        ui.isShuttingDown = false;
        ui.renderedGoodbye = false;

        return;
    }

    const int WINDOW_SCALE = 2;

    sf::RenderWindow window(sf::VideoMode(SCREEN_W * WINDOW_SCALE, SCREEN_H * WINDOW_SCALE), "zicPixMini (Desktop Emulation)");
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
                // Button 1..6 mappings (Z, X, C, V, B, N or Num1..6)
                if (event.key.code == sf::Keyboard::Z) ui.handleButtonKey('1', true, needFullRedraw);
                else if (event.key.code == sf::Keyboard::X) ui.handleButtonKey('2', true, needFullRedraw);
                else if (event.key.code == sf::Keyboard::C) ui.handleButtonKey('3', true, needFullRedraw);
                else if (event.key.code == sf::Keyboard::V) ui.handleButtonKey('4', true, needFullRedraw);
                else if (event.key.code == sf::Keyboard::B) ui.handleButtonKey('5', true, needFullRedraw);
                else if (event.key.code == sf::Keyboard::N) ui.handleButtonKey('6', true, needFullRedraw);
                else if (event.key.code == sf::Keyboard::P) ui.handleButtonKey('p', true, needFullRedraw);

                // Encoder Push 1..4 mappings (Q, W, E, R)
                else if (event.key.code == sf::Keyboard::Q) ui.handleEncoderPush(0, true, needFullRedraw);
                else if (event.key.code == sf::Keyboard::W) ui.handleEncoderPush(1, true, needFullRedraw);
                else if (event.key.code == sf::Keyboard::E) ui.handleEncoderPush(2, true, needFullRedraw);
                else if (event.key.code == sf::Keyboard::R) ui.handleEncoderPush(3, true, needFullRedraw);

                // Encoder Turn emulation (Keys 1..4)
                else if (event.key.code >= sf::Keyboard::Num1 && event.key.code <= sf::Keyboard::Num4) {
                    int encIdx = event.key.code - sf::Keyboard::Num1;
                    int dir = sf::Keyboard::isKeyPressed(sf::Keyboard::LShift) ? -1 : 1;
                    ui.handleEncoderTurn(encIdx, dir, needFullRedraw);
                }
            } else if (event.type == sf::Event::MouseMoved) {
                int mx = event.mouseMove.x / WINDOW_SCALE;
                int my = event.mouseMove.y / WINDOW_SCALE;

                if (my >= 28 && my <= 260) {
                    int col = (mx < 120) ? 0 : 1;
                    int row = (my < 145) ? 0 : 1;
                    ui.activeEncoderHover = row * 2 + col;
                } else {
                    ui.activeEncoderHover = -1;
                }
            } else if (event.type == sf::Event::MouseWheelScrolled) {
                int mx = event.mouseWheelScroll.x / WINDOW_SCALE;
                int my = event.mouseWheelScroll.y / WINDOW_SCALE;
                if (my >= 28 && my <= 260) {
                    int col = (mx < 120) ? 0 : 1;
                    int row = (my < 145) ? 0 : 1;
                    int encIdx = row * 2 + col;
                    int dir = (event.mouseWheelScroll.delta > 0) ? 1 : -1;
                    ui.handleEncoderTurn(encIdx, dir, needFullRedraw);
                }
            }
        }

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
