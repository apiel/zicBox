#pragma once

#include <SFML/Graphics.hpp>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <memory>
#include <string>
#include <vector>

#include "draw/draw.h"
#include "ui.h"

#ifndef BUFFER_SIZE
#define BUFFER_SIZE 4096
#endif

extern std::atomic<bool> keep_running;

void runDesktopSFML(Draw& d, UiKick& ui, bool& needFullRedraw)
{
    const int SCREEN_W = 640;
    const int SCREEN_H = 360;

    // Screenshot pipeline check
    const char* screenshotEnv = std::getenv("ZIC_SCREENSHOT");
    if (screenshotEnv != nullptr) {
        std::string basePath(screenshotEnv);
        std::vector<sf::Uint8> screenshotPixels(SCREEN_W * SCREEN_H * 4);

        d.setScreenSize({ SCREEN_W, SCREEN_H });

        if (ui.drawUI(d, SCREEN_W, SCREEN_H, needFullRedraw)) {
            for (unsigned y = 0; y < SCREEN_H; y++) {
                std::memcpy(&screenshotPixels[y * SCREEN_W * 4], d.screenBuffer[y], SCREEN_W * 4);
            }
            sf::Image screenshot;
            screenshot.create(SCREEN_W, SCREEN_H, screenshotPixels.data());
            screenshot.saveToFile(basePath + ".png");
            logInfo("Saved screenshot to %s.png", basePath.c_str());
        }
        return;
    }

    const int WINDOW_SCALE = 1;

    sf::RenderWindow window(sf::VideoMode(SCREEN_W * WINDOW_SCALE, SCREEN_H * WINDOW_SCALE), "zicKick - Wavetable Kick Synth (64-Step • 170 BPM)");
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
            } else if (event.type == sf::Event::MouseButtonPressed) {
                if (event.mouseButton.button == sf::Mouse::Left) {
                    sf::Vector2i mousePos = sf::Mouse::getPosition(window);
                    ui.handleMouseClick(mousePos.x, mousePos.y, needFullRedraw);
                }
            } else if (event.type == sf::Event::KeyPressed) {
                bool isCHeld = sf::Keyboard::isKeyPressed(sf::Keyboard::C);

                if (event.key.code == sf::Keyboard::Z) {
                    if (isCHeld) {
                        // c + z combo: Start or Stop Sequencer
                        if (!ui.worker.playing) {
                            ui.worker.startSequencer();
                        } else {
                            ui.worker.stopSequencer();
                        }
                    } else {
                        // z alone: Trigger Kick when stopped, Momentary Mute when playing
                        if (!ui.worker.playing) {
                            ui.worker.triggerKick();
                        } else {
                            ui.worker.isMuted = true;
                        }
                    }
                    needFullRedraw = true;
                } else if (event.key.code == sf::Keyboard::X) {
                    // x alone: Trigger Kick when stopped, Momentary Repeat when playing
                    if (!ui.worker.playing) {
                        ui.worker.triggerKick();
                    } else {
                        ui.worker.isRepeat = true;
                    }
                    needFullRedraw = true;
                } else if (event.key.code == sf::Keyboard::Up || event.key.code == sf::Keyboard::Right) {
                    ui.handleEncoderTurn(1, needFullRedraw);
                } else if (event.key.code == sf::Keyboard::Down || event.key.code == sf::Keyboard::Left) {
                    ui.handleEncoderTurn(-1, needFullRedraw);
                } else if (event.key.code == sf::Keyboard::Return) {
                    ui.handleEncoderPush(needFullRedraw);
                } else if (event.key.code == sf::Keyboard::Q) {
                    ui.worker.toggleAutoMorph();
                    needFullRedraw = true;
                } else if (event.key.code == sf::Keyboard::Tab || event.key.code == sf::Keyboard::Num3) {
                    ui.handleButton3(needFullRedraw);
                }
            } else if (event.type == sf::Event::KeyReleased) {
                if (event.key.code == sf::Keyboard::Z) {
                    ui.worker.isMuted = false;
                    needFullRedraw = true;
                } else if (event.key.code == sf::Keyboard::X) {
                    ui.worker.isRepeat = false;
                    needFullRedraw = true;
                }
            } else if (event.type == sf::Event::MouseWheelScrolled) {
                sf::Vector2i mousePos = sf::Mouse::getPosition(window);
                int delta = (event.mouseWheelScroll.delta > 0) ? 1 : -1;
                ui.handleMouseScroll(mousePos.x, mousePos.y, delta, needFullRedraw);
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
