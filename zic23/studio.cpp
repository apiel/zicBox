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
#include "zic23/audioWorker.h"
#include "zic23/generator.h"
#include "zic23/studio.h"
#include "zic23/uiTop.h"
#include "zic23/uiEq.h"
#include "zic23/uiMasterFx.h"
#include "zic23/uiSeq.h"

// Helper to trigger a non-blocking note preview (noteOn -> wait -> noteOff)
void triggerPreview(Track& trk, int note, float velocity, int durationMs = 200)
{
    {
        std::lock_guard<std::mutex> lock(studio.audioMutex);
        trk.engine->noteOn(note, velocity);
    }
    std::thread([&trk, note, durationMs]() {
        std::this_thread::sleep_for(std::chrono::milliseconds(durationMs));
        std::lock_guard<std::mutex> lock(studio.audioMutex);
        trk.engine->noteOff(note);
    }).detach();
}

void drawHelpOverlay(Draw& d, sf::Vector2u size)
{
    int W = (int)size.x, H = (int)size.y;
    int x = W < 1000 ? 50 : 200, y = 50, rW = W - x * 2, rH = H - y * 2;
    d.filledRect({ x, y }, { rW, rH }, { .color = { 20, 20, 25 } });
    d.rect({ x, y }, { rW, rH }, { .color = { 200, 200, 205 } });
    helpCloseRect = { x + rW - 50, y + 10, 40, 12 };
    d.filledRect({ helpCloseRect.left, helpCloseRect.top }, { 40, 12 }, { .color = { 200, 50, 50 } });
    d.text({ helpCloseRect.left + 6, helpCloseRect.top + 2 }, "Close", 8, { .color = { 255, 255, 255 }, .font = &PoppinsLight_8 });
    d.text({ x + 10, y + 10 }, "KEYBOARD SHORTCUTS", 16, { .color = { 0, 180, 255 }, .font = &PoppinsLight_16 });
    y += 40;
    auto dk = [&](std::string k, std::string v) {
        d.text({ x + 10, y }, k, 12, { .color = { 255, 255, 255 }, .font = &PoppinsLight_12 });
        d.text({ x + 160, y }, v, 12, { .color = { 180, 180, 190 }, .font = &PoppinsLight_12 });
        y += 24;
    };
    dk("SPACE", "Play / Stop");
    dk("H", "Help");
    y += 8;
    dk("1-6", "Trigger note");
    dk("M+1-6", "Mute / unmute");
    dk("G+1-6", "Generate");
    dk("SHIFT+1-6", "Select track");
    dk("F1-F12", "Scatter effect");
    y += 8;
    dk("D", "Duplicate sequence");
    dk("DELETE", "De-duplicate / delete page");
    dk("- / +", "Stretch / compress");
    y += 8;
    dk("CTRL+C/V", "Copy / paste step or track");
    dk("SCROLL", "Edit param / step");
    dk("N/V/P/L+SCROLL", "Edit Note / vel / prob / len");
    dk("MIDDLE CLICK", "Cycle step edit mode");
}

