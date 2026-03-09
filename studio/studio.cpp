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

#include "helpers/enc.h"
#include "helpers/midiNote.h"
#include "studio/audioWorker.h"
#include "studio/generator.h"
#include "studio/studio.h"

void drawStaticUI(Draw& d, sf::Vector2u size)
{
    d.clear();
    int winW = (int)size.x, margin = 10, rowH = 26;

    d.filledRect({ 0, 0 }, { winW, 25 }, { .color = d.styles.colors.quaternary });

    studio.transportRect = { margin, 4, 60, 17 };
    d.filledRect({ studio.transportRect.left, studio.transportRect.top }, { 60, 17 }, { .color = studio.isPlaying ? Color { 200, 50, 50 } : Color { 50, 200, 50 } });
    d.text({ margin + 6, 7 }, studio.isPlaying ? "STOP" : "PLAY", 8, { .color = { 255, 255, 255 }, .font = &PoppinsLight_8 });

    std::stringstream bss;
    bss << "BPM: " << std::fixed << std::setprecision(1) << studio.bpm.load();
    studio.bpmRect = { winW - 100, 0, 90, 25 };
    d.textRight({ winW - margin, 6 }, bss.str(), 8, { .color = { 255, 255, 255 }, .font = &PoppinsLight_8 });

    int currentY = 35;
    int paramsPerRow = 8, colW = (winW - (margin * 2)) / paramsPerRow;
    auto now = std::chrono::steady_clock::now();

    for (auto& trkPtr : studio.tracks) {
        Track& trk = *trkPtr;
        int startY = currentY;
        d.filledRect({ margin, currentY }, { colW / 2, 12 }, { .color = d.styles.colors.quaternary });
        d.text({ margin + 4, currentY + 1 }, trk.engine->getName(), 8, { .color = trk.themeColor, .font = &PoppinsLight_8 });

        // Removed old GEN button placement from here

        trk.vuRect = sf::IntRect(margin + (colW / 2) + 10, currentY - 2, WAVE_HISTORY, 16);
        currentY += 14;

        Param* params = trk.engine->getParams();
        for (size_t p = 0; p < trk.engine->getParamCount(); p++) {
            int x = margin + ((int)p % paramsPerRow) * colW, y = currentY + ((int)p / paramsPerRow) * rowH;
            d.filledRect({ x, y }, { colW - 2, rowH - 2 }, { .color = d.styles.colors.quaternary });
            d.text({ x + 4, y + 2 }, params[p].label, 12, { .color = d.styles.colors.text, .font = &PoppinsLight_12 });

            if (winW >= 900 || (trk.activeParamIdx == (int)p && std::chrono::duration_cast<std::chrono::milliseconds>(now - trk.lastEditTime).count() < 1500)) {
                std::stringstream ss;
                ss << std::fixed << std::setprecision(1) << params[p].value << params[p].unit;
                d.textRight({ x + colW - 6, y + 2 }, params[p].string ? params[p].string : ss.str(), 8, { .color = { 120, 120, 130 }, .font = &PoppinsLight_8 });
            }
            float pct = (params[p].value - params[p].min) / (params[p].max - params[p].min);
            d.filledRect({ x + 4, y + rowH - 8 }, { (int)((colW - 10) * pct), 3 }, { .color = trk.themeColor });
        }
        int sectionHeight = (((trk.engine->getParamCount() + 7) / 8) * rowH) + 14;
        trk.trackBounds = sf::IntRect(margin, startY, winW - (margin * 2), sectionHeight);
        currentY += sectionHeight - 2;
    }

    currentY += 10;

    // UI Layout Constants for the Mixer Row
    int muteW = 25;
    int volW = 70;
    int genW = 18;
    int spacing = 5;
    int mixerTotalWidth = muteW + spacing + volW + spacing + genW + 10; // Total offset before sequencer

    int stepW = (winW - (margin * 2 + mixerTotalWidth)) / 64, stepH = 14;

    for (int i = 0; i < MAX_TRACKS; i++) {
        Track& trk = *studio.tracks[i];
        int ty = currentY + (i * (stepH + 4));

        // 1. Mute Button
        trk.muteRect = { margin, ty, muteW, stepH };
        d.filledRect({ trk.muteRect.left, trk.muteRect.top }, { muteW, stepH }, { .color = trk.isMuted ? Color { 200, 50, 50 } : Color { 40, 40, 45 } });
        d.text({ trk.muteRect.left + 8, trk.muteRect.top + 1 }, "M", 8, { .color = { 255, 255, 255 }, .font = &PoppinsLight_8 });

        // 2. Volume Bar
        trk.volRect = { trk.muteRect.left + muteW + spacing, ty, volW, stepH };
        d.filledRect({ trk.volRect.left, trk.volRect.top }, { volW, stepH }, { .color = Color { 40, 40, 45 } });
        d.filledRect({ trk.volRect.left, trk.volRect.top + (stepH / 2) - 2 }, { (int)(volW * trk.volume), 4 }, { .color = trk.themeColor });

        // 3. Generator "G" Button - Placed after Volume
        trk.genRect = { trk.volRect.left + volW + spacing, ty, genW, stepH };
        d.filledRect({ trk.genRect.left, trk.genRect.top }, { genW, stepH }, { .color = Color { 60, 60, 75 } });
        d.text({ trk.genRect.left + 5, trk.genRect.top + 1 }, "G", 8, { .color = { 255, 255, 255 }, .font = &PoppinsLight_8 });

        // 4. Sequencer Steps
        int gridStartX = trk.genRect.left + genW + 10;
        for (int s = 0; s < SEQ_STEPS; s++)
            trk.stepRects[s] = { gridStartX + (s * stepW), ty, stepW - 1, stepH };
    }

    currentY += MAX_TRACKS * (stepH + 4) + 10;

    if (studio.selTrack != -1 && studio.selStep != -1) {
        int editorY = currentY;
        auto& s = studio.tracks[studio.selTrack]->sequence[studio.selStep];
        d.text({ margin, editorY }, "EDIT T" + std::to_string(studio.selTrack + 1) + " S" + std::to_string(studio.selStep + 1), 8, { .color = studio.tracks[studio.selTrack]->themeColor, .font = &PoppinsLight_8 });
        studio.editNoteRect = { 100, editorY - 2, 80, 15 };
        d.text({ 100, editorY }, "NOTE: " + std::string(MIDI_NOTES_STR[s.note]), 8, { .color = { 255, 255, 255 }, .font = &PoppinsLight_8 });
        studio.editVeloRect = { 200, editorY - 2, 80, 15 };
        d.text({ 200, editorY }, "VEL: " + std::to_string((int)(s.velocity * 100)) + "%", 8, { .color = { 255, 255, 255 }, .font = &PoppinsLight_8 });
        studio.editProbRect = { 300, editorY - 2, 80, 15 };
        d.text({ 300, editorY }, "PROB: " + std::to_string((int)(s.condition * 100)) + "%", 8, { .color = { 255, 255, 255 }, .font = &PoppinsLight_8 });
    }
}

