#include <algorithm>
#include <chrono>
#include <cmath>
#include <cstring>
#include <iomanip>
#include <iostream>
#include <memory>
#include <pthread.h>
#include <sstream>
#include <thread>
#include <vector>

#include "audio/Eq.h"
#include "helpers/enc.h"
#include "helpers/midiNote.h"
#include "zicRack/audioWorker.h"
#include "zicRack/generator.h"
#include "zicRack/studio.h"
#include "zicRack/uiMasterFx.h"
#include "zicRack/uiTopBar.h"

void drawStaticUI(Draw& d, sf::Vector2u size, bool& needFullRedraw)
{
    if (needFullRedraw) d.clear();
    const int winW = (int)size.x;

    int currentY = 0;
    currentY += TopBar::draw(d, winW, needFullRedraw);
    currentY += 10;

    drawMasterFxUI(d, size, currentY);


    needFullRedraw = false;
}

int main()
{
    snd_pcm_t* pcm_h = audioInit();
    pthread_setname_np(pthread_self(), "zicBox_UI");
    sf::RenderWindow window(sf::VideoMode(800, 480), "ZicRack");
    window.setFramerateLimit(60);
    window.setKeyRepeatEnabled(false);

    Styles appStyles = {
        .screen = { 800, 480 }, .margin = 2, .colors = { { 15, 15, 18 }, { 255, 255, 255 }, { 120, 120, 130 }, { 0, 180, 255 }, { 10, 10, 12 }, { 28, 28, 32 }, { 35, 35, 40 } }
    };
    auto drawer = std::make_unique<Draw>(appStyles);
    sf::Texture screenTexture;
    screenTexture.create(BUFFER_SIZE, BUFFER_SIZE);
    sf::Sprite screenSprite(screenTexture);
    std::vector<sf::Uint8> pixelBuffer(BUFFER_SIZE * BUFFER_SIZE * 4, 15);

    bool needFullRedraw = true;
    bool needRedraw = true;
    std::thread aThread(audioWorker, pcm_h);
    pthread_setname_np(aThread.native_handle(), "zicBox_Audio");

    while (window.isOpen() && keep_running) {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed) window.close();
            if (event.type == sf::Event::Resized) {
                window.setView(sf::View(sf::FloatRect(0, 0, (float)event.size.width, (float)event.size.height)));
                needRedraw = true;
                needFullRedraw = true;
            }

            if (event.type == sf::Event::MouseMoved) {
                float mx = (float)event.mouseMove.x, my = (float)event.mouseMove.y;
                if (filterDragging) {
                    studio.filter.setCutoff(std::clamp(((mx - filterPadRect.left) / filterPadRect.width) * 2.0f - 1.0f, -1.0f, 1.0f));
                    studio.filter.setResonance(std::clamp(1.0f - ((my - filterPadRect.top) / filterPadRect.height), 0.0f, 1.0f));
                    needRedraw = true;
                }
            }

            if (event.type == sf::Event::KeyReleased) {
                if (event.key.code >= sf::Keyboard::Num1 && event.key.code <= sf::Keyboard::Num6) {
                    int trkIdx = event.key.code - sf::Keyboard::Num1;
                    int note = (studio.selTrack == trkIdx && studio.selStep != -1) ? studio.tracks[trkIdx]->sequence[studio.selStep].note : 60;
                    std::lock_guard<std::mutex> lock(studio.audioMutex);
                    studio.tracks[trkIdx]->engine->noteOff(note);
                }
                if (event.key.code >= sf::Keyboard::F1 && event.key.code <= sf::Keyboard::F12) studio.activeScatterMode = 0;
            }
            if (event.type == sf::Event::KeyPressed) {
                if (event.key.code >= sf::Keyboard::F1 && event.key.code <= sf::Keyboard::F12) studio.activeScatterMode = (event.key.code - sf::Keyboard::F1) + 1;
                if (event.key.code == sf::Keyboard::H || (showHelp && event.key.code == sf::Keyboard::Escape)) {
                    showHelp = !showHelp;
                    needRedraw = true;
                }
                if (event.key.code == sf::Keyboard::Space) {
                    studio.isPlaying = !studio.isPlaying;
                    needRedraw = true;
                }

                if (event.key.code >= sf::Keyboard::Num1 && event.key.code <= sf::Keyboard::Num6) {
                    int trkIdx = event.key.code - sf::Keyboard::Num1;
                    if (sf::Keyboard::isKeyPressed(sf::Keyboard::E)) {
                        studio.pianoRollTrack = trkIdx;
                        needRedraw = true;
                    } else if (sf::Keyboard::isKeyPressed(sf::Keyboard::M)) {
                        studio.tracks[trkIdx]->isMuted = !studio.tracks[trkIdx]->isMuted;
                        needRedraw = true;
                    } else if (sf::Keyboard::isKeyPressed(sf::Keyboard::LShift)) {
                        studio.selTrack = trkIdx;
                        studio.selStep = 0;
                        needRedraw = true;
                    } else {
                        int note = (studio.selTrack == trkIdx && studio.selStep != -1) ? studio.tracks[trkIdx]->sequence[studio.selStep].note : 60;
                        std::lock_guard<std::mutex> lock(studio.audioMutex);
                        studio.tracks[trkIdx]->engine->noteOn(note, 1.0f);
                    }
                }
            }
            if (event.type == sf::Event::MouseButtonPressed) {
                int mx = event.mouseButton.x, my = event.mouseButton.y;
                if (showHelp) {
                    if (helpCloseRect.contains(mx, my) || !studio.overlayRect.contains(mx, my)) {
                        showHelp = false;
                        needRedraw = true;
                    }
                    continue;
                }

                TopBar::mouseButtonPressed({ mx, my }, needRedraw);

                if (filterPadRect.contains(mx, my)) {
                    filterDragging = true;
                    studio.filter.setCutoff(std::clamp(((float)(mx - filterPadRect.left) / filterPadRect.width) * 2.0f - 1.0f, -1.0f, 1.0f));
                    studio.filter.setResonance(std::clamp(1.0f - ((float)(my - filterPadRect.top) / filterPadRect.height), 0.0f, 1.0f));
                    needRedraw = true;
                }
            }
            if (event.type == sf::Event::MouseWheelScrolled) {
                if (showHelp) continue;

                int mx = event.mouseWheelScroll.x, my = event.mouseWheelScroll.y;
                float delta = event.mouseWheelScroll.delta;
                uint32_t now = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now().time_since_epoch()).count();

                if (!TopBar::mouseWheelScrolled({ mx, my }, delta, needRedraw, now)) {
                    for (int i = 0; i < 4; i++) {
                        if (compRects[i].contains(mx, my)) {
                            float step = (sf::Keyboard::isKeyPressed(sf::Keyboard::LShift)) ? 5.0f : 1.0f;
                            if (i == 0) studio.compressor.threshold = std::clamp(studio.compressor.threshold + (delta > 0 ? step : -step), -60.0f, 0.0f);
                            else if (i == 1) studio.compressor.ratio = std::clamp(studio.compressor.ratio + (delta > 0 ? 0.5f : -0.5f), 1.0f, 20.0f);
                            else if (i == 2) studio.compressor.attack = std::clamp(studio.compressor.attack + (delta > 0 ? 0.001f : -0.001f), 0.0001f, 0.1f);
                            else if (i == 3) studio.compressor.release = std::clamp(studio.compressor.release + (delta > 0 ? 0.01f : -0.01f), 0.01f, 1.0f);
                            needRedraw = true;
                        }
                    }
                }
            }
        }

        if (needRedraw) {
            sf::Vector2u winSize = window.getSize();
            drawer->setScreenSize({ (int)winSize.x, (int)winSize.y });
            drawStaticUI(*drawer, winSize, needFullRedraw);
            for (unsigned y = 0; y < winSize.y; y++)
                std::memcpy(&pixelBuffer[y * BUFFER_SIZE * 4], drawer->screenBuffer[y], winSize.x * 4);
            needRedraw = false;
        }

        updateCompressorMeter(pixelBuffer, BUFFER_SIZE);

        screenTexture.update(pixelBuffer.data());
        window.clear();
        window.draw(screenSprite);
        window.display();
    }

    keep_running = false;
    aThread.join();
    snd_pcm_close(pcm_h);
    return 0;
}