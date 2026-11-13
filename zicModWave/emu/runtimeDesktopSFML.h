#pragma once

#include <SFML/Graphics.hpp>
#include <chrono>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <mutex>
#include <string>

#include "draw/draw.h"
#include "uiModWave.h"

static constexpr int BUFFER_SIZE = 2048;
extern std::mutex audioMutex;

inline void runDesktopSFML(Draw& d, bool& needRedraw, UiModWave& ui)
{
    sf::RenderWindow window(sf::VideoMode(700, 740), "zicModWave - Wavetable & VA Synthesizer Module");
    window.setFramerateLimit(60);
    window.setKeyRepeatEnabled(false);

    sf::Texture screenTexture;
    screenTexture.create(BUFFER_SIZE, BUFFER_SIZE);
    sf::Sprite screenSprite(screenTexture);
    std::vector<sf::Uint8> pixelBuffer(BUFFER_SIZE * BUFFER_SIZE * 4, 15);

    sf::Vector2u winSize = window.getSize();

    bool isDraggingPot = false;
    PotIndex activeDragPot = POT_PITCH;
    int dragStartY = 0;
    float dragStartVal = 0.0f;

    while (window.isOpen()) {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed) {
                window.close();
            } else if (event.type == sf::Event::KeyPressed) {
                std::lock_guard<std::mutex> lock(audioMutex);
                switch (event.key.code) {
                    case sf::Keyboard::Up:
                    case sf::Keyboard::Right:
                        ui.handleEncoderTurn(1);
                        needRedraw = true;
                        break;
                    case sf::Keyboard::Down:
                    case sf::Keyboard::Left:
                        ui.handleEncoderTurn(-1);
                        needRedraw = true;
                        break;
                    case sf::Keyboard::Enter:
                    case sf::Keyboard::Space:
                    case sf::Keyboard::C:
                        ui.handleEncoderClick();
                        needRedraw = true;
                        break;
                    case sf::Keyboard::Num1:
                        ui.applyPotValue(POT_PITCH, std::fmod(ui.app.potValues[POT_PITCH] + 0.2f, 1.05f));
                        needRedraw = true;
                        break;
                    case sf::Keyboard::Num2:
                        ui.applyPotValue(POT_WAVE, std::fmod(ui.app.potValues[POT_WAVE] + 0.2f, 1.05f));
                        needRedraw = true;
                        break;
                    case sf::Keyboard::Num3:
                        ui.applyPotValue(POT_CUTOFF, std::fmod(ui.app.potValues[POT_CUTOFF] + 0.2f, 1.05f));
                        needRedraw = true;
                        break;
                    case sf::Keyboard::Num4:
                        ui.applyPotValue(POT_RESONANCE, std::fmod(ui.app.potValues[POT_RESONANCE] + 0.2f, 1.05f));
                        needRedraw = true;
                        break;
                    case sf::Keyboard::Num5:
                        ui.applyPotValue(POT_RELEASE, std::fmod(ui.app.potValues[POT_RELEASE] + 0.2f, 1.05f));
                        needRedraw = true;
                        break;
                    case sf::Keyboard::Num6:
                        ui.applyPotValue(POT_ENV_AMT, std::fmod(ui.app.potValues[POT_ENV_AMT] + 0.2f, 1.05f));
                        needRedraw = true;
                        break;
                    case sf::Keyboard::Num7:
                        ui.applyPotValue(POT_FILT_MORPH, std::fmod(ui.app.potValues[POT_FILT_MORPH] + 0.2f, 1.05f));
                        needRedraw = true;
                        break;
                    case sf::Keyboard::Num8:
                        ui.applyPotValue(POT_CRUSH_FM, std::fmod(ui.app.potValues[POT_CRUSH_FM] + 0.2f, 1.05f));
                        needRedraw = true;
                        break;
                    case sf::Keyboard::Num9:
                        ui.applyPotValue(POT_MOD_DEPTH, std::fmod(ui.app.potValues[POT_MOD_DEPTH] + 0.2f, 1.05f));
                        needRedraw = true;
                        break;
                    case sf::Keyboard::Num0:
                        ui.applyPotValue(POT_MOD_SPEED, std::fmod(ui.app.potValues[POT_MOD_SPEED] + 0.2f, 1.05f));
                        needRedraw = true;
                        break;
                    default:
                        break;
                }
            } else if (event.type == sf::Event::MouseWheelScrolled) {
                std::lock_guard<std::mutex> lock(audioMutex);
                float mx = (float)event.mouseWheelScroll.x;
                float my = (float)event.mouseWheelScroll.y;
                float delta = event.mouseWheelScroll.delta;

                struct PotPos { PotIndex idx; float x1; float y1; float x2; float y2; };
                PotPos pots[10] = {
                    { POT_PITCH,       30, 415, 180, 510 },
                    { POT_WAVE,       193, 415, 343, 510 },
                    { POT_CUTOFF,     356, 415, 506, 510 },
                    { POT_RESONANCE,   30, 520, 180, 615 },
                    { POT_RELEASE,    193, 520, 343, 615 },
                    { POT_ENV_AMT,    356, 520, 506, 615 },
                    { POT_FILT_MORPH,  30, 625, 180, 720 },
                    { POT_CRUSH_FM,   193, 625, 343, 720 },
                    { POT_MOD_DEPTH,  356, 625, 506, 720 },
                    { POT_MOD_SPEED,  519, 625, 669, 720 }
                };

                bool scrolledOverPot = false;
                for (int i = 0; i < 10; ++i) {
                    if (mx >= pots[i].x1 && mx <= pots[i].x2 && my >= pots[i].y1 && my <= pots[i].y2) {
                        float step = (delta > 0) ? 0.02f : -0.02f;
                        float curVal = ui.app.potValues[pots[i].idx];
                        ui.applyPotValue(pots[i].idx, curVal + step);
                        scrolledOverPot = true;
                        break;
                    }
                }

                if (!scrolledOverPot) {
                    int dir = (delta > 0) ? 1 : -1;
                    ui.handleEncoderTurn(dir);
                }
                needRedraw = true;
            } else if (event.type == sf::Event::MouseButtonPressed) {
                if (event.mouseButton.button == sf::Mouse::Left) {
                    int mx = event.mouseButton.x;
                    int my = event.mouseButton.y;

                    std::lock_guard<std::mutex> lock(audioMutex);

                    // Encoder Click button (Row 2, Col 4 button: x=529..659, y=552..584)
                    if (mx >= 529 && mx <= 659 && my >= 552 && my <= 584) {
                        ui.handleEncoderClick();
                        needRedraw = true;
                    }

                    struct PotClickArea { PotIndex idx; int x1, y1, x2, y2; };
                    PotClickArea pots[10] = {
                        { POT_PITCH,       30, 415, 180, 510 },
                        { POT_WAVE,       193, 415, 343, 510 },
                        { POT_CUTOFF,     356, 415, 506, 510 },
                        { POT_RESONANCE,   30, 520, 180, 615 },
                        { POT_RELEASE,    193, 520, 343, 615 },
                        { POT_ENV_AMT,    356, 520, 506, 615 },
                        { POT_FILT_MORPH,  30, 625, 180, 720 },
                        { POT_CRUSH_FM,   193, 625, 343, 720 },
                        { POT_MOD_DEPTH,  356, 625, 506, 720 },
                        { POT_MOD_SPEED,  519, 625, 669, 720 }
                    };

                    for (int i = 0; i < 10; ++i) {
                        if (mx >= pots[i].x1 && mx <= pots[i].x2 && my >= pots[i].y1 && my <= pots[i].y2) {
                            isDraggingPot = true;
                            activeDragPot = pots[i].idx;
                            dragStartY = my;
                            dragStartVal = ui.app.potValues[pots[i].idx];
                            break;
                        }
                    }
                }
            } else if (event.type == sf::Event::MouseButtonReleased) {
                if (event.mouseButton.button == sf::Mouse::Left) {
                    isDraggingPot = false;
                }
            } else if (event.type == sf::Event::MouseMoved) {
                if (isDraggingPot) {
                    int deltaY = dragStartY - event.mouseMove.y;
                    float newVal = dragStartVal + (float)deltaY * 0.005f;
                    std::lock_guard<std::mutex> lock(audioMutex);
                    ui.applyPotValue(activeDragPot, newVal);
                    needRedraw = true;
                }
            }
        }

        winSize = window.getSize();
        d.setScreenSize({ (int)winSize.x, (int)winSize.y });

        if (ui.draw(d, winSize.x, winSize.y, needRedraw)) {
            for (unsigned int y = 0; y < winSize.y; ++y) {
                std::memcpy(&pixelBuffer[y * BUFFER_SIZE * 4], d.screenBuffer[y], winSize.x * 4);
            }
            screenTexture.update(pixelBuffer.data());
        }

        window.clear();
        window.draw(screenSprite);
        window.display();
    }
}