void updateWaveforms(std::vector<sf::Uint8>& pixels, int stride)
{
    for (auto& trkPtr : studio.tracks) {
        std::lock_guard<std::mutex> hLock(trkPtr->historyMtx);
        for (int x = 0; x < WAVE_HISTORY; x++) {
            int barH = (int)(std::min(trkPtr->history[x], 1.0f) * (trkPtr->vuRect.height / 2));
            for (int y = 0; y < trkPtr->vuRect.height; y++) {
                size_t idx = ((trkPtr->vuRect.top + y) * stride + (trkPtr->vuRect.left + x)) * 4;
                bool isWave = std::abs(y - (trkPtr->vuRect.height / 2)) <= barH;
                pixels[idx] = isWave ? trkPtr->themeColor.r : 20;
                pixels[idx + 1] = isWave ? trkPtr->themeColor.g : 20;
                pixels[idx + 2] = isWave ? trkPtr->themeColor.b : 25;
            }
        }
    }
}

void updateSequencerPixels(std::vector<sf::Uint8>& pixels, int stride)
{
    int stepWidth = studio.tracks[0]->stepRects[0].width + 1;
    double progressInStep = studio.sampleCounter.load() / studio.samplesPerStep;
    int playheadGlobalX = (int)((studio.currentStep + progressInStep) * stepWidth);
    int gridStartX = studio.tracks[0]->stepRects[0].left;

    for (int t = 0; t < MAX_TRACKS; t++) {
        auto& trk = studio.tracks[t];
        for (int s = 0; s < SEQ_STEPS; s++) {
            auto& r = trk->stepRects[s];
            Color c = trk->sequence[s].active ? trk->themeColor : ((s % 4 == 0) ? Color { 35, 35, 40 } : Color { 25, 25, 30 });
            int drawSelectorY = (studio.selTrack == t && studio.selStep == s) ? r.height - 3 : r.height;

            for (int y = 0; y < r.height; y++) {
                for (int x = 0; x < r.width; x++) {
                    int globalX = r.left + x;
                    size_t idx = ((r.top + y) * stride + globalX) * 4;
                    if (studio.isPlaying && (globalX == gridStartX + playheadGlobalX || globalX == gridStartX + playheadGlobalX - 1)) {
                        pixels[idx] = 255;
                        pixels[idx + 1] = 255;
                        pixels[idx + 2] = 255;
                    } else if (y >= drawSelectorY) {
                        pixels[idx] = 255;
                        pixels[idx + 1] = 255;
                        pixels[idx + 2] = 255;
                    } else {
                        pixels[idx] = c.r;
                        pixels[idx + 1] = c.g;
                        pixels[idx + 2] = c.b;
                    }
                }
            }
        }
    }
}

