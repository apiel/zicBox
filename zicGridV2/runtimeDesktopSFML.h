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
            ViewManager::setActiveView(viewIdx);
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
            } else if (event.type == sf::Event::TextEntered) {
                if (ViewManager::getActiveViewIdx() == VIEW_PROJECT) {
                    auto projView = std::dynamic_pointer_cast<ProjectView>(ViewManager::getActiveView());
                    if (projView) {
                        projView->handleCharTyped((char)event.text.unicode);
                        needFullRedraw = true;
                    }
                }
            } else if (event.type == sf::Event::KeyPressed) {
                if (ViewManager::getActiveViewIdx() == VIEW_PROJECT) {
                    auto projView = std::dynamic_pointer_cast<ProjectView>(ViewManager::getActiveView());
                    if (projView && projView->getCurrentMode() != ProjectView::VIEW_LIST) {
                        if (event.key.code == sf::Keyboard::BackSpace) {
                            projView->handleKeyInput(8);
                            needFullRedraw = true;
                        } else if (event.key.code == sf::Keyboard::Return) {
                            projView->handleKeyInput(13);
                            needFullRedraw = true;
                        } else if (event.key.code == sf::Keyboard::Escape) {
                            projView->handleKeyInput(27);
                            needFullRedraw = true;
                        }
                        // Skip pad shortcut processing while typing text in NEW/RENAME mode
                        continue;
                    }
                }

                if (event.key.code == sf::Keyboard::Space) {
                    int trk = studio.selTrack;
                    auto& t = studio.tracks[trk];
                    if (t && t->engine) {
                        std::lock_guard<std::mutex> lock(studio.audioMutex);
                        t->engine->noteOn(60, 0.9f); // C4
                    }
                } else {
                    int col = -1, row = -1;
                    switch (event.key.code) {
                        case sf::Keyboard::Num1: col = 8;  row = 0; break;
                        case sf::Keyboard::Num2: col = 9;  row = 0; break;
                        case sf::Keyboard::Num3: col = 10; row = 0; break;
                        case sf::Keyboard::Num4: col = 11; row = 0; break;

                        case sf::Keyboard::Q: col = 8;  row = 1; break;
                        case sf::Keyboard::W: col = 9;  row = 1; break;
                        case sf::Keyboard::E: col = 10; row = 1; break;
                        case sf::Keyboard::R: col = 11; row = 1; break;

                        case sf::Keyboard::A: col = 8;  row = 2; break;
                        case sf::Keyboard::S: col = 9;  row = 2; break;
                        case sf::Keyboard::D: col = 10; row = 2; break;
                        case sf::Keyboard::F: col = 11; row = 2; break;

                        case sf::Keyboard::Z: col = 8;  row = 3; break;
                        case sf::Keyboard::X: col = 9;  row = 3; break;
                        case sf::Keyboard::C: col = 10; row = 3; break;
                        case sf::Keyboard::V: col = 11; row = 3; break;
                        default: break;
                    }
                    if (col >= 0 && row >= 0) {
                        gridState.pads[col][row].pressed = true;
                        ViewManager::handlePadPress(col, row, true);
                    }
                }
            } else if (event.type == sf::Event::KeyReleased) {
                if (ViewManager::getActiveViewIdx() == VIEW_PROJECT) {
                    auto projView = std::dynamic_pointer_cast<ProjectView>(ViewManager::getActiveView());
                    if (projView && projView->getCurrentMode() != ProjectView::VIEW_LIST) {
                        // Skip pad shortcut release processing while typing text in NEW/RENAME mode
                        continue;
                    }
                }

                if (event.key.code == sf::Keyboard::Space) {
                    int trk = studio.selTrack;
                    auto& t = studio.tracks[trk];
                    if (t && t->engine) {
                        std::lock_guard<std::mutex> lock(studio.audioMutex);
                        t->engine->noteOff(60); // C4
                    }
                } else {
                    int col = -1, row = -1;
                    switch (event.key.code) {
                        case sf::Keyboard::Num1: col = 8;  row = 0; break;
                        case sf::Keyboard::Num2: col = 9;  row = 0; break;
                        case sf::Keyboard::Num3: col = 10; row = 0; break;
                        case sf::Keyboard::Num4: col = 11; row = 0; break;

                        case sf::Keyboard::Q: col = 8;  row = 1; break;
                        case sf::Keyboard::W: col = 9;  row = 1; break;
                        case sf::Keyboard::E: col = 10; row = 1; break;
                        case sf::Keyboard::R: col = 11; row = 1; break;

                        case sf::Keyboard::A: col = 8;  row = 2; break;
                        case sf::Keyboard::S: col = 9;  row = 2; break;
                        case sf::Keyboard::D: col = 10; row = 2; break;
                        case sf::Keyboard::F: col = 11; row = 2; break;

                        case sf::Keyboard::Z: col = 8;  row = 3; break;
                        case sf::Keyboard::X: col = 9;  row = 3; break;
                        case sf::Keyboard::C: col = 10; row = 3; break;
                        case sf::Keyboard::V: col = 11; row = 3; break;
                        default: break;
                    }
                    if (col >= 0 && row >= 0) {
                        gridState.pads[col][row].pressed = false;
                        ViewManager::handlePadPress(col, row, false);
                    }
                }
            } else if (event.type == sf::Event::MouseButtonPressed) {
                int mx = event.mouseButton.x;
                int my = event.mouseButton.y;

                int w = winSize.x;
                int h = winSize.y;
                int margin = 4;
                int usableW = w - margin * 2;

                int encoderH = 3 * UiDraw::ROW_H; // 108
                int padGridH = 130;
                int padGridY = h - padGridH - margin;

                int padMatrixW = (usableW * DYNAMIC_PAD_COLS) / PAD_COLS;
                int globalUtilityW = usableW - padMatrixW;

                if (my >= padGridY && my < padGridY + padGridH) {
                    if (mx >= margin && mx < margin + padMatrixW) {
                        // Clicked Dynamic 8x4 Pad Matrix
                        int padW = padMatrixW / DYNAMIC_PAD_COLS;
                        int padH = padGridH / PAD_ROWS;
                        int col = std::clamp((mx - margin) / std::max(1, padW), 0, DYNAMIC_PAD_COLS - 1);
                        int row = std::clamp((my - padGridY) / std::max(1, padH), 0, PAD_ROWS - 1);
                        gridState.pads[col][row].pressed = true;
                        ViewManager::handlePadPress(col, row, true);
                    } else if (mx >= margin + padMatrixW && mx < margin + usableW) {
                        // Clicked Global Utility Zone (cols 8..11)
                        int padW = globalUtilityW / GLOBAL_PAD_COLS;
                        int padH = padGridH / PAD_ROWS;
                        int col = 8 + std::clamp((mx - (margin + padMatrixW)) / std::max(1, padW), 0, GLOBAL_PAD_COLS - 1);
                        int row = std::clamp((my - padGridY) / std::max(1, padH), 0, PAD_ROWS - 1);
                        gridState.pads[col][row].pressed = true;
                        ViewManager::handlePadPress(col, row, true);
                    }
                } else if (my >= margin && my < margin + encoderH && mx >= margin && mx < margin + usableW) {
                    // Clicked 4x3 Encoder grid
                    int cardW = usableW / ENCODER_COLS;
                    int cardH = encoderH / ENCODER_ROWS;
                    int col = std::clamp((mx - margin) / std::max(1, cardW), 0, ENCODER_COLS - 1);
                    int row = std::clamp((my - margin) / std::max(1, cardH), 0, ENCODER_ROWS - 1);
                    int encId = row * ENCODER_COLS + col + 1;
                    int delta = (event.mouseButton.button == sf::Mouse::Right) ? -1 : 1;
                    ViewManager::handleEncoder(encId, delta);
                }
            } else if (event.type == sf::Event::MouseButtonReleased) {
                for (int r = 0; r < PAD_ROWS; ++r) {
                    for (int c = 0; c < PAD_COLS; ++c) {
                        if (gridState.pads[c][r].pressed) {
                            gridState.pads[c][r].pressed = false;
                            ViewManager::handlePadPress(c, r, false);
                        }
                    }
                }
            } else if (event.type == sf::Event::MouseWheelScrolled) {
                int mx = event.mouseWheelScroll.x;
                int my = event.mouseWheelScroll.y;

                int w = winSize.x;
                int margin = 4;
                int usableW = w - margin * 2;
                int encoderH = 3 * UiDraw::ROW_H; // 108

                if (my >= margin && my < margin + encoderH && mx >= margin && mx < margin + usableW) {
                    int cardW = usableW / ENCODER_COLS;
                    int cardH = encoderH / ENCODER_ROWS;
                    int col = std::clamp((mx - margin) / std::max(1, cardW), 0, ENCODER_COLS - 1);
                    int row = std::clamp((my - margin) / std::max(1, cardH), 0, ENCODER_ROWS - 1);
                    int encId = row * ENCODER_COLS + col + 1;
                    int delta = (event.mouseWheelScroll.delta > 0) ? 1 : -1;
                    ViewManager::handleEncoder(encId, delta);
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