void drawStaticUI(Draw& d, sf::Vector2u size)
{
    d.clear();
    const int winW = (int)size.x;
    drawTopBarUI(d, size);

    int currentY = 35;
    const int paramsPerRow = 8;
    const int colW = (winW - MARGIN * 2) / paramsPerRow;
    auto now = std::chrono::steady_clock::now();

    int trackIdx = 0;
    for (auto& trkPtr : studio.tracks) {
        Track& trk = *trkPtr;
        int startY = currentY;

        d.filledRect({ MARGIN, currentY + 2 }, { colW / 2, TRACK_H - 4 }, { .color = d.styles.colors.quaternary });
        d.text({ MARGIN + 4, currentY + 4 }, trk.engine->getName(), 8, { .color = trk.themeColor, .font = &PoppinsLight_8 });

        int vuX = MARGIN + colW / 2 + 4;
        trk.vuRect = sf::IntRect(vuX, currentY + 2, WAVE_HISTORY, TRACK_H - 4);

        int specX = vuX + WAVE_HISTORY + 4;
        specRects[trackIdx] = sf::IntRect(specX, currentY + 2, SPEC_W, TRACK_H - 4);
        trackIdx++;

        currentY += TRACK_H;

        // Param rows
        Param* params = trk.engine->getParams();
        Color bgColor = darken(d.styles.colors.quaternary, 0.1);

        for (size_t p = 0; p < trk.engine->getParamCount(); p++) {
            int x = MARGIN + ((int)p % paramsPerRow) * colW;
            int y = currentY + ((int)p / paramsPerRow) * ROW_H;

            d.filledRect({ x, y }, { colW - 2, ROW_H - 2 }, { .color = bgColor });

            d.text({ x + 4, y + 2 }, params[p].label, 12, { .color = d.styles.colors.text, .font = &PoppinsLight_12 });
            if (winW >= 900 || (trk.activeParamIdx == (int)p && std::chrono::duration_cast<std::chrono::milliseconds>(now - trk.lastEditTime).count() < 1500)) {
                std::stringstream ss;
                ss << std::fixed << std::setprecision(params[p].precision) << params[p].value << params[p].unit;
                d.textRight({ x + colW - 6, y + 2 }, params[p].string ? params[p].string : ss.str(), 8,
                    { .color = { 120, 120, 130 }, .font = &PoppinsLight_8 });
            }
            float range = params[p].max - params[p].min;
            float pct = (params[p].value - params[p].min) / (range <= 0 ? 1.f : range);
            int bX = x + 4, bY = y + ROW_H - 8, bW = colW - 10;
            if (params[p].type & VALUE_CENTERED) {
                int mid = bX + bW / 2;
                int fw = (int)((bW / 2) * (params[p].value / params[p].max));
                if (fw < 0) d.filledRect({ mid + fw, bY }, { std::abs(fw), 3 }, { .color = trk.themeColor });
                else d.filledRect({ mid, bY }, { fw, 3 }, { .color = trk.themeColor });
                d.filledRect({ mid, bY - 1 }, { 1, 5 }, { .color = { 100, 100, 100 } });
            } else {
                d.filledRect({ bX, bY }, { (int)(bW * pct), 3 }, { .color = trk.themeColor });
            }
        }
        int secH = (((int)trk.engine->getParamCount() + 7) / 8) * ROW_H + TRACK_H;
        trk.trackBounds = sf::IntRect(MARGIN, startY, winW - MARGIN * 2, secH);
        currentY += secH - TRACK_H + 2;
    }

    currentY += 10;
    currentY = drawMixerUI(d, size, currentY);
    currentY += 4;
    currentY = drawSeqUI(d, size, currentY);
    currentY += 4;
    drawEqUI(d, size, currentY);
    drawMasterFxUI(d, size, currentY);

    if (showHelp) drawHelpOverlay(d, size);
}

void updateWaveforms(std::vector<sf::Uint8>& pixels, int stride)
{
    for (auto& p : studio.tracks) {
        std::lock_guard<std::mutex> lk(p->historyMtx);
        for (int x = 0; x < WAVE_HISTORY; x++) {
            int bH = (int)(std::min(p->history[x], 1.f) * (p->vuRect.height / 2));
            for (int y = 0; y < p->vuRect.height; y++) {
                size_t idx = ((p->vuRect.top + y) * stride + p->vuRect.left + x) * 4;
                bool w = std::abs(y - p->vuRect.height / 2) <= bH;
                pixels[idx] = w ? p->themeColor.r : 20;
                pixels[idx + 1] = w ? p->themeColor.g : 20;
                pixels[idx + 2] = w ? p->themeColor.b : 25;
            }
        }
    }
}