int main()
{
    snd_pcm_t* pcm_h = audioInit();
    pthread_setname_np(pthread_self(), "zicBox_UI");
    sf::RenderWindow window(sf::VideoMode(1080, 950), "zicBox Studio");
    window.setFramerateLimit(60);

    Styles appStyles = { .screen = { 1080, 950 }, .margin = 2, .colors = { { 15, 15, 18 }, { 255, 255, 255 }, { 120, 120, 130 }, { 0, 180, 255 }, { 10, 10, 12 }, { 28, 28, 32 }, { 35, 35, 40 } } };
    auto drawer = std::make_unique<Draw>(appStyles);
    sf::Texture screenTexture;
    screenTexture.create(BUFFER_SIZE, BUFFER_SIZE);
    sf::Sprite screenSprite(screenTexture);
    std::vector<sf::Uint8> pixelBuffer(BUFFER_SIZE * BUFFER_SIZE * 4, 15);

    bool static_needs_redraw = true;
    std::thread aThread(audioWorker, pcm_h);
    pthread_setname_np(aThread.native_handle(), "zicBox_Audio");

    while (window.isOpen() && keep_running) {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed) window.close();
            if (event.type == sf::Event::Resized) {
                window.setView(sf::View(sf::FloatRect(0, 0, (float)event.size.width, (float)event.size.height)));
                static_needs_redraw = true;
            }
            if (event.type == sf::Event::KeyPressed) {
                if (event.key.code == sf::Keyboard::Space) {
                    studio.isPlaying = !studio.isPlaying;
                    static_needs_redraw = true;
                }
                if (event.key.code >= sf::Keyboard::Num1 && event.key.code <= sf::Keyboard::Num8) {
                    std::lock_guard<std::mutex> lock(studio.audioMutex);
                    studio.tracks[event.key.code - sf::Keyboard::Num1]->engine->noteOn(60, 1.0f);
                }
            }
            if (event.type == sf::Event::MouseButtonPressed) {
                int mx = event.mouseButton.x, my = event.mouseButton.y;
                if (studio.transportRect.contains(mx, my)) {
                    studio.isPlaying = !studio.isPlaying;
                    static_needs_redraw = true;
                }

                for (int t = 0; t < (int)studio.tracks.size(); t++) {
                    auto& trk = studio.tracks[t];
                    if (trk->generate != nullptr && trk->genRect.contains(mx, my)) {
                        trk->generate(trk->sequence);
                        static_needs_redraw = true;
                    }
                    if (trk->muteRect.contains(mx, my)) {
                        trk->isMuted = !trk->isMuted;
                        static_needs_redraw = true;
                    }
                    for (int s = 0; s < SEQ_STEPS; s++) {
                        if (trk->stepRects[s].contains(mx, my)) {
                            studio.selTrack = t;
                            studio.selStep = s;
                            if (!sf::Keyboard::isKeyPressed(sf::Keyboard::LShift)) trk->sequence[s].active = !trk->sequence[s].active;
                            static_needs_redraw = true;
                        }
                    }
                }
            }
            if (event.type == sf::Event::MouseWheelScrolled) {
                int mx = event.mouseWheelScroll.x, my = event.mouseWheelScroll.y;
                float delta = event.mouseWheelScroll.delta;
                uint32_t now = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now().time_since_epoch()).count();
                if (studio.bpmRect.contains(mx, my)) {
                    int scaled = encGetScaledDirection(delta, now, lastBpmTick);
                    lastBpmTick = now;
                    studio.bpm = CLAMP(studio.bpm + (scaled * (sf::Keyboard::isKeyPressed(sf::Keyboard::LShift) ? 5.0f : 0.5f)), 20.0f, 300.0f);
                    studio.updateClock();
                    static_needs_redraw = true;
                } else if (studio.selTrack != -1 && studio.selStep != -1 && (studio.editNoteRect.contains(mx, my) || studio.editVeloRect.contains(mx, my) || studio.editProbRect.contains(mx, my))) {
                    auto& step = studio.tracks[studio.selTrack]->sequence[studio.selStep];
                    int scaled = (delta > 0) ? 1 : -1;
                    if (studio.editNoteRect.contains(mx, my)) step.note = CLAMP(step.note + scaled, 0, (int)MIDI_LAST_NOTE);
                    else if (studio.editVeloRect.contains(mx, my)) step.velocity = CLAMP(step.velocity + (scaled * 0.05f), 0.0f, 1.0f);
                    else if (studio.editProbRect.contains(mx, my)) step.condition = CLAMP(step.condition + (scaled * 0.05f), 0.0f, 1.0f);
                    static_needs_redraw = true;
                } else {
                    for (auto& trk : studio.tracks) {
                        if (trk->volRect.contains(mx, my)) {
                            int scaled = encGetScaledDirection(delta, now, trk->lastVolShiftTick);
                            trk->lastVolShiftTick = now;
                            trk->volume = CLAMP(trk->volume + (scaled * (sf::Keyboard::isKeyPressed(sf::Keyboard::LShift) ? 0.05f : 0.01f)), 0.0f, 1.0f);
                            static_needs_redraw = true;
                        } else if (trk->trackBounds.contains(mx, my)) {
                            int margin = 10, pPerRow = 8, rowH = 26;
                            int colW = ((int)window.getSize().x - (margin * 2)) / pPerRow;
                            int pIdx = ((my - (trk->trackBounds.top + 14)) / rowH) * pPerRow + (mx - margin) / colW;
                            if (pIdx >= 0 && (size_t)pIdx < trk->engine->getParamCount()) {
                                std::lock_guard<std::mutex> lock(studio.audioMutex);
                                Param& p = trk->engine->getParams()[pIdx];
                                int scaled = encGetScaledDirection(delta, now, trk->lastShiftTicks[pIdx]);
                                trk->lastShiftTicks[pIdx] = now;
                                p.set(p.value + (scaled * p.step * (sf::Keyboard::isKeyPressed(sf::Keyboard::LShift) ? 5.0f : 1.0f)));
                                trk->activeParamIdx = pIdx;
                                trk->lastEditTime = std::chrono::steady_clock::now();
                                static_needs_redraw = true;
                            }
                        }
                    }
                }
            }
        }

        if (static_needs_redraw) {
            sf::Vector2u winSize = window.getSize();
            drawer->setScreenSize({ (int)winSize.x, (int)winSize.y });
            drawStaticUI(*drawer, winSize);
            for (unsigned int y = 0; y < winSize.y; y++)
                std::memcpy(&pixelBuffer[y * BUFFER_SIZE * 4], drawer->screenBuffer[y], winSize.x * 4);
            static_needs_redraw = false;
        }

        updateWaveforms(pixelBuffer, BUFFER_SIZE);
        updateSequencerPixels(pixelBuffer, BUFFER_SIZE);
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