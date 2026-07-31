#pragma once

#include <SFML/Graphics.hpp>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <string>

#include "zicGridV2/ViewManager.h"
#include "zicGridV2/audioWorker.h"
#include "zicGridV2/gridState.h"
#include "zicGridV2/studio.h"
#include "zicGridV2/ui.h"

inline void runDesktopSFML(Draw& d, bool& needFullRedraw)
{
    // Check for screenshot environment variable (headless screenshot generation)
    const char* screenshotEnv = std::getenv("ZIC_SCREENSHOT");
    if (screenshotEnv != nullptr) {
        std::string basePath(screenshotEnv);
        sf::Image screenshot;
        std::vector<sf::Uint8> pixelBuf(SCREEN_W * SCREEN_H * 4, 255);

        for (int viewIdx = 0; viewIdx < VIEW_COUNT; ++viewIdx) {
            viewManager.setActiveView(viewIdx);
            needFullRedraw = true;

            d.setScreenSize({ SCREEN_W, SCREEN_H });
            if (drawUI(d, SCREEN_W, SCREEN_H, needFullRedraw)) {
                for (unsigned y = 0; y < SCREEN_H; y++) {
                    std::memcpy(&pixelBuf[y * SCREEN_W * 4], d.screenBuffer[y], SCREEN_W * 4);
                }
                screenshot.create(SCREEN_W, SCREEN_H, pixelBuf.data());
                std::string filePath = basePath + "_" + std::to_string(viewIdx) + ".png";
                screenshot.saveToFile(filePath);
                std::cout << "[Screenshot] Generated: " << filePath << std::endl;
            }
        }
        return;
    }

    sf::RenderWindow window(sf::VideoMode(SCREEN_W, SCREEN_H), "Zic Grid V2 - 480x640 (48 Pads & 12 Encoders)");
    window.setFramerateLimit(60);
    window.setKeyRepeatEnabled(false);

    sf::Texture screenTexture;
    screenTexture.create(BUFFER_SIZE, BUFFER_SIZE);
    sf::Sprite screenSprite(screenTexture);
    std::vector<sf::Uint8> pixelBuffer(BUFFER_SIZE * BUFFER_SIZE * 4, 15);

    sf::Vector2u winSize = window.getSize();

    while (window.isOpen() && keep_running) {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed) {
                window.close();
                keep_running = false;
            } else if (event.type == sf::Event::Resized) {
                window.setView(sf::View(sf::FloatRect(0, 0, (float)event.size.width, (float)event.size.height)));
                needFullRedraw = true;
            } else if (event.type == sf::Event::KeyPressed) {
                if (event.key.code == sf::Keyboard::Space) {
                    studio.isPlaying = !studio.isPlaying;
                    gridState.utility.playActive = studio.isPlaying;
                } else if (event.key.code == sf::Keyboard::Q) {
                    viewManager.setActiveView(VIEW_STEP_SEQ);
                } else if (event.key.code == sf::Keyboard::W) {
                    viewManager.setActiveView(VIEW_INSTRUMENT);
                } else if (event.key.code == sf::Keyboard::E) {
                    viewManager.setActiveView(VIEW_KEYBOARD);
                } else if (event.key.code == sf::Keyboard::R) {
                    viewManager.setActiveView(VIEW_CLIP_LAUNCH);
                } else if (event.key.code >= sf::Keyboard::Num1 && event.key.code <= sf::Keyboard::Num8) {
                    int trk = event.key.code - sf::Keyboard::Num1;
                    studio.selTrack = trk;
                    gridState.utility.activeTrack = trk;
                }
            } else if (event.type == sf::Event::MouseButtonPressed) {
                int mx = event.mouseButton.x;
                int my = event.mouseButton.y;

                int encoderH = 165;
                int padMatrixStartY = 26 + encoderH + 20; // 211
                int padMatrixH = 225;
                int globalUtilityStartY = padMatrixStartY + padMatrixH + 28; // 464

                if (my >= globalUtilityStartY) {
                    // Clicked Compact Global Utility Zone (Bottom 4x4 Grid)
                    int padW = (winSize.x - 8) / 4;
                    int padH = 170 / 4;
                    int col = 8 + std::clamp((mx - 4) / std::max(1, padW), 0, 3);
                    int row = std::clamp((my - globalUtilityStartY) / std::max(1, padH), 0, 3);
                    viewManager.handlePadPress(col, row, true);
                } else if (my >= padMatrixStartY && my < padMatrixStartY + padMatrixH) {
                    // Clicked Dynamic 8x4 Pad Matrix
                    int padW = (winSize.x - 8) / 8;
                    int padH = padMatrixH / 4;
                    int col = std::clamp((mx - 4) / std::max(1, padW), 0, 7);
                    int row = std::clamp((my - padMatrixStartY) / std::max(1, padH), 0, 3);
                    viewManager.handlePadPress(col, row, true);
                }
            } else if (event.type == sf::Event::MouseButtonReleased) {
                for (int r = 0; r < PAD_ROWS; ++r) {
                    for (int c = 0; c < PAD_COLS; ++c) {
                        gridState.pads[c][r].pressed = false;
                    }
                }
            } else if (event.type == sf::Event::MouseWheelScrolled) {
                int delta = (event.mouseWheelScroll.delta > 0) ? 1 : -1;
                int mx = event.mouseWheelScroll.x;
                int my = event.mouseWheelScroll.y;

                if (my >= 26 && my < 26 + 165) {
                    // Scrolled over 4x3 Encoder grid on TOP
                    int cardW = (winSize.x - 8) / 4;
                    int cardH = 165 / 3;
                    int c = std::clamp((mx - 4) / std::max(1, cardW), 0, 3);
                    int r = std::clamp((my - 26) / std::max(1, cardH), 0, 2);
                    int encId = r * 4 + c + 1;
                    viewManager.handleEncoder(encId, delta);
                }
            }
        }

        if (needFullRedraw) {
            winSize = window.getSize();
            d.setScreenSize({ (int)winSize.x, (int)winSize.y });
        }

        if (drawUI(d, winSize.x, winSize.y, needFullRedraw)) {
            for (unsigned y = 0; y < winSize.y; y++) {
                std::memcpy(&pixelBuffer[y * BUFFER_SIZE * 4], d.screenBuffer[y], winSize.x * 4);
            }
            screenTexture.update(pixelBuffer.data());
        }

        window.clear();
        window.draw(screenSprite);
        window.display();
    }
}