int main()
{
    snd_pcm_t* pcm_h = audioInit();
    pthread_setname_np(pthread_self(), "zicBox_UI");
    sf::RenderWindow window(sf::VideoMode(1080, 1080), "Zic23");
    window.setFramerateLimit(60);

    Styles appStyles = {
        .screen = { 1080, 1080 }, .margin = 2, .colors = { { 15, 15, 18 }, { 255, 255, 255 }, { 120, 120, 130 }, { 0, 180, 255 }, { 10, 10, 12 }, { 28, 28, 32 }, { 35, 35, 40 } }
    };
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

            if (event.type == sf::Event::MouseButtonReleased) {
                eqDragging = false;
                eqDragBand = -1;
                filterDragging = false;
            }

            if (event.type == sf::Event::MouseMoved) {
                float mx = (float)event.mouseMove.x, my = (float)event.mouseMove.y;
                if (eqDragging) {
                    studio.tracks[eqActiveTrack]->eq.applyDrag(eqDragBand, mx, my, (float)eqCanvasRect.left, eqCanvasY_f, (float)eqCanvasRect.width, eqCanvasH_f, EQ_DB_RANGE, SAMPLE_RATE);
                    static_needs_redraw = true;
                }
                if (filterDragging) {
                    studio.filter.setCutoff(std::clamp(((mx - filterPadRect.left) / filterPadRect.width) * 2.0f - 1.0f, -1.0f, 1.0f));
                    studio.filter.setResonance(std::clamp(1.0f - ((my - filterPadRect.top) / filterPadRect.height), 0.0f, 1.0f));
                    static_needs_redraw = true;
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
                    static_needs_redraw = true;
                }
                if (event.key.code == sf::Keyboard::Space) {
                    studio.isPlaying = !studio.isPlaying;
                    static_needs_redraw = true;
                }
                if (studio.selTrack != -1) {
                    if (event.key.code == sf::Keyboard::D) {
                        duplicateTrackSequence(*studio.tracks[studio.selTrack]);
                        static_needs_redraw = true;
                    }
                    if (event.key.code == sf::Keyboard::Delete) {
                        deleteTrackSequence(*studio.tracks[studio.selTrack]);
                        static_needs_redraw = true;
                    }
                    if (event.key.code == sf::Keyboard::Dash || event.key.code == sf::Keyboard::Subtract) {
                        stretchTrackSequence(*studio.tracks[studio.selTrack], true);
                        static_needs_redraw = true;
                    }
                    if (event.key.code == sf::Keyboard::Equal || event.key.code == sf::Keyboard::Add) {
                        compressTrackSequence(*studio.tracks[studio.selTrack], true);
                        static_needs_redraw = true;
                    }
                }
                if (sf::Keyboard::isKeyPressed(sf::Keyboard::LControl) || sf::Keyboard::isKeyPressed(sf::Keyboard::RControl)) {
                    if (event.key.code == sf::Keyboard::C && studio.selTrack != -1 && studio.selStep != -1) {
                        copyTrackIdx = studio.selTrack;
                        copyStepIdx = studio.selStep;
                        copiedStep = studio.tracks[copyTrackIdx]->sequence[copyStepIdx];
                    }
                    if (event.key.code == sf::Keyboard::V && studio.selTrack != -1 && studio.selStep != -1 && copyTrackIdx != -1) {
                        if (copyTrackIdx == studio.selTrack) {
                            studio.tracks[studio.selTrack]->sequence[studio.selStep] = copiedStep;
                        } else {
                            for (int i = 0; i < SEQ_STEPS; i++)
                                studio.tracks[studio.selTrack]->sequence[i] = studio.tracks[copyTrackIdx]->sequence[i];
                            studio.tracks[studio.selTrack]->volume = studio.tracks[copyTrackIdx]->volume;
                            studio.tracks[studio.selTrack]->genLen = studio.tracks[copyTrackIdx]->genLen;
                        }
                        static_needs_redraw = true;
                    }
                }
                if (event.key.code >= sf::Keyboard::Num1 && event.key.code <= sf::Keyboard::Num6) {
                    int trkIdx = event.key.code - sf::Keyboard::Num1;
                    if (sf::Keyboard::isKeyPressed(sf::Keyboard::G)) {
                        runGeneration(trkIdx);
                        static_needs_redraw = true;
                    } else if (sf::Keyboard::isKeyPressed(sf::Keyboard::M)) {
                        studio.tracks[trkIdx]->isMuted = !studio.tracks[trkIdx]->isMuted;
                        static_needs_redraw = true;
                    } else if (sf::Keyboard::isKeyPressed(sf::Keyboard::LShift)) {
                        studio.selTrack = trkIdx;
                        studio.selStep = 0;
                        static_needs_redraw = true;
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
                    if (helpCloseRect.contains(mx, my)) {
                        showHelp = false;
                        static_needs_redraw = true;
                    }
                    continue;
                }
                if (filterPadRect.contains(mx, my)) {
                    filterDragging = true;
                    studio.filter.setCutoff(std::clamp(((float)(mx - filterPadRect.left) / filterPadRect.width) * 2.0f - 1.0f, -1.0f, 1.0f));
                    studio.filter.setResonance(std::clamp(1.0f - ((float)(my - filterPadRect.top) / filterPadRect.height), 0.0f, 1.0f));
                    static_needs_redraw = true;
                }
                for (int t = 0; t < MAX_TRACKS; t++)
                    if (eqTrackBtnRects[t].contains(mx, my)) {
                        eqActiveTrack = t;
                        static_needs_redraw = true;
                    }
                if (eqCanvasRect.contains(mx, my)) {
                    auto& eq = studio.tracks[eqActiveTrack]->eq;
                    for (int b = 0; b < EQ::NUM_BANDS; b++) {
                        auto [px, py] = eq.dotPos(b, (float)eqCanvasRect.left, eqCanvasY_f, (float)eqCanvasRect.width, eqCanvasH_f, EQ_DB_RANGE);
                        if ((mx - (int)px) * (mx - (int)px) + (my - (int)py) * (my - (int)py) <= (EQ_DOT_R + 4) * (EQ_DOT_R + 4)) {
                            eqDragging = true;
                            eqDragBand = b;
                            break;
                        }
                    }
                }
                if (event.mouseButton.button == sf::Mouse::Middle) {
                    studio.stepEditMode = static_cast<StepEditMode>((studio.stepEditMode + 1) % MODE_COUNT);
                    static_needs_redraw = true;
                }
                if (studio.transportRect.contains(mx, my)) {
                    studio.isPlaying = !studio.isPlaying;
                    static_needs_redraw = true;
                }
                if (helpBtnRect.contains(mx, my)) {
                    showHelp = true;
                    static_needs_redraw = true;
                }
                for (int t = 0; t < (int)studio.tracks.size(); t++) {
                    auto& trk = studio.tracks[t];
                    if (trk->genRect.contains(mx, my)) {
                        runGeneration(t);
                        static_needs_redraw = true;
                    }
                    if (trk->muteRect.contains(mx, my)) {
                        trk->isMuted = !trk->isMuted;
                        static_needs_redraw = true;
                    }
                    for (int s = 0; s < SEQ_STEPS; s++)
                        if (trk->stepRects[s].contains(mx, my)) {
                            studio.selTrack = t;
                            studio.selStep = s;
                            if (event.mouseButton.button == sf::Mouse::Left && !sf::Keyboard::isKeyPressed(sf::Keyboard::LShift)) {
                                trk->sequence[s].active = !trk->sequence[s].active;
                                if (trk->sequence[s].active && !studio.isPlaying) triggerPreview(*trk, trk->sequence[s].note, trk->sequence[s].velocity);
                            }
                            static_needs_redraw = true;
                        }
                }
            }
            if (event.type == sf::Event::MouseWheelScrolled) {
                if (showHelp) continue;
                int mx = event.mouseWheelScroll.x, my = event.mouseWheelScroll.y;
                float delta = event.mouseWheelScroll.delta;
                uint32_t now = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now().time_since_epoch()).count();

                // Compressor interactions
                for (int i = 0; i < 4; i++) {
                    if (compRects[i].contains(mx, my)) {
                        float step = (sf::Keyboard::isKeyPressed(sf::Keyboard::LShift)) ? 5.0f : 1.0f;
                        if (i == 0) studio.compressor.threshold = std::clamp(studio.compressor.threshold + (delta > 0 ? step : -step), -60.0f, 0.0f);
                        else if (i == 1) studio.compressor.ratio = std::clamp(studio.compressor.ratio + (delta > 0 ? 0.5f : -0.5f), 1.0f, 20.0f);
                        else if (i == 2) studio.compressor.attack = std::clamp(studio.compressor.attack + (delta > 0 ? 0.001f : -0.001f), 0.0001f, 0.1f);
                        else if (i == 3) studio.compressor.release = std::clamp(studio.compressor.release + (delta > 0 ? 0.01f : -0.01f), 0.01f, 1.0f);
                        static_needs_redraw = true;
                    }
                }

                bool handled = false;
                for (int t = 0; t < MAX_TRACKS && !handled; t++) {
                    auto& trk = studio.tracks[t];
                    if (trk->lenBtnRect.contains(mx, my)) {
                        if (delta > 0) compressTrackSequence(*trk, true);
                        else stretchTrackSequence(*trk, true);
                        static_needs_redraw = true;
                        handled = true;
                        break;
                    }
                    for (int s = 0; s < SEQ_STEPS; s++)
                        if (trk->stepRects[s].contains(mx, my)) {
                            int sc = delta > 0 ? 1 : -1;
                            auto& step = trk->sequence[s];
                            bool isNote = sf::Keyboard::isKeyPressed(sf::Keyboard::N) || (studio.stepEditMode == EDIT_NOTE && !sf::Keyboard::isKeyPressed(sf::Keyboard::L) && !sf::Keyboard::isKeyPressed(sf::Keyboard::P) && !sf::Keyboard::isKeyPressed(sf::Keyboard::V));
                            if (sf::Keyboard::isKeyPressed(sf::Keyboard::L)) editStep(step, EDIT_LEN, sc);
                            else if (sf::Keyboard::isKeyPressed(sf::Keyboard::P)) editStep(step, EDIT_PROB, sc);
                            else if (sf::Keyboard::isKeyPressed(sf::Keyboard::V)) editStep(step, EDIT_VELO, sc);
                            else if (sf::Keyboard::isKeyPressed(sf::Keyboard::N)) editStep(step, EDIT_NOTE, sc);
                            else editStep(step, studio.stepEditMode, sc);
                            if (isNote && !studio.isPlaying) triggerPreview(*trk, step.note, step.velocity);
                            studio.selTrack = t;
                            studio.selStep = s;
                            static_needs_redraw = true;
                            handled = true;
                            break;
                        }
                }
                if (!handled) {
                    if (studio.bpmRect.contains(mx, my)) {
                        int scaled = encGetScaledDirection(delta, now, lastBpmTick);
                        lastBpmTick = now;
                        studio.bpm = std::clamp(studio.bpm + (scaled * (sf::Keyboard::isKeyPressed(sf::Keyboard::LShift) ? 5.0f : 0.5f)), 20.0f, 300.0f);
                        studio.updateClock();
                        static_needs_redraw = true;
                    } else if (studio.selTrack != -1 && studio.selStep != -1 && (studio.editNoteRect.contains(mx, my) || studio.editVeloRect.contains(mx, my) || studio.editProbRect.contains(mx, my) || studio.editLenRect.contains(mx, my))) {
                        auto& trk = studio.tracks[studio.selTrack];
                        auto& step = trk->sequence[studio.selStep];
                        int scaled = delta > 0 ? 1 : -1;
                        if (studio.editNoteRect.contains(mx, my)) {
                            editStep(step, EDIT_NOTE, scaled);
                            if (!studio.isPlaying) triggerPreview(*trk, step.note, step.velocity);
                        } else if (studio.editVeloRect.contains(mx, my)) editStep(step, EDIT_VELO, scaled);
                        else if (studio.editProbRect.contains(mx, my)) editStep(step, EDIT_PROB, scaled);
                        else if (studio.editLenRect.contains(mx, my)) editStep(step, EDIT_LEN, scaled);
                        static_needs_redraw = true;
                    } else {
                        for (auto& trk : studio.tracks) {
                            if (trk->volRect.contains(mx, my)) {
                                int scaled = encGetScaledDirection(delta, now, trk->lastVolShiftTick);
                                trk->lastVolShiftTick = now;
                                trk->volume = std::clamp(trk->volume + scaled * (sf::Keyboard::isKeyPressed(sf::Keyboard::LShift) ? 0.05f : 0.01f), 0.f, 1.f);
                                static_needs_redraw = true;
                            } else if (trk->trackBounds.contains(mx, my)) {
                                const int winW = (int)window.getSize().x;
                                const int cW = (winW - MARGIN * 2) / 8;
                                int pIdx = ((my - (trk->trackBounds.top + TRACK_H)) / ROW_H) * 8 + (mx - MARGIN) / cW;
                                if (pIdx >= 0 && (size_t)pIdx < trk->engine->getParamCount()) {
                                    std::lock_guard<std::mutex> lock(studio.audioMutex);
                                    Param& p = trk->engine->getParams()[pIdx];
                                    int scaled = encGetScaledDirection(delta, now, trk->lastShiftTicks[pIdx]);
                                    trk->lastShiftTicks[pIdx] = now;
                                    p.set(p.value + scaled * p.step * (sf::Keyboard::isKeyPressed(sf::Keyboard::LShift) ? 5.f : 1.f));
                                    trk->activeParamIdx = pIdx;
                                    trk->lastEditTime = std::chrono::steady_clock::now();
                                    static_needs_redraw = true;
                                }
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
            for (unsigned y = 0; y < winSize.y; y++)
                std::memcpy(&pixelBuffer[y * BUFFER_SIZE * 4], drawer->screenBuffer[y], winSize.x * 4);
            static_needs_redraw = false;
        }

        if (!showHelp) {
            updateWaveforms(pixelBuffer, BUFFER_SIZE);
            updateSequencerPixels(pixelBuffer, BUFFER_SIZE);
            updateSpectrumPixels(pixelBuffer, BUFFER_SIZE);
            updateCompressorMeter(pixelBuffer, BUFFER_SIZE);
        }

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