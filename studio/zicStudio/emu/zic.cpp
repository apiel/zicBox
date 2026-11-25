/**
 * zicStudio: an 8 track rack hosting the Kick32v5, Acid32 and Fm4 engines.
 *
 * - one shared transport (play / stop / BPM) drives every track's sequencer
 * - every track has its own engine, panel, sequencer, mixer strip and colour
 * - one master FX bus (delay + reverb)
 * - the selected track is edited with its real instrument panel (reused from
 *   the standalone studios) and its sequencer, on the right or below depending
 *   on the window shape
 * - one project file remembers everything
 */

#include <alsa/asoundlib.h>
#include <atomic>
#include <cctype>
#include <chrono>
#include <cmath>
#include <cstdio>
#include <cstring>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <memory>
#include <mutex>
#include <pthread.h>
#include <sched.h>
#include <sys/resource.h>
#include <sstream>
#include <thread>
#include <type_traits>
#include <vector>

#include <SFML/Graphics.hpp>

#include "Persist.h"

#include "studio/zicStudioKick/emu/Sequencer.h"
#include "studio/zicStudioKick/core/Kick32v5Panel.h"
#include "studio/zicStudioAcid/emu/AcidSequencer.h"
#include "studio/zicStudioAcid/core/Acid32Panel.h"
#include "studio/zicStudioFm4/emu/Fm4Sequencer.h"
#include "studio/zicStudioFm4/core/Fm4Panel.h"

#include "audio/engines/Kick32v5.h"
#include "audio/engines/Acid32.h"
#include "audio/engines/Fm4.h"
#include "audio/effects/StudioFx.h"

#include "ui/KickUi.h"
#include "ui/AcidUi.h"
#include "ui/Fm4Ui.h"

static constexpr unsigned int SAMPLE_RATE = 44100;
static constexpr size_t NUM_FRAMES = 512;
static constexpr int TRACK_COUNT = 8;

// globals shared with the reused instrument UIs
std::mutex audioMutex;
std::atomic<bool> keep_running(true);
std::atomic<bool> transportPlaying(false);
std::atomic<float> transportBpm(140.0f);
std::atomic<int> selectedTrack(0);

std::atomic<int> kickTrigger { 0 };
std::atomic<int> auditionNote { 0 };
std::atomic<bool> kickMuted { false };
std::atomic<bool> acidMuted { false };

// master section (UI <-> audio, all normalised 0..1)
std::atomic<float> masterVolume(0.8f);
std::atomic<float> cpuUsage(0.0f);
std::atomic<float> uiFps(0.0f);
static bool seqOnRight = true;      // wide layout: sequencer on the right or bottom
static float seqSplit = 0.0f;       // 0 = natural panel, 1 = panel shrunk to minimum
std::atomic<float> masterMeter(0.0f);
std::atomic<float> fxDelayTime(0.28f);
std::atomic<float> fxDelayFeedback(0.35f);
std::atomic<float> fxDelayLevel(0.45f);
std::atomic<float> fxReverbSize(0.55f);
std::atomic<float> fxReverbLevel(0.35f);

enum EngineType { ENGINE_KICK = 0, ENGINE_ACID = 1, ENGINE_FM4 = 2, ENGINE_COUNT = 3 };

static const char* engineName(int t) { return t == 0 ? "Kick32v5" : (t == 1 ? "Acid32" : "Fm4"); }
static const char* engineShort(int t) { return t == 0 ? "KIK" : (t == 1 ? "ACD" : "FM4"); }

struct Track {
    int index = 0;
    int type = ENGINE_KICK;
    sf::Color color;

    std::unique_ptr<Kick32v5> kick;
    std::unique_ptr<Kick32v5Panel::Panel> kickPanel;
    std::unique_ptr<Sequencer> kickSeq;
    kick_ui::Capture kickCap;
    std::unique_ptr<kick_ui::EmuUi> kickUi;
    int kickScopePos = -1;

    std::unique_ptr<Acid32> acid;
    std::unique_ptr<Acid32Panel::Panel> acidPanel;
    std::unique_ptr<AcidSequencer> acidSeq;
    acid_ui::AcidCapture acidCap;
    std::unique_ptr<acid_ui::EmuUi> acidUi;

    std::unique_ptr<Fm4> fm4;
    std::unique_ptr<Fm4Panel::Panel> fm4Panel;
    std::unique_ptr<Fm4Sequencer> fm4Seq;
    fm4_ui::Fm4Capture fm4Cap;
    std::unique_ptr<fm4_ui::EmuUi> fm4Ui;

    float volume = 0.75f;
    float pan = 0.0f;
    float delaySend = 0.18f;
    float reverbSend = 0.12f;
    bool mute = false;
    bool solo = false;
    std::atomic<float> meter { 0.0f };

    Track(int i, sf::Color c)
        : index(i)
        , color(c)
    {
        kick = std::make_unique<Kick32v5>(SAMPLE_RATE);
        kickPanel = std::make_unique<Kick32v5Panel::Panel>(*kick);
        kickSeq = std::make_unique<Sequencer>(SAMPLE_RATE);

        acid = std::make_unique<Acid32>(SAMPLE_RATE);
        acidPanel = std::make_unique<Acid32Panel::Panel>(*acid);
        acidSeq = std::make_unique<AcidSequencer>(SAMPLE_RATE);

        fm4 = std::make_unique<Fm4>(SAMPLE_RATE);
        fm4Panel = std::make_unique<Fm4Panel::Panel>(*fm4);
        fm4Seq = std::make_unique<Fm4Sequencer>(SAMPLE_RATE);
    }

    void createUis()
    {
        kickUi = std::make_unique<kick_ui::EmuUi>(*kickPanel, *kickSeq, kickCap);
        kickUi->showTopBar = false;
        kickUi->showSequencer = false;

        acidUi = std::make_unique<acid_ui::EmuUi>(*acidPanel, *acidSeq, acidCap);
        acidUi->showTopBar = false;
        acidUi->showSequencer = false;

        fm4Ui = std::make_unique<fm4_ui::EmuUi>(*fm4Panel, *fm4Seq, fm4Cap);
        fm4Ui->showTopBar = false;
        fm4Ui->showSequencer = false;
    }

    void setType(int t)
    {
        if (t < 0 || t >= ENGINE_COUNT || t == type) return;
        if (type == ENGINE_ACID) acid->noteOff();
        if (type == ENGINE_FM4) fm4->noteOff();
        type = t;
        kickTrigger = 0;
        auditionNote = 0;
        // All three sequencers of every track run with the shared transport,
        // so switching engine mid-play stays on the same step phase.
    }

    float peak = 0.0f;
};

std::vector<std::unique_ptr<Track>> tracks;

// ---------------------------------------------------------------- audio

snd_pcm_t* audioInit()
{
    snd_pcm_t* pcm;
    int err = snd_pcm_open(&pcm, "default", SND_PCM_STREAM_PLAYBACK, 0);
    if (err < 0) {
        std::cerr << "Audio open error: " << snd_strerror(err) << "\n";
        return nullptr;
    }
    snd_pcm_set_params(pcm, SND_PCM_FORMAT_S16_LE, SND_PCM_ACCESS_RW_INTERLEAVED, 2, SAMPLE_RATE, 1, 40000);
    return pcm;
}

static void triggerKick(Track& t, int note)
{
    t.kick->noteOn((uint8_t)std::clamp(note, 0, 127), 1.0f);
    auto& cap = t.kickCap;
    for (int i = 0; i < kick_ui::Capture::N; ++i) cap.lo[i] = cap.hi[i] = 0.0f;
    cap.note = note;
    cap.hitId++;
    cap.count = 0;
    t.kickScopePos = 0;
}

static void triggerAcid(Track& t, int note, float vel, bool slide, bool accent)
{
    t.acid->noteOn((uint8_t)std::clamp(note, 0, 127), vel, slide, accent);
    t.acidCap.note = note;
    t.acidCap.hitId++;
}

static void triggerFm4(Track& t, int note, float vel, bool slide)
{
    t.fm4->noteOn((uint8_t)std::clamp(note, 0, 127), vel, slide);
    t.fm4Cap.note = note;
    t.fm4Cap.hitId++;
}

static void captureKick(Track& t, float out)
{
    auto& cap = t.kickCap;
    if (t.kickScopePos >= 0) {
        int b = t.kickScopePos / (int)(SAMPLE_RATE * kick_ui::Capture::MS_PER_BUCKET / 1000.0f);
        if (b < kick_ui::Capture::N) {
            cap.lo[b] = std::min(cap.lo[b], out);
            cap.hi[b] = std::max(cap.hi[b], out);
            if (t.kickScopePos < kick_ui::Capture::SAMPLES) cap.samples[t.kickScopePos] = out;
            t.kickScopePos++;
            cap.count = std::min(t.kickScopePos, kick_ui::Capture::SAMPLES);
        } else {
            t.kickScopePos = -1;
        }
    }
    cap.ring[cap.ringPos] = out;
    cap.ringPos = (cap.ringPos + 1) % kick_ui::Capture::RING;
    if (++cap.trailDiv >= 3) {
        cap.trailDiv = 0;
        cap.trailPre[cap.trailPos] = t.kick->livePre;
        cap.trailPost[cap.trailPos] = t.kick->livePost;
        cap.trailPos = (cap.trailPos + 1) % kick_ui::Capture::TRAIL;
    }
}

static void captureAcid(Track& t, float out)
{
    auto& cap = t.acidCap;
    cap.ring[cap.ringPos] = out;
    cap.ringPos = (cap.ringPos + 1) % acid_ui::AcidCapture::RING;
    cap.livePre = t.acid->livePre;
    cap.livePost = t.acid->livePost;
    cap.liveCutoff = t.acid->liveCutoffHz;
    cap.liveReso = t.acid->liveReso;
    cap.liveEnvAmp = t.acid->liveEnvAmp;
    cap.liveEnvFilter = t.acid->liveEnvFilter;
    cap.liveLfoVal = t.acid->liveLfoVal;
}

static void captureFm4(Track& t, float out)
{
    auto& cap = t.fm4Cap;
    cap.ring[cap.ringPos] = out;
    cap.ringPos = (cap.ringPos + 1) % fm4_ui::Fm4Capture::RING;
    cap.peak = std::max(cap.peak, std::fabs(out));
}

void audioWorker(snd_pcm_t* pcm)
{
    std::vector<int16_t> buffer(NUM_FRAMES * 2);
    StudioFx delayFx, reverbFx;
    delayFx.reverbAmount = 0.0f;
    delayFx.delayLevel = 1.0f;
    reverbFx.delayLevel = 0.0f;
    bool wasPlaying = false;
    float meter = 0.0f;

    while (keep_running) {
        {
            std::lock_guard<std::mutex> lock(audioMutex);
            bool play = transportPlaying.load();
            float bpm = transportBpm.load();

            // master FX parameters (set on the UI thread, consumed per block)
            float delayMs = 40.0f + fxDelayTime.load() * 760.0f;
            delayFx.delaySamples = (int)std::clamp(delayMs * 0.001f * SAMPLE_RATE, 1.0f, (float)StudioFx::MAX_DELAY - 1);
            delayFx.delayFeedback = std::clamp(fxDelayFeedback.load(), 0.0f, 0.9f);
            delayFx.delayLevel = std::clamp(fxDelayLevel.load(), 0.0f, 1.0f);
            reverbFx.reverbAmount = std::clamp(fxReverbSize.load(), 0.0f, 0.95f);
            reverbFx.reverbLevel = std::clamp(fxReverbLevel.load(), 0.0f, 1.0f);
            float masterGain = masterVolume.load();

            if (play != wasPlaying) {
                for (auto& tp : tracks) {
                    Track& t = *tp;
                    if (play) {
                        t.kickSeq->play();
                        t.acidSeq->play();
                        t.fm4Seq->play();
                    } else {
                        t.kickSeq->stop();
                        t.acidSeq->stop();
                        t.fm4Seq->stop();
                        t.acid->noteOff();
                        t.fm4->noteOff();
                    }
                }
                wasPlaying = play;
            }

            int sel = selectedTrack.load();
            int trig = auditionNote.exchange(0);
            int kTrig = kickTrigger.exchange(0);

            bool anySolo = false;
            for (auto& tp : tracks) anySolo = anySolo || tp->solo;

            // audition / trig buttons of the selected track fire once per block
            if (kTrig) {
                Track& t = *tracks[sel];
                if (t.type == ENGINE_KICK) triggerKick(t, 60);
            }
            if (trig > 0) {
                Track& t = *tracks[sel];
                if (t.type == ENGINE_ACID) triggerAcid(t, trig, 1.0f, false, true);
                else if (t.type == ENGINE_FM4) triggerFm4(t, trig, 1.0f, false);
            }

            for (size_t f = 0; f < NUM_FRAMES; ++f) {
                float dryL = 0.0f, dryR = 0.0f;
                float delayL = 0.0f, delayR = 0.0f;
                float reverbL = 0.0f, reverbR = 0.0f;

                for (auto& tp : tracks) {
                    Track& t = *tp;
                    t.kickSeq->bpm = bpm;
                    t.acidSeq->bpm = bpm;
                    t.fm4Seq->bpm = bpm;

                    // keep all three sequencers running in lockstep; only the
                    // selected engine makes sound, so engine swaps stay on beat
                    int kickNote = 0;
                    bool kickStep = t.kickSeq->tick(kickNote);
                    auto acidEv = t.acidSeq->tick();
                    auto fm4Ev = t.fm4Seq->tick();

                    float out = 0.0f;
                    if (t.type == ENGINE_KICK) {
                        if (kickStep) {
                            bool held = (t.index == sel && kickMuted.load());
                            if (!held) triggerKick(t, 60 + t.kickSeq->transpose + kickNote);
                        }
                        out = t.kick->sample();
                        captureKick(t, out);
                    } else if (t.type == ENGINE_ACID) {
                        if (acidEv.noteOff) t.acid->noteOff();
                        if (acidEv.noteOn) triggerAcid(t, acidEv.note, acidEv.velocity, acidEv.slide, acidEv.accent);
                        out = t.acid->sample();
                        captureAcid(t, out);
                    } else {
                        if (fm4Ev.noteOff) t.fm4->noteOff();
                        if (fm4Ev.noteOn) triggerFm4(t, fm4Ev.note, fm4Ev.velocity, fm4Ev.slide);
                        out = t.fm4->sample();
                        captureFm4(t, out);
                    }

                    float peak = std::fabs(out);
                    t.peak = std::max(peak, t.peak * 0.9995f);
                    t.meter = t.peak;

                    bool audible = !t.mute && (!anySolo || t.solo);
                    if (!audible) continue;

                    float ang = (t.pan + 1.0f) * 0.25f * 3.14159265f;
                    float gl = std::cos(ang), gr = std::sin(ang);
                    float postL = out * t.volume * gl;
                    float postR = out * t.volume * gr;

                    dryL += postL;
                    dryR += postR;
                    delayL += postL * t.delaySend;
                    delayR += postR * t.delaySend;
                    reverbL += postL * t.reverbSend;
                    reverbR += postR * t.reverbSend;
                }

                float wetDelayL = delayFx.process(delayL);
                float wetDelayR = delayFx.process(delayR);
                float wetReverbL = reverbFx.process(reverbL);
                float wetReverbR = reverbFx.process(reverbR);

                float outL = (dryL + wetDelayL + wetReverbL) * masterGain;
                float outR = (dryR + wetDelayR + wetReverbR) * masterGain;
                outL = std::clamp(outL, -1.0f, 1.0f);
                outR = std::clamp(outR, -1.0f, 1.0f);
                meter = std::max(meter, std::max(std::fabs(outL), std::fabs(outR)));

                int16_t vl = (int16_t)(outL * 32767.0f);
                int16_t vr = (int16_t)(outR * 32767.0f);
                buffer[f * 2] = vl;
                buffer[f * 2 + 1] = vr;
            }

            masterMeter = meter;
            meter *= 0.90f;
        }
        if (!pcm) {
            // offline / screenshot mode: keep the DSP clock roughly real-time
            std::this_thread::sleep_for(std::chrono::microseconds(11600));
            continue;
        }
        int w = snd_pcm_writei(pcm, buffer.data(), NUM_FRAMES);
        if (w < 0 && snd_pcm_recover(pcm, w, 0) < 0) std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
}

// ---------------------------------------------------------------- UI

namespace Theme {
inline sf::Color rgb(int r, int g, int b, int a = 255) { return sf::Color(r, g, b, a); }
const sf::Color bg = rgb(10, 12, 16);
const sf::Color card = rgb(18, 21, 27);
const sf::Color cardLine = rgb(34, 38, 48);
const sf::Color cell = rgb(27, 30, 38);
const sf::Color text = rgb(234, 236, 242);
const sf::Color muted = rgb(126, 131, 148);
const sf::Color faint = rgb(68, 72, 88);
const sf::Color accent = rgb(56, 212, 255);
}

struct Layout {
    bool wide = true;
    bool seqRight = true;
    float split = 0.0f;
    float W = 1920.0f, H = 1080.0f;
    sf::FloatRect sidebar, panel, seq;
};

static Layout makeLayout(bool wide, float winW, float winH, bool seqRight, float split)
{
    Layout l;
    l.wide = wide;
    l.seqRight = seqRight && wide;
    l.split = split;
    l.W = winW;
    l.H = winH;
    float sb = wide ? 210.0f : 200.0f;
    l.sidebar = { 0, 0, sb, winH };
    float mx = sb, mw = winW - sb;
    if (l.seqRight) {
        // split 0 = balanced default, 1 = panel shrunk to the minimum
        float base = std::max(700.0f, std::min(1320.0f, mw - 320.0f));
        float pw = base - (base - 700.0f) * std::clamp(split, 0.0f, 1.0f);
        l.panel = { mx, 0, pw, winH };
        l.seq = { mx + pw, 0, mw - pw, winH };
    } else {
        // split 0 is the "natural panel" value used by the right layout; for
        // the stacked layout fall back to a balanced 65/35 split.
        float s = split > 0.05f ? split : 0.35f;
        float ph = std::clamp(winH * (1.0f - s), 380.0f, winH - 200.0f);
        l.panel = { mx, 0, mw, ph };
        l.seq = { mx, ph, mw, winH - ph };
    }
    return l;
}

static bool inRect(const sf::FloatRect& r, sf::Vector2f p) { return r.contains(p); }

class StudioUi {
public:
    StudioUi() { loadFont(); }

    void draw(sf::RenderTarget& t, sf::Vector2f mouse)
    {
        int sel = selectedTrack.load();
        Track& tr = *tracks[sel];
        t.clear(Theme::bg);

        // 1. the real instrument panel (cached in a texture)
        drawInstrument(t, tr, mouse);

        // 2. our shell on top
        t.setView(baseView);
        drawSidebar(t, mouse);
        drawSequencer(t, tr, mouse);
        drawOutputScope(t, tr);
        drawSplitter(t, mouse);
    }

    void mousePressed(sf::Vector2f p, sf::Mouse::Button button, bool shift)
    {
        Layout& l = layout;
        int sel = selectedTrack.load();
        panelDragging = false;
        panelDirty = true;

        if (splitterRect().contains(p)) { dragSplit = true; return; }
        if (inRect(l.sidebar, p)) { sidebarPressed(p, button, shift); return; }
        if (inRect(l.seq, p)) { seqPressed(p, button, shift); return; }

        if (inRect(l.panel, p)) {
            Track& tr = *tracks[sel];
            sf::Vector2f q;
            if (toPanel(tr, p, q)) {
                panelDragging = true;
                panelMousePressed(tr, q, button, shift);
            }
        }
    }

    void mouseReleased()
    {
        for (auto& tp : tracks) {
            tp->kickUi->mouseReleased();
            tp->acidUi->mouseReleased();
            tp->fm4Ui->mouseReleased();
        }
        dragKind = DragNone;
        dragFx = FxNone;
        panelDragging = false;
        dragSidebarVol = -1;
        dragMasterVol = false;
        dragSplit = false;
        seqPainting = false;
    }

    void mouseMoved(sf::Vector2f p, bool shift)
    {
        Layout& l = layout;
        int sel = selectedTrack.load();
        Track& tr = *tracks[sel];

        if (dragSplit) {
            float sbw = layout.sidebar.width;
            if (layout.seqRight) {
                float base = std::max(700.0f, std::min(1320.0f, layout.W - sbw - 320.0f));
                float want = p.x - sbw;
                layout.split = std::clamp((base - want) / std::max(1.0f, base - 700.0f), 0.0f, 1.0f);
            } else {
                layout.split = std::clamp(1.0f - p.y / std::max(1.0f, layout.H), 0.0f, 1.0f);
            }
            seqSplit = layout.split;
            relayout(layout.W, layout.H);
            return;
        }
        if (dragFx != FxNone) { fxDragSidebar(p); return; }
        if (dragSidebarVol >= 0) { sidebarVolDrag(p); return; }
        if (dragMasterVol) { masterVolDrag(p); return; }
        if (seqPainting) { seqPaint(p); return; }

        if (panelDragging) { panelMouseMoved(tr, toPanelRaw(tr, p), shift); return; }

        if (inRect(l.panel, p)) {
            sf::Vector2f q;
            if (toPanel(tr, p, q)) panelMouseMoved(tr, q, shift);
        }
    }

    void wheel(sf::Vector2f p, float delta, bool shift)
    {
        Layout& l = layout;
        int sel = selectedTrack.load();
        Track& tr = *tracks[sel];
        panelDirty = true;

        if (inRect(l.panel, p)) {
            sf::Vector2f q;
            if (toPanel(tr, p, q)) panelWheel(tr, q, delta, shift);
        } else if (inRect(l.seq, p)) {
            seqWheel(p, delta, shift);
        } else if (inRect(l.sidebar, p)) {
            sidebarWheel(p, delta, shift);
        }
    }

    void key(sf::Keyboard::Key k)
    {
        panelDirty = true;
        if (k == sf::Keyboard::Space || k == sf::Keyboard::P || k == sf::Keyboard::Return) {
            transportPlaying = !transportPlaying.load();
            return;
        }
        if (k >= sf::Keyboard::Num1 && k <= sf::Keyboard::Num8) {
            selectedTrack = (int)k - (int)sf::Keyboard::Num1;
            return;
        }
        // let the instrument panel have the keys too (presets etc.)
        int sel = selectedTrack.load();
        panelKey(*tracks[sel], k);
    }

    Layout layout { makeLayout(true, 1920.0f, 1080.0f, true, 0.0f) };
    sf::View baseView;
    sf::FloatRect baseViewport { 0, 0, 1, 1 };

    // The instrument panel is the expensive part of the frame (dozens of
    // knobs + labels + scopes). Cache it in a texture and only repaint when
    // something actually changed, or a few times per second for the scopes.
    sf::RenderTexture panelRT;
    bool panelRTValid = false;
    bool panelDirty = true;
    int panelRTKey = -1;
    unsigned panelRTW = 0, panelRTH = 0;
    int panelRTFrame = 0;
    sf::Vector2f panelRTMouse { -1, -1 };

    void relayout(float w, float h)
    {
        bool wide = w / h >= 1.4f;
        layout = makeLayout(wide, w, h, seqOnRight, seqSplit);
    }

private:
    enum DragKind { DragNone, DragVolume, DragPan, DragDelaySend, DragReverbSend };
    DragKind dragKind = DragNone;
    int dragTrack = -1;
    float dragStart = 0.0f;
    bool panelDragging = false;

    enum FxKnob { FxNone, FxDelayTime, FxDelayFeedback, FxDelayLevel, FxReverbSize, FxReverbLevel };
    FxKnob dragFx = FxNone;
    static constexpr int FX_KNOBS = 5;
    int dragSidebarVol = -1;
    bool dragMasterVol = false;
    bool dragSplit = false;
    int engineMenuTrack = -1;   // track whose engine cog menu is open
    sf::FloatRect seqPosRect;
    int seqSelected = -1;       // sequencer step whose edit strip is open
    bool seqPainting = false;   // dragging across the grid paints notes
    bool seqPaintOn = true;
    sf::FloatRect trackCogRect[TRACK_COUNT];
    sf::FloatRect trackRowRect[TRACK_COUNT];
    sf::FloatRect trackVolRect[TRACK_COUNT];
    sf::FloatRect trackMuteRect[TRACK_COUNT];
    sf::FloatRect trackSoloRect[TRACK_COUNT];
    sf::FloatRect engineDropRect;
    sf::FloatRect engineDropItemRect[ENGINE_COUNT];
    sf::FloatRect masterVolRect;
    sf::FloatRect fxKnobRect[FX_KNOBS];
    sf::FloatRect playBtnRect;
    sf::FloatRect bpmBoxRect, bpmMinusRect, bpmPlusRect;
    sf::FloatRect cpuRect;
    float dragFxStart = 0.0f;
    float dragFxY = 0.0f;

    sf::Font font;

    void loadFont()
    {
        const char* paths[] = {
            "/usr/share/fonts/truetype/roboto/unhinted/RobotoTTF/Roboto-Regular.ttf",
            "../../../draw/fonts/Roboto-Regular.ttf", "../../draw/fonts/Roboto-Regular.ttf",
            "../draw/fonts/Roboto-Regular.ttf", "draw/fonts/Roboto-Regular.ttf",
            "/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf"
        };
        for (auto p : paths)
            if (font.loadFromFile(p)) break;
    }

    void label(sf::RenderTarget& t, const std::string& s, float x, float y, int size, sf::Color c,
        bool center = false, bool bold = false, float spacing = 1.0f)
    {
        sf::Text txt(s, font, size);
        txt.setFillColor(c);
        txt.setLetterSpacing(spacing);
        if (bold) txt.setStyle(sf::Text::Bold);
        auto b = txt.getLocalBounds();
        txt.setOrigin(center ? b.left + b.width * 0.5f : b.left, b.top);
        txt.setPosition(std::round(x), std::round(y));
        t.draw(txt);
    }

    void labelR(sf::RenderTarget& t, const std::string& s, float rightX, float y, int size, sf::Color c,
        bool bold = false, float spacing = 1.0f)
    {
        sf::Text txt(s, font, size);
        txt.setFillColor(c);
        txt.setLetterSpacing(spacing);
        if (bold) txt.setStyle(sf::Text::Bold);
        auto b = txt.getLocalBounds();
        txt.setOrigin(b.left + b.width, b.top);
        txt.setPosition(std::round(rightX), std::round(y));
        t.draw(txt);
    }

    void rrect(sf::RenderTarget& t, sf::FloatRect r, float rad, sf::Color fill, sf::Color line = sf::Color::Transparent)
    {
        sf::ConvexShape s;
        const int seg = 6;
        s.setPointCount(4 * (seg + 1));
        const float cx[4] = { r.left + r.width - rad, r.left + r.width - rad, r.left + rad, r.left + rad };
        const float cy[4] = { r.top + rad, r.top + r.height - rad, r.top + r.height - rad, r.top + rad };
        int k = 0;
        for (int c = 0; c < 4; ++c)
            for (int i = 0; i <= seg; ++i) {
                float a = ((float)c * 90.0f + (float)i * 90.0f / seg) * 3.14159265f / 180.0f;
                s.setPoint(k++, { cx[c] + rad * std::cos(a - 1.5707963f), cy[c] + rad * std::sin(a - 1.5707963f) });
            }
        s.setFillColor(fill);
        if (line.a) {
            s.setOutlineColor(line);
            s.setOutlineThickness(-1.0f);
        }
        t.draw(s);
    }

    // ---- instrument panel mapping

    template <typename UI>
    struct PanelBox {
        float scale, ox, oy, vw, vh, cw, ch;
    };

    template <typename UI>
    PanelBox<UI> panelBox()
    {
        PanelBox<UI> b;
        b.cw = UI::W;
        b.ch = UI::PANEL_BOTTOM - UI::PANEL_TOP;
        // never upscale the panel: 1:1 pixels keeps the knob labels crisp
        b.scale = std::min({ 1.0f, layout.panel.width / b.cw, (layout.panel.height - 12.0f) / b.ch });
        b.vw = b.cw * b.scale;
        b.vh = b.ch * b.scale;
        b.ox = layout.panel.left + (layout.panel.width - b.vw) * 0.5f;
        b.oy = layout.panel.top + 6.0f;
        return b;
    }

    template <typename UI>
    bool mapPanelCoords(sf::Vector2f p, sf::Vector2f& out)
    {
        PanelBox<UI> b = panelBox<UI>();
        sf::FloatRect box { b.ox, b.oy, b.vw, b.vh };
        if (!box.contains(p)) return false;
        out = { (p.x - b.ox) / b.scale, UI::PANEL_TOP + (p.y - b.oy) / b.scale };
        return true;
    }

    bool toPanel(Track& tr, sf::Vector2f p, sf::Vector2f& out)
    {
        if (tr.type == ENGINE_KICK) return mapPanelCoords<kick_ui::EmuUi>(p, out);
        if (tr.type == ENGINE_ACID) return mapPanelCoords<acid_ui::EmuUi>(p, out);
        return mapPanelCoords<fm4_ui::EmuUi>(p, out);
    }

    // Same mapping without the bounds check, so a knob drag keeps tracking
    // when the pointer leaves the panel area.
    template <typename UI>
    sf::Vector2f panelPoint(sf::Vector2f p)
    {
        PanelBox<UI> b = panelBox<UI>();
        return { (p.x - b.ox) / b.scale, UI::PANEL_TOP + (p.y - b.oy) / b.scale };
    }

    sf::Vector2f toPanelRaw(Track& tr, sf::Vector2f p)
    {
        if (tr.type == ENGINE_KICK) return panelPoint<kick_ui::EmuUi>(p);
        if (tr.type == ENGINE_ACID) return panelPoint<acid_ui::EmuUi>(p);
        return panelPoint<fm4_ui::EmuUi>(p);
    }

    template <typename UI>
    void drawPanelView(sf::RenderTarget& t, UI& panelUi, sf::Vector2f mouse, int key)
    {
        PanelBox<UI> b = panelBox<UI>();
        sf::Vector2f q;
        bool inside = mapPanelCoords<UI>(mouse, q);
        sf::Vector2f pm = inside ? q : sf::Vector2f(-10000, -10000);

        unsigned w = (unsigned)std::ceil(b.vw);
        unsigned h = (unsigned)std::ceil(b.vh);
        bool resized = (w != panelRTW || h != panelRTH);
        if (resized) {
            if (!panelRT.create(w, h)) return;
            panelRTW = w;
            panelRTH = h;
            panelRTValid = false;
        }
        panelRTFrame++;
        bool hoverChanged = (pm.x != panelRTMouse.x || pm.y != panelRTMouse.y);
        // repaint on change, or ~5 fps for the animated scopes/meters
        int refresh = transportPlaying.load() ? 3 : 8;
        if (panelDirty || !panelRTValid || resized || key != panelRTKey || hoverChanged || panelRTFrame >= refresh) {
            panelRTFrame = 0;
            panelRTKey = key;
            panelRTMouse = pm;
            panelDirty = false;
            panelRT.setView(sf::View(sf::FloatRect(0, UI::PANEL_TOP, b.cw, b.ch)));
            panelUi.draw(panelRT, pm);
            panelRT.display();
            panelRTValid = true;
        }

        sf::Sprite sp(panelRT.getTexture());
        sp.setPosition(b.ox, b.oy);
        sp.setScale(b.vw / (float)panelRTW, b.vh / (float)panelRTH);
        t.draw(sp);
    }

    void drawInstrument(sf::RenderTarget& t, Track& tr, sf::Vector2f mouse)
    {
        int key = tr.index * ENGINE_COUNT + tr.type;
        if (tr.type == ENGINE_KICK) drawPanelView(t, *tr.kickUi, mouse, key);
        else if (tr.type == ENGINE_ACID) drawPanelView(t, *tr.acidUi, mouse, key);
        else drawPanelView(t, *tr.fm4Ui, mouse, key);
    }

    void panelMousePressed(Track& tr, sf::Vector2f q, sf::Mouse::Button b, bool shift)
    {
        if (tr.type == ENGINE_KICK) tr.kickUi->mousePressed(q, b, shift);
        else if (tr.type == ENGINE_ACID) tr.acidUi->mousePressed(q, b, shift);
        else tr.fm4Ui->mousePressed(q, b, shift);
    }

    void panelMouseMoved(Track& tr, sf::Vector2f q, bool shift)
    {
        if (tr.type == ENGINE_KICK) tr.kickUi->mouseMoved(q, shift);
        else if (tr.type == ENGINE_ACID) tr.acidUi->mouseMoved(q, shift);
        else tr.fm4Ui->mouseMoved(q, shift);
    }

    void panelWheel(Track& tr, sf::Vector2f q, float delta, bool shift)
    {
        if (tr.type == ENGINE_KICK) tr.kickUi->wheel(q, delta, shift);
        else if (tr.type == ENGINE_ACID) tr.acidUi->wheel(q, delta, shift);
        else tr.fm4Ui->wheel(q, delta, shift);
    }

    void panelKey(Track& tr, sf::Keyboard::Key k)
    {
        if (tr.type == ENGINE_KICK) tr.kickUi->key(k);
        else if (tr.type == ENGINE_ACID) tr.acidUi->key(k);
        else tr.fm4Ui->key(k);
    }

    // When the instrument panel is shorter than the left column, use the
    // leftover space for a live output scope instead of leaving a hole.
    void drawOutputScope(sf::RenderTarget& t, Track& tr)
    {
        float contentBottom = layout.panel.top;
        const float* ring = nullptr;
        int ringSize = 0, ringPos = 0;

        if (tr.type == ENGINE_KICK) {
            PanelBox<kick_ui::EmuUi> b = panelBox<kick_ui::EmuUi>();
            contentBottom = b.oy + b.vh;
            ring = tr.kickCap.ring;
            ringSize = kick_ui::Capture::RING;
            ringPos = tr.kickCap.ringPos;
        } else if (tr.type == ENGINE_ACID) {
            PanelBox<acid_ui::EmuUi> b = panelBox<acid_ui::EmuUi>();
            contentBottom = b.oy + b.vh;
            ring = tr.acidCap.ring;
            ringSize = acid_ui::AcidCapture::RING;
            ringPos = tr.acidCap.ringPos;
        } else {
            PanelBox<fm4_ui::EmuUi> b = panelBox<fm4_ui::EmuUi>();
            contentBottom = b.oy + b.vh;
            ring = tr.fm4Cap.ring;
            ringSize = fm4_ui::Fm4Capture::RING;
            ringPos = tr.fm4Cap.ringPos;
        }

        float top = contentBottom + 8.0f;
        float h = layout.panel.top + layout.panel.height - top - 8.0f;
        if (h < 44.0f || !ring) return;

        sf::FloatRect r { layout.panel.left + 24, top, layout.panel.width - 48, h };
        rrect(t, r, 10.0f, sf::Color(16, 18, 24), Theme::cardLine);
        label(t, std::string(engineName(tr.type)) + " OUTPUT", r.left + 14, r.top + 7, 8, Theme::muted, false, true, 1.6f);
        label(t, "live scope", r.left + 14, r.top + 20, 7, Theme::faint, false, false, 0.6f);

        // grid
        float mid = r.top + r.height * 0.58f;
        float amp = std::max(8.0f, r.height * 0.32f);
        for (int i = 1; i < 4; ++i)
            rrect(t, { r.left + 14, r.top + r.height * i / 4.0f, r.width - 28, 1 }, 0, sf::Color(255, 255, 255, 10));
        for (int i = 1; i < 8; ++i)
            rrect(t, { r.left + 14 + (r.width - 28) * i / 8.0f, r.top + 8, 1, r.height - 16 }, 0, sf::Color(255, 255, 255, 8));
        rrect(t, { r.left + 14, mid, r.width - 28, 1 }, 0, sf::Color(255, 255, 255, 20));

        // waveform: bucket min/max so dense rings still look like a scope
        int cols = std::max(32, (int)r.width - 28);
        int span = std::max(1, ringSize / 4);
        sf::VertexArray ribbon(sf::TriangleStrip);
        for (int i = 0; i < cols; ++i) {
            int idx = ringPos - span + (int)((long long)i * span / cols);
            float lo = 0.0f, hi = 0.0f;
            int n = std::max(1, span / cols);
            for (int k = 0; k < n; ++k) {
                int q = ((idx + k) % ringSize + ringSize) % ringSize;
                lo = std::min(lo, ring[q]);
                hi = std::max(hi, ring[q]);
            }
            float x = r.left + 14 + i;
            sf::Color c = tr.color;
            c.a = 170;
            ribbon.append(sf::Vertex({ x, mid - hi * amp }, c));
            ribbon.append(sf::Vertex({ x, mid - lo * amp }, c));
        }
        t.draw(ribbon);

        label(t, "OUT", r.left + r.width - 14, r.top + 7, 7, Theme::faint, false);
    }

    // ---------------------------------------------------------------- top bar
    // On wide screens the transport sits in the middle of the bar and the
    // master section is pinned to the right edge; on narrow screens the whole
    // strip is packed from the left so nothing collides.
    float transportBase() const { return layout.wide ? layout.W * 0.5f - 103.0f : 718.0f; }
    sf::FloatRect trackTab(int i) const { return { 146.0f + i * 70.0f, 8.0f, 66.0f, 36.0f }; }
    sf::FloatRect playBtn() const { return { transportBase(), 8.0f, 54.0f, 36.0f }; }
    sf::FloatRect bpmMinus() const { return { transportBase() + 70.0f, 8.0f, 30.0f, 36.0f }; }
    sf::FloatRect bpmBox() const { return { transportBase() + 104.0f, 8.0f, 76.0f, 36.0f }; }
    sf::FloatRect bpmPlus() const { return { transportBase() + 184.0f, 8.0f, 30.0f, 36.0f }; }
    sf::FloatRect masterBox() const { return { layout.wide ? layout.W - 314.0f : 954.0f, 8.0f, 168.0f, 36.0f }; }
    sf::FloatRect meterBox() const { return { masterBox().left + 180.0f, 8.0f, 118.0f, 36.0f }; }

    // ---------------------------------------------------------------- sidebar (track list + transport + master FX)
    void drawSidebar(sf::RenderTarget& t, sf::Vector2f mouse)
    {
        float x = layout.sidebar.left;
        float w = layout.sidebar.width;
        float H = layout.H;
        rrect(t, layout.sidebar, 0, sf::Color(14, 16, 21));
        sf::RectangleShape edge({ 1, H });
        edge.setPosition(w - 1, 0);
        edge.setFillColor(Theme::cardLine);
        t.draw(edge);

        // title
        label(t, "zic", x + 12, 12, 20, Theme::text, false, true, 0.4f);
        label(t, "Studio", x + 44, 12, 20, Theme::accent, false, true, 0.4f);
        seqPosRect = { x + w - 54, 12, 46, 20 };
        rrect(t, seqPosRect, 6.0f, Theme::card, Theme::cardLine);
        label(t, layout.seqRight ? "SEQ >" : "SEQ v", seqPosRect.left + seqPosRect.width * 0.5f,
              seqPosRect.top + 6, 7, Theme::muted, true, true, 1.0f);

        // cpu meter under the title
        cpuRect = { x + 12, 40, w - 24, 14 };
        rrect(t, cpuRect, 4.0f, Theme::card, Theme::cardLine);
        char cpubuf[40];
        snprintf(cpubuf, sizeof(cpubuf), "CPU %.0f%%  %.0ffps", cpuUsage.load(), uiFps.load());
        label(t, cpubuf, cpuRect.left + 6, cpuRect.top + 3, 8, Theme::muted, false, false, 0.4f);
        float cpuLvl = std::clamp(cpuUsage.load() / 100.0f, 0.0f, 1.0f);
        rrect(t, { cpuRect.left + cpuRect.width - 46, cpuRect.top + 3, 40, 8 }, 3, Theme::cell);
        rrect(t, { cpuRect.left + cpuRect.width - 46, cpuRect.top + 3, 40 * cpuLvl, 8 }, 3,
            cpuLvl > 0.85f ? sf::Color(255, 86, 98) : (cpuLvl > 0.6f ? sf::Color(255, 200, 60) : Theme::accent));

        // track list, right under the title
        float y = 64.0f;
        float rowH = 50.0f;
        for (int i = 0; i < TRACK_COUNT; ++i) {
            Track& tr = *tracks[i];
            bool sel = (selectedTrack.load() == i);
            sf::FloatRect row { x + 8, y, w - 16, rowH - 4 };
            trackRowRect[i] = row;
            rrect(t, row, 7.0f, sel ? sf::Color(30, 34, 44) : Theme::card, sel ? tr.color : Theme::cardLine);

            rrect(t, { row.left + 7, row.top + 9, 6, 6 }, 3.0f, tr.color);
            char nb[8];
            snprintf(nb, sizeof(nb), "%02d", i + 1);
            label(t, nb, row.left + 18, row.top + 6, 11, sel ? tr.color : Theme::text, false, true, 0.4f);

            // engine name + cog on the second line
            label(t, engineName(tr.type), row.left + 18, row.top + 25, 9, Theme::muted, false, true, 0.8f);
            sf::FloatRect cg { row.left + 68, row.top + 24, 14, 14 };
            trackCogRect[i] = cg;
            bool chot = cg.contains(mouse);
            rrect(t, cg, 4.0f, chot ? sf::Color(48, 52, 64) : Theme::cell, Theme::cardLine);
            drawCog(t, cg.left + 7, cg.top + 7, 4.0f, Theme::muted);

            // volume slider (top line, M/S to its right)
            sf::FloatRect vr { row.left + 40, row.top + 8, row.width - 90, 6 };
            trackVolRect[i] = vr;
            rrect(t, vr, 3.0f, Theme::cell);
            rrect(t, { vr.left, vr.top, vr.width * tr.volume, vr.height }, 3.0f, sel ? tr.color : Theme::accent);

            // mute / solo
            sf::FloatRect mr { row.left + row.width - 44, row.top + 5, 18, 16 };
            sf::FloatRect sr { row.left + row.width - 22, row.top + 5, 18, 16 };
            trackMuteRect[i] = mr;
            trackSoloRect[i] = sr;
            rrect(t, mr, 4.0f, tr.mute ? sf::Color(230, 70, 80) : Theme::cell, Theme::cardLine);
            rrect(t, sr, 4.0f, tr.solo ? sf::Color(255, 200, 60) : Theme::cell, Theme::cardLine);
            label(t, "M", mr.left + 6, mr.top + 2, 8, tr.mute ? sf::Color(12, 14, 16) : Theme::muted, true);
            label(t, "S", sr.left + 6, sr.top + 2, 8, tr.solo ? sf::Color(12, 14, 16) : Theme::muted, true);

            // activity meter hugging the row edge
            float lvl = std::clamp(tr.meter.load(), 0.0f, 1.0f);
            if (!tr.mute && lvl > 0.02f)
                rrect(t, { row.left + row.width - 2, row.top + 4, 2, row.height - 8 }, 1.0f, sel ? tr.color : Theme::accent);
            y += rowH;
        }

        // engine cog menu (drawn above everything else in the sidebar)
        if (engineMenuTrack >= 0 && engineMenuTrack < TRACK_COUNT) {
            sf::FloatRect cg = trackCogRect[engineMenuTrack];
            sf::FloatRect menu { x + 8, cg.top + cg.height + 2, w - 16, ENGINE_COUNT * 26.0f + 4 };
            if (menu.top + menu.height > H) menu.top = cg.top - menu.height - 2;
            rrect(t, menu, 7.0f, Theme::card, Theme::cardLine);
            for (int i = 0; i < ENGINE_COUNT; ++i) {
                sf::FloatRect ir { menu.left + 2, menu.top + 2 + i * 26.0f, menu.width - 4, 26 };
                engineDropItemRect[i] = ir;
                bool hot = ir.contains(mouse);
                bool cur = tracks[engineMenuTrack]->type == i;
                rrect(t, ir, 5.0f, cur ? tracks[engineMenuTrack]->color : (hot ? sf::Color(38, 42, 52) : Theme::card), Theme::cardLine);
                label(t, engineName(i), ir.left + 8, ir.top + 7, 9, cur ? sf::Color(12, 12, 14) : Theme::text, false, true, 0.6f);
            }
        }

        // ---- sticky bottom: master FX, master volume, transport
        float bottom = H - 10.0f;

        // transport: play + tempo
        playBtnRect = { x + 12, bottom - 92, w - 24, 34 };
        bool playing = transportPlaying.load();
        rrect(t, playBtnRect, 9.0f, playing ? sf::Color(70, 210, 130) : Theme::card, playing ? sf::Color(70, 210, 130) : Theme::cardLine);
        if (playing) {
            sf::RectangleShape stop({ 12, 12 });
            stop.setPosition(playBtnRect.left + (playBtnRect.width - 12) * 0.5f, playBtnRect.top + 11);
            stop.setFillColor(sf::Color(12, 14, 16));
            t.draw(stop);
        } else {
            float cx = playBtnRect.left + playBtnRect.width * 0.5f;
            sf::ConvexShape pl(3);
            pl.setPoint(0, { cx - 6, playBtnRect.top + 9 });
            pl.setPoint(1, { cx + 8, playBtnRect.top + 17 });
            pl.setPoint(2, { cx - 6, playBtnRect.top + 25 });
            pl.setFillColor(Theme::text);
            t.draw(pl);
        }
        label(t, playing ? "STOP" : "PLAY", playBtnRect.left + 12, playBtnRect.top + 10, 10, playing ? sf::Color(12, 14, 16) : Theme::text, false, true, 1.0f);

        bpmMinusRect = { x + 12, bottom - 52, 30, 28 };
        bpmBoxRect = { x + 46, bottom - 52, w - 92, 28 };
        bpmPlusRect = { x + w - 42, bottom - 52, 30, 28 };
        rrect(t, bpmMinusRect, 7.0f, Theme::card, Theme::cardLine);
        rrect(t, bpmBoxRect, 7.0f, Theme::card, Theme::cardLine);
        rrect(t, bpmPlusRect, 7.0f, Theme::card, Theme::cardLine);
        label(t, "-", bpmMinusRect.left + 12, bpmMinusRect.top + 6, 16, Theme::muted, true);
        label(t, "+", bpmPlusRect.left + 12, bpmPlusRect.top + 6, 16, Theme::muted, true);
        char buf[32];
        snprintf(buf, sizeof(buf), "%.0f", transportBpm.load());
        label(t, buf, bpmBoxRect.left + bpmBoxRect.width * 0.5f, bpmBoxRect.top + 4, 15, Theme::text, true, true, 0.5f);
        label(t, "BPM", bpmBoxRect.left + bpmBoxRect.width * 0.5f, bpmBoxRect.top + 20, 7, Theme::muted, true, false, 1.4f);

        // master volume + meter
        masterVolRect = { x + 12, bottom - 162, w - 24, 22 };
        rrect(t, masterVolRect, 7.0f, Theme::card, Theme::cardLine);
        sf::FloatRect mbar { masterVolRect.left + 8, masterVolRect.top + 13, masterVolRect.width - 16, 5 };
        rrect(t, mbar, 2.5f, Theme::cell);
        float mv = masterVolume.load();
        rrect(t, { mbar.left, mbar.top, mbar.width * mv, mbar.height }, 2.5f, Theme::accent);
        label(t, "MASTER", masterVolRect.left + 8, masterVolRect.top + 2, 7, Theme::muted, false);
        label(t, std::to_string((int)std::round(mv * 100)) + "%", masterVolRect.left + masterVolRect.width - 36, masterVolRect.top + 2, 7, Theme::text, false);

        label(t, "OUT", x + 12, bottom - 132, 7, Theme::muted, false, false, 1.2f);
        sf::FloatRect meterR { x + 44, bottom - 133, w - 56, 8 };
        rrect(t, meterR, 4.0f, Theme::cell);
        float lvl = std::clamp(masterMeter.load() * 0.82f, 0.0f, 1.0f);
        if (lvl > 0.01f) {
            sf::Color mc = lvl > 0.92f ? sf::Color(255, 86, 98) : (lvl > 0.7f ? sf::Color(255, 200, 60) : Theme::accent);
            rrect(t, { meterR.left, meterR.top, meterR.width * lvl, meterR.height }, 4.0f, mc);
        }

        // master FX in one row above the master volume
        label(t, "MASTER FX", x + 12, bottom - 236, 8, Theme::muted, false, true, 1.6f);
        float knobR = 14.0f;
        float spacing = (w - 24) / (float)FX_KNOBS;
        const char* fxNames[FX_KNOBS] = { "DLY", "FDBK", "DLVL", "REV", "RVLV" };
        for (int i = 0; i < FX_KNOBS; ++i) {
            float cx = x + 12 + spacing * (i + 0.5f);
            float cy = bottom - 214 + knobR;
            fxKnobRect[i] = { cx - knobR - 2, cy - knobR - 2, knobR * 2 + 4, knobR * 2 + 4 };
            float v = fxParam(i).load();
            sf::CircleShape knob(knobR);
            knob.setOrigin(knobR, knobR);
            knob.setPosition(cx, cy);
            knob.setFillColor(sf::Color(27, 30, 38));
            knob.setOutlineThickness(1.5f);
            knob.setOutlineColor(Theme::cardLine);
            t.draw(knob);
            float a = (135.0f + 270.0f * v) * 3.14159265f / 180.0f;
            sf::RectangleShape ind({ knobR - 4, 2.0f });
            ind.setOrigin(0, 1);
            ind.setPosition(cx, cy);
            ind.setRotation(a * 180.0f / 3.14159265f);
            ind.setFillColor(Theme::text);
            t.draw(ind);
            label(t, fxNames[i], cx, cy + knobR + 2, 6, Theme::muted, true, true, 1.0f);
            char vbuf[16];
            snprintf(vbuf, sizeof(vbuf), "%.0f", v * 100);
            label(t, vbuf, cx, cy + knobR + 10, 6, Theme::text, true, false, 0.6f);
        }
    }

    // small gear glyph drawn with primitives (no font dependency)
    static void drawCog(sf::RenderTarget& t, float cx, float cy, float r, sf::Color c)
    {
        for (int i = 0; i < 8; ++i) {
            float a = i * 0.785398f;
            sf::RectangleShape tooth({ 2.0f, 3.0f });
            tooth.setOrigin(1.0f, 1.5f);
            tooth.setPosition(cx + std::cos(a) * (r + 1.2f), cy + std::sin(a) * (r + 1.2f));
            tooth.setRotation(a * 180.0f / 3.14159265f + 90.0f);
            tooth.setFillColor(c);
            t.draw(tooth);
        }
        sf::CircleShape ring(r);
        ring.setOrigin(r, r);
        ring.setPosition(cx, cy);
        ring.setFillColor(sf::Color::Transparent);
        ring.setOutlineThickness(1.6f);
        ring.setOutlineColor(c);
        t.draw(ring);
        sf::CircleShape hole(r * 0.35f);
        hole.setOrigin(r * 0.35f, r * 0.35f);
        hole.setPosition(cx, cy);
        hole.setFillColor(c);
        t.draw(hole);
    }

    sf::FloatRect splitterRect()
    {
        if (layout.seqRight)
            return { layout.seq.left - 4, 0, 8, layout.H };
        return { layout.sidebar.width, layout.seq.top - 4, layout.W - layout.sidebar.width, 8 };
    }

    void drawSplitter(sf::RenderTarget& t, sf::Vector2f mouse)
    {
        sf::FloatRect r = splitterRect();
        bool hot = r.contains(mouse) || dragSplit;
        rrect(t, r, 0.0f, hot ? sf::Color(56, 60, 74) : sf::Color(22, 25, 32));
        if (layout.seqRight) {
            for (int i = 0; i < 3; ++i)
                rrect(t, { r.left + 2.5f, layout.H * 0.5f - 18 + i * 14.0f, 3, 8 }, 1.5f, Theme::muted);
        } else {
            for (int i = 0; i < 3; ++i)
                rrect(t, { layout.W * 0.5f - 18 + i * 14.0f, r.top + 2.5f, 8, 3 }, 1.5f, Theme::muted);
        }
    }

    std::atomic<float>& fxParam(int i)
    {
        switch (i) {
        case 0: return fxDelayTime;
        case 1: return fxDelayFeedback;
        case 2: return fxDelayLevel;
        case 3: return fxReverbSize;
        default: return fxReverbLevel;
        }
    }

    void sidebarPressed(sf::Vector2f p, sf::Mouse::Button button, bool shift)
    {
        // engine cog menu handling first
        if (engineMenuTrack >= 0) {
            for (int i = 0; i < ENGINE_COUNT; ++i) {
                if (engineDropItemRect[i].contains(p)) {
                    std::lock_guard<std::mutex> lock(audioMutex);
                    tracks[engineMenuTrack]->setType(i);
                    engineMenuTrack = -1;
                    return;
                }
            }
            bool onCog = false;
            for (int i = 0; i < TRACK_COUNT; ++i) if (trackCogRect[i].contains(p)) onCog = true;
            if (!onCog) { engineMenuTrack = -1; return; }
        }
        for (int i = 0; i < TRACK_COUNT; ++i) {
            if (trackCogRect[i].contains(p)) { engineMenuTrack = (engineMenuTrack == i) ? -1 : i; return; }
        }

        if (seqPosRect.contains(p)) {
            layout.seqRight = !layout.seqRight;
            seqOnRight = layout.seqRight;
            seqSplit = layout.seqRight ? 0.0f : 0.35f;
            layout.split = seqSplit;
            relayout(layout.W, layout.H);
            return;
        }
        if (playBtnRect.contains(p)) { transportPlaying = !transportPlaying.load(); return; }
        if (bpmMinusRect.contains(p)) { transportBpm = std::max(60.0f, transportBpm.load() - (shift ? 10.0f : 1.0f)); return; }
        if (bpmPlusRect.contains(p)) { transportBpm = std::min(300.0f, transportBpm.load() + (shift ? 10.0f : 1.0f)); return; }
        if (bpmBoxRect.contains(p)) { transportBpm = std::clamp(transportBpm.load() + (shift ? 10.0f : 1.0f), 60.0f, 300.0f); return; }

        for (int i = 0; i < TRACK_COUNT; ++i) {
            if (trackMuteRect[i].contains(p)) { tracks[i]->mute = !tracks[i]->mute; return; }
            if (trackSoloRect[i].contains(p)) { tracks[i]->solo = !tracks[i]->solo; return; }
            if (trackVolRect[i].contains(p)) {
                dragSidebarVol = i;
                float tv = std::clamp((p.x - trackVolRect[i].left) / trackVolRect[i].width, 0.0f, 1.0f);
                tracks[i]->volume = tv;
                return;
            }
            if (trackRowRect[i].contains(p)) { selectedTrack = i; return; }
        }
        if (masterVolRect.contains(p)) {
            float tv = std::clamp((p.x - (masterVolRect.left + 8)) / (masterVolRect.width - 16), 0.0f, 1.0f);
            masterVolume = tv;
            dragMasterVol = true;
            return;
        }
        for (int i = 0; i < FX_KNOBS; ++i) {
            if (fxKnobRect[i].contains(p)) {
                dragFx = (FxKnob)(FxDelayTime + i);
                dragFxStart = fxParam(i).load();
                dragFxY = p.y;
                return;
            }
        }
        (void)button;
    }

    void sidebarVolDrag(sf::Vector2f p)
    {
        if (dragSidebarVol < 0 || dragSidebarVol >= TRACK_COUNT) return;
        float tv = std::clamp((p.x - trackVolRect[dragSidebarVol].left) / trackVolRect[dragSidebarVol].width, 0.0f, 1.0f);
        tracks[dragSidebarVol]->volume = tv;
    }

    void masterVolDrag(sf::Vector2f p)
    {
        float tv = std::clamp((p.x - (masterVolRect.left + 8)) / (masterVolRect.width - 16), 0.0f, 1.0f);
        masterVolume = tv;
    }

    void sidebarWheel(sf::Vector2f p, float delta, bool shift)
    {
        for (int i = 0; i < TRACK_COUNT; ++i) {
            if (trackRowRect[i].contains(p) || trackVolRect[i].contains(p)) {
                tracks[i]->volume = std::clamp(tracks[i]->volume + (delta > 0 ? 0.05f : -0.05f) * (shift ? 4.0f : 1.0f), 0.0f, 1.0f);
                return;
            }
        }
        if (masterVolRect.contains(p)) {
            masterVolume = std::clamp(masterVolume.load() + (delta > 0 ? 0.03f : -0.03f) * (shift ? 5.0f : 1.0f), 0.0f, 1.0f);
            return;
        }
        if (bpmBoxRect.contains(p) || bpmMinusRect.contains(p) || bpmPlusRect.contains(p)) {
            transportBpm = std::clamp(transportBpm.load() + (delta > 0 ? 1.0f : -1.0f) * (shift ? 10.0f : 1.0f), 60.0f, 300.0f);
            return;
        }
        for (int i = 0; i < FX_KNOBS; ++i) {
            if (fxKnobRect[i].contains(p)) {
                float v = fxParam(i).load();
                fxParam(i) = std::clamp(v + (delta > 0 ? 0.03f : -0.03f) * (shift ? 4.0f : 1.0f), 0.0f, 1.0f);
                return;
            }
        }
    }

    void fxDragSidebar(sf::Vector2f p)
    {
        if (dragFx == FxNone) return;
        int i = (int)dragFx - (int)FxDelayTime;
        if (i < 0 || i >= FX_KNOBS) return;
        float v = std::clamp(dragFxStart + (dragFxY - p.y) / 150.0f, 0.0f, 1.0f);
        fxParam(i) = v;
    }


    struct SeqGeom {
        sf::FloatRect card, len16, len32, trMinus, trBox, trPlus, clear;
        sf::FloatRect editNote, editLen, editVel, editSlide, editAccent;
        float left, top, w, h, colW, rowH;
        int cols, lo, hi;
        bool editStrip = false;
    };

    SeqGeom seqGeom(Track& tr)
    {
        SeqGeom g;
        g.card = layout.seq;
        float toolY = layout.seq.top + 28.0f;
        g.len16 = { layout.seq.left + 12, toolY, 38, 20 };
        g.len32 = { layout.seq.left + 54, toolY, 38, 20 };
        g.clear = { layout.seq.left + layout.seq.width - 58, toolY, 46, 20 };
        g.trPlus = { g.clear.left - 12 - 20, toolY, 20, 20 };
        g.trBox = { g.trPlus.left - 4 - 46, toolY, 46, 20 };
        g.trMinus = { g.trBox.left - 4 - 20, toolY, 20, 20 };

        // edit strip on its own row, so it is available at every width
        g.editStrip = true;
        {
            float ew = (layout.seq.width - 24.0f - 4 * 6.0f) / 5.0f;
            float ex = layout.seq.left + 12.0f;
            float ey = toolY + 25.0f;
            g.editNote = { ex, ey, ew, 18 }; ex += ew + 6;
            g.editLen = { ex, ey, ew, 18 }; ex += ew + 6;
            g.editVel = { ex, ey, ew, 18 }; ex += ew + 6;
            g.editSlide = { ex, ey, ew, 18 }; ex += ew + 6;
            g.editAccent = { ex, ey, ew, 18 };
        }

        g.top = layout.seq.top + 88.0f;
        g.h = layout.seq.height - 96.0f;
        g.left = layout.seq.left + 12.0f;
        g.w = layout.seq.width - 24.0f;

        if (tr.type == ENGINE_KICK) {
            g.cols = 16;
        } else {
            g.left += 34.0f;
            g.w -= 34.0f;
            g.cols = tr.type == ENGINE_ACID
                ? std::clamp(tr.acidSeq->patternLength, 1, AcidSequencer::STEPS)
                : std::clamp(tr.fm4Seq->patternLength, 1, Fm4Sequencer::STEPS);
        }
        g.colW = g.w / g.cols;

        if (tr.type == ENGINE_KICK) {
            g.lo = -24;
            g.hi = 24;
            g.rowH = g.h / 4.0f;
        } else {
            g.lo = 127;
            g.hi = 0;
            if (tr.type == ENGINE_ACID) {
                for (int i = 0; i < g.cols; ++i)
                    if (tr.acidSeq->steps[i].on) {
                        g.lo = std::min(g.lo, (int)tr.acidSeq->steps[i].note);
                        g.hi = std::max(g.hi, (int)tr.acidSeq->steps[i].note);
                    }
            } else {
                for (int i = 0; i < g.cols; ++i)
                    if (tr.fm4Seq->steps[i].on) {
                        g.lo = std::min(g.lo, (int)tr.fm4Seq->steps[i].note);
                        g.hi = std::max(g.hi, (int)tr.fm4Seq->steps[i].note);
                    }
            }
            if (g.hi < g.lo) { g.lo = 36; g.hi = 48; }
            if (g.hi - g.lo < 12) { int m = (g.lo + g.hi) / 2; g.lo = m - 6; g.hi = m + 6; }
            // fill the tall right-hand panel with a useful octave range
            int minRows = std::max(12, (int)(g.h / 24.0f));
            int rows = g.hi - g.lo + 1;
            if (rows < minRows) {
                int extra = minRows - rows;
                g.lo -= extra / 2;
                g.hi += extra - extra / 2;
            }
            g.lo = std::max(0, g.lo);
            g.hi = std::min(127, g.hi);
            g.rowH = g.h / std::max(1, g.hi - g.lo + 1);
        }
        return g;
    }

    int& seqTranspose(Track& tr)
    {
        if (tr.type == ENGINE_KICK) return tr.kickSeq->transpose;
        if (tr.type == ENGINE_ACID) return tr.acidSeq->transpose;
        return tr.fm4Seq->transpose;
    }

    int seqTransposeMax(Track& tr) { return tr.type == ENGINE_KICK ? 24 : 48; }

    static const char* noteName(int n)
    {
        static const char* names[12] = { "C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B" };
        static char buf[12];
        snprintf(buf, sizeof(buf), "%s%d", names[((n % 12) + 12) % 12], n / 12 - 1);
        return buf;
    }

    void drawSequencer(sf::RenderTarget& t, Track& tr, sf::Vector2f mouse)
    {
        const sf::FloatRect card = layout.seq;
        rrect(t, card, 0, sf::Color(14, 16, 21));

        int cur = -1;
        int steps = Sequencer::STEPS;
        if (tr.type == ENGINE_KICK) cur = tr.kickSeq->current.load();
        else if (tr.type == ENGINE_ACID) { cur = tr.acidSeq->currentStep.load(); steps = tr.acidSeq->patternLength; }
        else { cur = tr.fm4Seq->currentStep.load(); steps = tr.fm4Seq->patternLength; }

        label(t, "SEQUENCER", card.left + 12, card.top + 7, 9, Theme::text, false, true, 1.8f);

        char info[80];
        snprintf(info, sizeof(info), "%s  -  %d steps  -  1/16", engineName(tr.type), steps);
        label(t, info, card.left + 92, card.top + 8, 8, Theme::muted, false, false, 0.4f);

        if (card.width > 600.0f)
            labelR(t, "click draw  /  wheel pitch  /  shift+wheel vel  /  shift+click accent  /  right clear",
                   card.left + card.width - 12, card.top + 8, 7, Theme::faint);

        SeqGeom g = seqGeom(tr);
        auto pill = [&](sf::FloatRect r, const char* txt, bool active, sf::Color c) {
            bool hot = r.contains(mouse);
            rrect(t, r, 6.0f, active ? c : (hot ? sf::Color(37, 41, 51) : Theme::cell), active ? c : Theme::cardLine);
            label(t, txt, r.left + r.width * 0.5f, r.top + r.height * 0.5f - 4.0f, 8, active ? sf::Color(12, 12, 14) : Theme::text, true, true, 0.6f);
        };

        if (tr.type != ENGINE_KICK) {
            int len = tr.type == ENGINE_ACID ? tr.acidSeq->patternLength : tr.fm4Seq->patternLength;
            pill(g.len16, "16", len == 16, Theme::accent);
            pill(g.len32, "32", len == 32, Theme::accent);
        } else {
            label(t, "64 STEPS", g.len16.left, g.len16.top + 5, 8, Theme::faint, false, false, 1.4f);
        }
        pill(g.clear, "CLEAR", false, sf::Color(230, 70, 80));

        // transpose stepper
        auto symBtn = [&](sf::FloatRect r, const char* s) {
            bool hot = r.contains(mouse);
            rrect(t, r, 6.0f, hot ? sf::Color(37, 41, 51) : Theme::cell, Theme::cardLine);
            label(t, s, r.left + r.width * 0.5f, r.top + r.height * 0.5f - 4.0f, 10, Theme::text, true, true, 0.2f);
        };
        symBtn(g.trMinus, "-");
        symBtn(g.trPlus, "+");
        rrect(t, g.trBox, 6.0f, Theme::cell, Theme::cardLine);
        char tb[24];
        snprintf(tb, sizeof(tb), "TR %+d", seqTranspose(tr));
        label(t, tb, g.trBox.left + g.trBox.width * 0.5f, g.trBox.top + 6, 8, Theme::text, true, false, 0.4f);

        // selected-step edit strip (mirrors the standalone sequencers)
        if (g.editStrip) {
            auto editBox = [&](sf::FloatRect r, const char* txt, bool active, sf::Color acc) {
                bool hot = r.contains(mouse);
                rrect(t, r, 6.0f, active ? acc : (hot ? sf::Color(37, 41, 51) : Theme::cell), Theme::cardLine);
                label(t, txt, r.left + r.width * 0.5f, r.top + r.height * 0.5f - 4.0f, 7,
                      active ? sf::Color(12, 12, 14) : Theme::text, true, true, 0.4f);
            };
            if (seqSelected >= 0 && seqSelected < g.cols) {
                bool on = false, accent = false, slide = false;
                int note = 0;
                float vel = 0.8f, len = 1.0f;
                if (tr.type == ENGINE_KICK) {
                    on = tr.kickSeq->steps[seqSelected].on;
                    note = 60 + tr.kickSeq->steps[seqSelected].note;
                } else if (tr.type == ENGINE_ACID) {
                    auto& s = tr.acidSeq->steps[seqSelected];
                    on = s.on; note = s.note; vel = s.velocity; len = s.length; accent = s.accent; slide = s.slide;
                } else {
                    auto& s = tr.fm4Seq->steps[seqSelected];
                    on = s.on; note = s.note; vel = s.velocity; len = s.length; accent = s.accent; slide = s.slide;
                }
                char b[24];
                snprintf(b, sizeof(b), "%s", tr.type == ENGINE_KICK ? (noteName(note)) : noteName(note));
                editBox(g.editNote, b, on, tr.color);
                snprintf(b, sizeof(b), "L %.2f", len);
                editBox(g.editLen, b, false, Theme::accent);
                snprintf(b, sizeof(b), "V %d", (int)std::lround(vel * 100.0f));
                editBox(g.editVel, b, false, Theme::accent);
                editBox(g.editSlide, "SLIDE", slide, Theme::accent);
                editBox(g.editAccent, "ACC", accent, sf::Color(255, 176, 64));
            } else {
                editBox(g.editNote, "NOTE", false, Theme::accent);
                editBox(g.editLen, "LEN", false, Theme::accent);
                editBox(g.editVel, "VEL", false, Theme::accent);
                editBox(g.editSlide, "SLIDE", false, Theme::accent);
                editBox(g.editAccent, "ACC", false, sf::Color(255, 176, 64));
            }
        }

        // grid frame
        rrect(t, { g.left, g.top, g.w, g.h }, 4.0f, sf::Color(16, 18, 24), Theme::cardLine);

        if (tr.type == ENGINE_KICK) {
            for (int row = 0; row < 4; ++row)
                for (int col = 0; col < 16; ++col) {
                    int i = row * 16 + col;
                    if (i >= Sequencer::STEPS) continue;
                    sf::FloatRect r { g.left + col * g.colW + 2, g.top + row * g.rowH + 3, g.colW - 4, g.rowH - 6 };
                    bool on = tr.kickSeq->steps[i].on;
                    bool isCur = i == cur;
                    bool hot = r.contains(mouse);
                    sf::Color fill = on ? tr.color : (hot ? sf::Color(34, 38, 47) : (isCur ? sf::Color(30, 34, 42) : Theme::cell));
                    rrect(t, r, 5.0f, fill, isCur ? Theme::text : (on ? sf::Color(0, 0, 0, 40) : sf::Color::Transparent));
                    if (on) {
                        char nb[8];
                        if (tr.kickSeq->steps[i].note == 0) snprintf(nb, sizeof(nb), "·");
                        else snprintf(nb, sizeof(nb), "%+d", tr.kickSeq->steps[i].note);
                        label(t, nb, r.left + r.width * 0.5f, r.top + r.height * 0.5f - 6, 9, sf::Color(12, 12, 14), true, true);
                    }
                    if (row == 0 && col % 4 == 0) {
                        char sn[16];
                        snprintf(sn, sizeof(sn), "%d", col + 1);
                        label(t, sn, r.left + 5, r.top + 4, 6, Theme::faint, false);
                    }
                }
        } else {
            // piano roll rows
            for (int y = 0; y <= g.hi - g.lo; ++y) {
                int note = g.hi - y;
                bool black = (note % 12 == 1 || note % 12 == 3 || note % 12 == 6 || note % 12 == 8 || note % 12 == 10);
                sf::Color rowc = black ? sf::Color(18, 20, 27) : sf::Color(21, 24, 31);
                rrect(t, { g.left, g.top + y * g.rowH, g.w, g.rowH }, 0, rowc);
                if (note % 12 == 0) {
                    rrect(t, { g.left, g.top + y * g.rowH, g.w, 1 }, 0, sf::Color(255, 255, 255, 28));
                    label(t, noteName(note), g.left - 17, g.top + y * g.rowH + g.rowH * 0.5f - 4, 7, Theme::muted, true);
                }
            }

            // bar / step lines
            for (int c = 0; c <= g.cols; ++c) {
                float x = g.left + c * g.colW;
                bool bar = (c % 4) == 0;
                rrect(t, { x, g.top, 1, g.h }, 0, bar ? sf::Color(255, 255, 255, 26) : sf::Color(255, 255, 255, 10));
            }
            if (cur >= 0 && cur < g.cols)
                rrect(t, { g.left + cur * g.colW, g.top, g.colW, g.h }, 0, sf::Color(255, 255, 255, 16));

            // notes
            for (int c = 0; c < g.cols; ++c) {
                bool on, accent, slide;
                int note;
                float vel;
                if (tr.type == ENGINE_ACID) {
                    auto& s = tr.acidSeq->steps[c];
                    on = s.on; note = s.note; accent = s.accent; slide = s.slide; vel = s.velocity;
                } else {
                    auto& s = tr.fm4Seq->steps[c];
                    on = s.on; note = s.note; accent = s.accent; slide = s.slide; vel = s.velocity;
                }
                if (!on) continue;

                int y = g.hi - std::clamp(note, g.lo, g.hi);
                float bh = std::max(5.0f, (g.rowH - 2) * (0.45f + 0.5f * std::clamp(vel, 0.0f, 1.0f)));
                float len = 0.85f;
                if (tr.type == ENGINE_ACID) len = tr.acidSeq->steps[c].length;
                else len = tr.fm4Seq->steps[c].length;
                float nw = std::max(5.0f, std::min(g.colW * len - 2.0f, (g.cols - c) * g.colW - 3.0f));
                sf::FloatRect r { g.left + c * g.colW + 2, g.top + y * g.rowH + (g.rowH - bh) - 1, nw, bh };
                sf::Color ccol = accent ? sf::Color(255, 176, 64) : (slide ? sf::Color(56, 212, 255) : tr.color);
                bool hot = r.contains(mouse);
                rrect(t, r, 3.0f, ccol, hot ? Theme::text : (accent ? sf::Color(255, 220, 150, 90) : sf::Color(0, 0, 0, 50)));
                if (slide) {
                    sf::ConvexShape tri(3);
                    tri.setPoint(0, { r.left + r.width, r.top + r.height * 0.5f });
                    tri.setPoint(1, { r.left + r.width - 6, r.top + 1 });
                    tri.setPoint(2, { r.left + r.width - 6, r.top + r.height - 1 });
                    tri.setFillColor(sf::Color(56, 212, 255));
                    t.draw(tri);
                }
                if (accent)
                    rrect(t, { r.left + r.width - 5, r.top + 2, 3, 3 }, 1.5f, sf::Color(90, 40, 10));
            }

            // step numbers
            for (int c = 0; c < g.cols; c += 4) {
                char sn[16];
                snprintf(sn, sizeof(sn), "%d", c + 1);
                label(t, sn, g.left + c * g.colW + 2, g.top + g.h + 1, 6, Theme::faint, false);
            }
        }
    }

    int seqStepAt(Track& tr, SeqGeom& g, sf::Vector2f p)
    {
        if (p.x < g.left || p.x >= g.left + g.w || p.y < g.top || p.y >= g.top + g.h) return -1;
        int c = (int)((p.x - g.left) / g.colW);
        if (c < 0 || c >= g.cols) return -1;
        if (tr.type == ENGINE_KICK) {
            int row = (int)((p.y - g.top) / g.rowH);
            if (row < 0 || row > 3) return -1;
            return row * 16 + c;
        }
        return c;
    }

    void seqPressed(sf::Vector2f p, sf::Mouse::Button button, bool shift)
    {
        std::lock_guard<std::mutex> lock(audioMutex);
        Track& tr = *tracks[selectedTrack.load()];
        SeqGeom g = seqGeom(tr);

        // toolbar
        if (tr.type != ENGINE_KICK) {
            if (g.len16.contains(p)) {
                if (tr.type == ENGINE_ACID) tr.acidSeq->patternLength = 16;
                else tr.fm4Seq->patternLength = 16;
                return;
            }
            if (g.len32.contains(p)) {
                if (tr.type == ENGINE_ACID) tr.acidSeq->patternLength = 32;
                else tr.fm4Seq->patternLength = 32;
                return;
            }
        }
        if (g.clear.contains(p)) {
            if (tr.type == ENGINE_KICK) tr.kickSeq->clear();
            else if (tr.type == ENGINE_ACID) tr.acidSeq->clear();
            else tr.fm4Seq->clear();
            return;
        }
        if (g.trMinus.contains(p) || g.trPlus.contains(p) || g.trBox.contains(p)) {
            int& tp = seqTranspose(tr);
            int d = g.trMinus.contains(p) ? -1 : 1;
            if (g.trBox.contains(p)) d = 0;
            if (d != 0) tp = std::clamp(tp + d * (shift ? 12 : 1), -seqTransposeMax(tr), seqTransposeMax(tr));
            return;
        }

        if (g.editStrip && seqSelected >= 0 && seqSelected < g.cols) {
            float d = shift ? -1.0f : 1.0f;
            if (tr.type != ENGINE_KICK) {
                bool acid = tr.type == ENGINE_ACID;
                auto setNote = [&](int v) {
                    if (acid) tr.acidSeq->steps[seqSelected].note = (uint8_t)std::clamp(v, 0, 127);
                    else tr.fm4Seq->steps[seqSelected].note = (uint8_t)std::clamp(v, 0, 127);
                };
                auto setLen = [&](float v) {
                    if (acid) tr.acidSeq->steps[seqSelected].length = std::clamp(v, 0.1f, 8.0f);
                    else tr.fm4Seq->steps[seqSelected].length = std::clamp(v, 0.1f, 8.0f);
                };
                auto setVel = [&](float v) {
                    if (acid) tr.acidSeq->steps[seqSelected].velocity = std::clamp(v, 0.0f, 1.0f);
                    else tr.fm4Seq->steps[seqSelected].velocity = std::clamp(v, 0.0f, 1.0f);
                };
                if (g.editNote.contains(p)) { setNote((int)(acid ? tr.acidSeq->steps[seqSelected].note : tr.fm4Seq->steps[seqSelected].note) + (int)d); return; }
                if (g.editLen.contains(p)) { setLen((acid ? tr.acidSeq->steps[seqSelected].length : tr.fm4Seq->steps[seqSelected].length) + d * 0.25f); return; }
                if (g.editVel.contains(p)) { setVel((acid ? tr.acidSeq->steps[seqSelected].velocity : tr.fm4Seq->steps[seqSelected].velocity) + d * 0.1f); return; }
                if (g.editSlide.contains(p)) {
                    if (acid) tr.acidSeq->steps[seqSelected].slide = !tr.acidSeq->steps[seqSelected].slide;
                    else tr.fm4Seq->steps[seqSelected].slide = !tr.fm4Seq->steps[seqSelected].slide;
                    return;
                }
                if (g.editAccent.contains(p)) {
                    if (acid) tr.acidSeq->steps[seqSelected].accent = !tr.acidSeq->steps[seqSelected].accent;
                    else tr.fm4Seq->steps[seqSelected].accent = !tr.fm4Seq->steps[seqSelected].accent;
                    return;
                }
            } else if (g.editNote.contains(p)) {
                auto& s = tr.kickSeq->steps[seqSelected];
                s.note = (int8_t)std::clamp((int)s.note + (int)d, -24, 24);
                return;
            }
        }

        int c = seqStepAt(tr, g, p);
        if (c < 0) return;
        seqSelected = c;

        if (tr.type == ENGINE_KICK) {
            auto& s = tr.kickSeq->steps[c];
            if (button == sf::Mouse::Right) { s.on = false; s.note = 0; seqPainting = false; }
            else { s.on = !s.on; seqPaintOn = s.on; seqPainting = true; }
            return;
        }

        int note = std::clamp(g.hi - (int)((p.y - g.top) / g.rowH), 0, 127);
        if (tr.type == ENGINE_ACID) {
            auto& s = tr.acidSeq->steps[c];
            if (button == sf::Mouse::Right) { s.on = false; seqPainting = false; return; }
            if (s.on && (int)s.note == note) {
                if (shift) { s.accent = !s.accent; seqPainting = false; }
                else { s.on = false; seqPaintOn = false; seqPainting = true; }
            } else {
                s.on = true;
                s.note = (uint8_t)note;
                s.velocity = 0.85f;
                s.length = 0.85f;
                s.accent = shift;
                s.slide = false;
                seqPaintOn = true;
                seqPainting = true;
            }
        } else {
            auto& s = tr.fm4Seq->steps[c];
            if (button == sf::Mouse::Right) { s.on = false; seqPainting = false; return; }
            if (s.on && (int)s.note == note) {
                if (shift) { s.accent = !s.accent; seqPainting = false; }
                else { s.on = false; seqPaintOn = false; seqPainting = true; }
            } else {
                s.on = true;
                s.note = (uint8_t)note;
                s.velocity = 0.85f;
                s.length = 0.85f;
                s.accent = shift;
                s.slide = false;
                seqPaintOn = true;
                seqPainting = true;
            }
        }
    }

    void seqPaint(sf::Vector2f p)
    {
        std::lock_guard<std::mutex> lock(audioMutex);
        Track& tr = *tracks[selectedTrack.load()];
        SeqGeom g = seqGeom(tr);
        int c = seqStepAt(tr, g, p);
        if (c < 0) return;
        if (tr.type == ENGINE_KICK) {
            tr.kickSeq->steps[c].on = seqPaintOn;
            return;
        }
        int note = std::clamp(g.hi - (int)((p.y - g.top) / g.rowH), 0, 127);
        auto paint = [&](auto& s) {
            if (seqPaintOn) {
                s.on = true;
                s.note = (uint8_t)note;
                if (s.velocity < 0.1f) s.velocity = 0.85f;
            } else {
                s.on = false;
            }
        };
        if (tr.type == ENGINE_ACID) paint(tr.acidSeq->steps[c]);
        else paint(tr.fm4Seq->steps[c]);
    }

    void seqWheel(sf::Vector2f p, float delta, bool shift)
    {
        std::lock_guard<std::mutex> lock(audioMutex);
        Track& tr = *tracks[selectedTrack.load()];
        SeqGeom g = seqGeom(tr);
        if (g.trMinus.contains(p) || g.trBox.contains(p) || g.trPlus.contains(p)) {
            int& tp = seqTranspose(tr);
            tp = std::clamp(tp + (delta > 0 ? 1 : -1) * (shift ? 12 : 1), -seqTransposeMax(tr), seqTransposeMax(tr));
            return;
        }
        int c = seqStepAt(tr, g, p);
        if (c < 0) return;
        float dir = delta > 0 ? 1.0f : -1.0f;

        if (tr.type == ENGINE_KICK) {
            int d = (int)dir * (shift ? 12 : 1);
            tr.kickSeq->steps[c].note = (int8_t)std::clamp((int)tr.kickSeq->steps[c].note + d, -24, 24);
            return;
        }
        if (tr.type == ENGINE_ACID) {
            auto& s = tr.acidSeq->steps[c];
            if (shift) s.velocity = std::clamp(s.velocity + dir * 0.05f, 0.0f, 1.0f);
            else s.note = (uint8_t)std::clamp((int)s.note + (int)dir, 0, 127);
        } else {
            auto& s = tr.fm4Seq->steps[c];
            if (shift) s.velocity = std::clamp(s.velocity + dir * 0.05f, 0.0f, 1.0f);
            else s.note = (uint8_t)std::clamp((int)s.note + (int)dir, 0, 127);
        }
    }
};

// ---------------------------------------------------------------- persistence

StudioUi ui;

// window geometry, remembered across runs
static int winX = -1, winY = -1, winW = 0, winH = 0;

static std::string serializeProject()
{
    std::lock_guard<std::mutex> lock(audioMutex);
    std::ostringstream o;
    o << std::fixed << std::setprecision(4);
    o << "version 2\n";
    o << "bpm " << transportBpm.load() << "\n";
    o << "selected " << selectedTrack.load() << "\n";
    o << "masterVolume " << masterVolume.load() << "\n";
    o << "fx " << fxDelayTime.load() << " " << fxDelayFeedback.load() << " " << fxDelayLevel.load()
      << " " << fxReverbSize.load() << " " << fxReverbLevel.load() << "\n";
    o << "window " << winX << " " << winY << " " << winW << " " << winH << "\n";
    o << "seqLayout " << (seqOnRight ? 1 : 0) << " " << seqSplit << "\n";

    for (int i = 0; i < TRACK_COUNT; ++i) {
        Track& t = *tracks[i];
        o << "track " << i << " " << t.type << " " << t.volume << " " << t.pan << " "
          << (t.mute ? 1 : 0) << " " << (t.solo ? 1 : 0) << " " << t.delaySend << " " << t.reverbSend << "\n";
        auto dump = [&](const char* tag, auto& panel) {
            for (int p = 0; p < 32; ++p)
                o << "param " << i << " " << tag << " " << panel.param(p).key << " " << panel.param(p).value << "\n";
        };
        dump("kick", *t.kickPanel);
        dump("acid", *t.acidPanel);
        dump("fm4", *t.fm4Panel);

        o << "seqKick " << i << " " << t.kickSeq->transpose;
        for (auto& s : t.kickSeq->steps)
            o << " " << (s.on ? 1 : 0) << ":" << (int)s.note;
        o << "\n";

        o << "seqAcid " << i << " " << t.acidSeq->patternLength << " " << t.acidSeq->transpose;
        for (auto& s : t.acidSeq->steps)
            o << " " << (s.on ? 1 : 0) << ":" << (int)s.note << ":" << s.velocity << ":"
              << (s.accent ? 1 : 0) << ":" << (s.slide ? 1 : 0) << ":" << s.length;
        o << "\n";

        o << "seqFm4 " << i << " " << t.fm4Seq->patternLength << " " << t.fm4Seq->transpose;
        for (auto& s : t.fm4Seq->steps)
            o << " " << (s.on ? 1 : 0) << ":" << (int)s.note << ":" << s.velocity << ":"
              << (s.accent ? 1 : 0) << ":" << (s.slide ? 1 : 0) << ":" << s.length;
        o << "\n";
    }
    return o.str();
}

static void loadProject(const std::string& file)
{
    std::ifstream f(file);
    if (!f) return;
    std::string line;
    while (std::getline(f, line)) {
        std::istringstream l(line);
        std::string key;
        l >> key;
        if (key == "bpm") { float v; if (l >> v) transportBpm = std::clamp(v, 40.0f, 300.0f); }
        else if (key == "selected") { int v; if (l >> v) selectedTrack = std::clamp(v, 0, TRACK_COUNT - 1); }
        else if (key == "masterVolume" || key == "masterVol") { float v; if (l >> v) masterVolume = std::clamp(v, 0.0f, 1.0f); }
        // version 1 mixer lines
        else if (key == "instVol") { int i; float v; if ((l >> i >> v) && i >= 0 && i < TRACK_COUNT) tracks[i]->volume = std::clamp(v, 0.0f, 1.0f); }
        else if (key == "instMute") { int i, m; if ((l >> i >> m) && i >= 0 && i < TRACK_COUNT) tracks[i]->mute = m != 0; }
        else if (key == "fx") {
            float a, b, c, d, e;
            if (l >> a >> b >> c >> d >> e) {
                fxDelayTime = std::clamp(a, 0.0f, 1.0f);
                fxDelayFeedback = std::clamp(b, 0.0f, 1.0f);
                fxDelayLevel = std::clamp(c, 0.0f, 1.0f);
                fxReverbSize = std::clamp(d, 0.0f, 1.0f);
                fxReverbLevel = std::clamp(e, 0.0f, 1.0f);
            }
        } else if (key == "window") {
            l >> winX >> winY >> winW >> winH;
        } else if (key == "seqLayout") {
            int r = 1;
            float s = 0.0f;
            if (l >> r >> s) { seqOnRight = r != 0; seqSplit = std::clamp(s, 0.0f, 1.0f); }
        } else if (key == "track") {
            int i, type, mute, solo;
            float vol, pan, dsend = 0.18f, rsend = 0.12f;
            if (l >> i >> type >> vol >> pan >> mute >> solo && i >= 0 && i < TRACK_COUNT) {
                if (!(l >> dsend >> rsend)) { dsend = 0.18f; rsend = 0.12f; }
                Track& t = *tracks[i];
                t.type = std::clamp(type, 0, ENGINE_COUNT - 1);
                t.volume = std::clamp(vol, 0.0f, 1.0f);
                t.pan = std::clamp(pan, -1.0f, 1.0f);
                t.mute = mute != 0;
                t.solo = solo != 0;
                t.delaySend = std::clamp(dsend, 0.0f, 1.0f);
                t.reverbSend = std::clamp(rsend, 0.0f, 1.0f);
            }
        } else if (key == "param") {
            // version 2: param <track> <inst> <name> <value>
            // version 1: param <inst> <name> <value>  (one fixed instrument each)
            std::string first;
            if (!(l >> first)) continue;
            int i = -1;
            std::string inst = first;
            if (!first.empty() && std::isdigit((unsigned char)first[0])) {
                i = atoi(first.c_str());
                if (!(l >> inst) || i < 0 || i >= TRACK_COUNT) continue;
            } else {
                int want = inst == "kick" ? ENGINE_KICK : inst == "acid" ? ENGINE_ACID : ENGINE_FM4;
                for (int k = 0; k < TRACK_COUNT; ++k)
                    if (tracks[k]->type == want) { i = k; break; }
                if (i < 0) continue;
            }
            std::string name;
            float value;
            if (!(l >> name >> value)) continue;
            Track& t = *tracks[i];
            auto apply = [&](auto& panel) {
                for (int p = 0; p < 32; ++p)
                    if (panel.param(p).key && name == panel.param(p).key) panel.param(p).set(value);
            };
            if (inst == "kick") apply(*t.kickPanel);
            else if (inst == "acid") apply(*t.acidPanel);
            else if (inst == "fm4") apply(*t.fm4Panel);
        } else if (key == "stepsKick") {
            // version 1 flat kick notes (-100 = off)
            int i;
            if (!(l >> i) || i < 0 || i >= TRACK_COUNT) continue;
            for (int s = 0; s < Sequencer::STEPS; ++s) {
                int v = -100;
                if (!(l >> v)) break;
                tracks[i]->kickSeq->steps[s].on = v != -100;
                tracks[i]->kickSeq->steps[s].note = (int8_t)std::clamp(v == -100 ? 0 : v, -24, 24);
            }
        } else if (key == "stepsAcid" || key == "stepsFm4") {
            // version 1 flat acid / fm4 notes (-100 = off)
            int i;
            if (!(l >> i) || i < 0 || i >= TRACK_COUNT) continue;
            bool acid = key == "stepsAcid";
            for (int s = 0; s < AcidSequencer::STEPS; ++s) {
                int v = -100;
                if (!(l >> v)) break;
                if (acid) {
                    auto& st = tracks[i]->acidSeq->steps[s];
                    st.on = v != -100;
                    st.note = (uint8_t)std::clamp(v == -100 ? 36 : v, 0, 127);
                    st.velocity = 0.8f;
                    st.accent = false;
                    st.slide = false;
                    st.length = 1.0f;
                } else {
                    auto& st = tracks[i]->fm4Seq->steps[s];
                    st.on = v != -100;
                    st.note = (uint8_t)std::clamp(v == -100 ? 36 : v, 0, 127);
                    st.velocity = 0.8f;
                    st.accent = false;
                    st.slide = false;
                    st.length = 0.85f;
                }
            }
        } else if (key == "seqKick") {
            int i;
            if (!(l >> i) || i < 0 || i >= TRACK_COUNT) continue;
            Track& t = *tracks[i];
            auto parseStep = [&](const std::string& tok, int s) {
                int on = 0, note = 0;
                if (sscanf(tok.c_str(), "%d:%d", &on, &note) >= 1) {
                    t.kickSeq->steps[s].on = on != 0;
                    t.kickSeq->steps[s].note = (int8_t)std::clamp(note, -24, 24);
                }
            };
            std::string tok;
            int s = 0;
            if (l >> tok) {
                // version 1 wrote steps first, version 2 starts with transpose
                if (tok.find(':') != std::string::npos) {
                    parseStep(tok, 0);
                    s = 1;
                } else {
                    t.kickSeq->transpose = std::clamp(atoi(tok.c_str()), -24, 24);
                }
            }
            for (; s < Sequencer::STEPS; ++s) {
                if (!(l >> tok)) break;
                parseStep(tok, s);
            }
        } else if (key == "seqAcid") {
            int i, len = 16, trans = 0;
            if (!(l >> i >> len >> trans) || i < 0 || i >= TRACK_COUNT) continue;
            Track& t = *tracks[i];
            t.acidSeq->patternLength = std::clamp(len, 1, AcidSequencer::STEPS);
            t.acidSeq->transpose = std::clamp(trans, -48, 48);
            for (int s = 0; s < AcidSequencer::STEPS; ++s) {
                std::string tok;
                if (!(l >> tok)) break;
                int on = 0, note = 36, accent = 0, slide = 0;
                float vel = 0.8f, length = 1.0f;
                if (sscanf(tok.c_str(), "%d:%d:%f:%d:%d:%f", &on, &note, &vel, &accent, &slide, &length) >= 2) {
                    auto& st = t.acidSeq->steps[s];
                    st.on = on != 0;
                    st.note = (uint8_t)std::clamp(note, 0, 127);
                    st.velocity = std::clamp(vel, 0.0f, 1.0f);
                    st.accent = accent != 0;
                    st.slide = slide != 0;
                    st.length = std::clamp(length, 0.1f, 8.0f);
                }
            }
        } else if (key == "seqFm4") {
            int i, len = 16, trans = 0;
            if (!(l >> i >> len >> trans) || i < 0 || i >= TRACK_COUNT) continue;
            Track& t = *tracks[i];
            t.fm4Seq->patternLength = std::clamp(len, 1, Fm4Sequencer::STEPS);
            t.fm4Seq->transpose = std::clamp(trans, -48, 48);
            for (int s = 0; s < Fm4Sequencer::STEPS; ++s) {
                std::string tok;
                if (!(l >> tok)) break;
                int on = 0, note = 36, accent = 0, slide = 0;
                float vel = 0.8f, length = 0.85f;
                if (sscanf(tok.c_str(), "%d:%d:%f:%d:%d:%f", &on, &note, &vel, &accent, &slide, &length) >= 2) {
                    auto& st = t.fm4Seq->steps[s];
                    st.on = on != 0;
                    st.note = (uint8_t)std::clamp(note, 0, 127);
                    st.velocity = std::clamp(vel, 0.0f, 1.0f);
                    st.accent = accent != 0;
                    st.slide = slide != 0;
                    st.length = std::clamp(length, 0.1f, 8.0f);
                }
            }
        }
    }
}

// ---------------------------------------------------------------- main

int main()
{
    const sf::Color palette[TRACK_COUNT] = {
        { 56, 212, 255 }, { 255, 176, 64 }, { 172, 128, 255 }, { 75, 210, 130 },
        { 255, 86, 98 }, { 255, 200, 60 }, { 120, 180, 255 }, { 240, 130, 220 }
    };
    for (int i = 0; i < TRACK_COUNT; ++i) {
        tracks.push_back(std::make_unique<Track>(i, palette[i]));
        tracks.back()->type = i % ENGINE_COUNT;
    }

    std::string stateFile = Persist::path();
    const char* shot = std::getenv("ZIC_SCREENSHOT");
    bool screenshot = shot != nullptr;
    bool demo = screenshot && std::getenv("ZIC_DEMO");
    if (!screenshot) loadProject(stateFile);
    if (const char* tr = std::getenv("ZIC_TRACK"))
        selectedTrack = std::clamp(atoi(tr), 0, TRACK_COUNT - 1);

    // ZIC_SILENT renders the full UI and sequencers without an audio device,
    // used by the screenshot verification target and for headless checks.
    bool silent = std::getenv("ZIC_SILENT") != nullptr;
    snd_pcm_t* pcm = silent ? nullptr : audioInit();
    if (!silent && !pcm) {
        std::cerr << "No audio device (set ZIC_SILENT=1 to run without sound)\n";
        return 1;
    }

    std::thread audioThread(audioWorker, pcm);
    sched_param sch {};
    sch.sched_priority = 35;
    pthread_setschedparam(audioThread.native_handle(), SCHED_FIFO, &sch);

    auto desktop = sf::VideoMode::getDesktopMode();
    bool forceNarrow = std::getenv("ZIC_NARROW") != nullptr;
    bool wide = desktop.width / (float)desktop.height >= 1.4f;
    if (!screenshot && winW > 0 && winH > 0) wide = winW / (float)winH >= 1.4f;
    if (forceNarrow) wide = false;

    unsigned winWidth, winHeight;
    if (screenshot) {
        winWidth = forceNarrow ? 1320 : 1920;
        winHeight = forceNarrow ? 1650 : 1080;
    } else if (winW >= 640 && winH >= 480) {
        winWidth = std::min((unsigned)winW, (unsigned)desktop.width);
        winHeight = std::min((unsigned)winH, (unsigned)desktop.height);
    } else {
        winWidth = std::min((unsigned)desktop.width, 1920u);
        winHeight = std::min((unsigned)desktop.height - 60u, 1200u);
    }
    sf::ContextSettings ctx;
    ctx.antialiasingLevel = 0;
    unsigned long style = screenshot ? sf::Style::None : sf::Style::Default;
    sf::RenderWindow window(sf::VideoMode(winWidth, winHeight), "zicStudio", style, ctx);
    window.setFramerateLimit(30);
    window.setKeyRepeatEnabled(false);
    if (!screenshot && winX != -1 && winY != -1)
        window.setPosition({ winX, winY });
    else
        window.setPosition({ 60, 40 });

    for (auto& t : tracks) t->createUis();

    auto fitView = [&](unsigned w, unsigned h) {
        // 1:1 pixels: the shell is laid out in real window pixels so text stays
        // crisp at any size instead of being resampled by the view.
        ui.relayout((float)w, (float)h);
        sf::View view(sf::FloatRect(0, 0, (float)w, (float)h));
        view.setViewport({ 0, 0, 1, 1 });
        ui.baseView = view;
        ui.baseViewport = { 0, 0, 1, 1 };
        window.setView(view);
    };
    ui.layout.seqRight = seqOnRight;
    ui.layout.split = seqSplit;
    fitView(window.getSize().x, window.getSize().y);
    auto logical = [&](int x, int y) { return window.mapPixelToCoords({ x, y }); };

    if (demo) {
        for (int i = 0; i < TRACK_COUNT; ++i) tracks[i]->type = i % ENGINE_COUNT;
        if (!std::getenv("ZIC_TRACK")) selectedTrack = 0;
        transportPlaying = true;
    }
    if (std::getenv("ZIC_PLAY")) transportPlaying = true;

    int autosave = 0;
    int frame = 0;
    bool shotDone = false;
    while (window.isOpen()) {
        // full rate while the transport runs, relaxed when idle
        window.setFramerateLimit(transportPlaying.load() ? 24 : 10);
        sf::Event e;
        while (window.pollEvent(e)) {
            bool shift = sf::Keyboard::isKeyPressed(sf::Keyboard::LShift) || sf::Keyboard::isKeyPressed(sf::Keyboard::RShift);
            switch (e.type) {
            case sf::Event::Closed:
                if (!screenshot || shotDone) window.close();
                break;
            case sf::Event::Resized:
                fitView(e.size.width, e.size.height);
                break;
            case sf::Event::MouseButtonPressed:
                if (!screenshot) ui.mousePressed(logical(e.mouseButton.x, e.mouseButton.y), e.mouseButton.button, shift);
                break;
            case sf::Event::MouseButtonReleased:
                if (!screenshot) ui.mouseReleased();
                break;
            case sf::Event::MouseMoved:
                if (!screenshot) ui.mouseMoved(logical(e.mouseMove.x, e.mouseMove.y), shift);
                break;
            case sf::Event::MouseWheelScrolled:
                if (!screenshot) ui.wheel(logical(e.mouseWheelScroll.x, e.mouseWheelScroll.y), e.mouseWheelScroll.delta, shift);
                break;
            case sf::Event::KeyPressed:
                if (e.key.code == sf::Keyboard::Escape) window.close();
                else if (!screenshot) ui.key(e.key.code);
                break;
            default:
                break;
            }
        }

        if (!screenshot && ++autosave >= 60) {
            autosave = 0;
            auto wp = window.getPosition();
            winX = wp.x;
            winY = wp.y;
            winW = (int)window.getSize().x;
            winH = (int)window.getSize().y;
            static std::string last;
            std::string now = serializeProject();
            if (now != last && Persist::save(stateFile, now)) last = now;
        }

        // fps + cpu usage (process, smoothed)
        {
            static auto fpsWall = std::chrono::steady_clock::now();
            static int fpsFrames = 0;
            fpsFrames++;
            auto fpsNow = std::chrono::steady_clock::now();
            double fpsDt = std::chrono::duration<double>(fpsNow - fpsWall).count();
            if (fpsDt > 0.5) {
                uiFps = (float)(fpsFrames / fpsDt);
                fpsFrames = 0;
                fpsWall = fpsNow;
            }
        }
        {
            static auto lastWall = std::chrono::steady_clock::now();
            static double lastCpu = 0.0;
            static bool init = false;
            rusage ru;
            getrusage(RUSAGE_SELF, &ru);
            double cpu = ru.ru_utime.tv_sec + ru.ru_utime.tv_usec / 1e6 + ru.ru_stime.tv_sec + ru.ru_stime.tv_usec / 1e6;
            auto now = std::chrono::steady_clock::now();
            if (init) {
                double dw = std::chrono::duration<double>(now - lastWall).count();
                if (dw > 0.05) {
                    float pct = (float)((cpu - lastCpu) / dw * 100.0);
                    float prev = cpuUsage.load();
                    cpuUsage = prev * 0.7f + pct * 0.3f;
                }
            } else {
                init = true;
            }
            lastWall = now;
            lastCpu = cpu;
        }

        auto mp = sf::Mouse::getPosition(window);
        sf::Vector2f uiMouse = screenshot ? sf::Vector2f(-10000.0f, -10000.0f) : logical(mp.x, mp.y);
        ui.draw(window, uiMouse);
        window.display();

        if (screenshot) {
            if (++frame == (demo ? 30 : 5)) {
                sf::Texture tex;
                tex.create(window.getSize().x, window.getSize().y);
                tex.update(window);
                if (!tex.copyToImage().saveToFile(shot)) std::cerr << "could not save " << shot << "\n";
                shotDone = true;
                window.close();
            }
        }
    }

    keep_running = false;
    audioThread.join();
    if (pcm) snd_pcm_close(pcm);
    if (!screenshot) {
        auto wp = window.getPosition();
        winX = wp.x;
        winY = wp.y;
        winW = (int)window.getSize().x;
        winH = (int)window.getSize().y;
        Persist::save(stateFile, serializeProject());
    }
    return 0;
}
