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
static constexpr size_t NUM_FRAMES = 256;
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
std::atomic<float> masterMeter(0.0f);
std::atomic<float> fxDelayTime(0.28f);
std::atomic<float> fxDelayFeedback(0.35f);
std::atomic<float> fxDelayLevel(0.45f);
std::atomic<float> fxReverbSize(0.55f);
std::atomic<float> fxReverbLevel(0.35f);

enum EngineType { ENGINE_KICK = 0, ENGINE_ACID = 1, ENGINE_FM4 = 2, ENGINE_COUNT = 3 };

static const char* engineName(int t) { return t == 0 ? "Kick" : (t == 1 ? "Acid" : "Fm4"); }
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
    snd_pcm_set_params(pcm, SND_PCM_FORMAT_S16_LE, SND_PCM_ACCESS_RW_INTERLEAVED, 2, SAMPLE_RATE, 1, 20000);
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
            std::this_thread::sleep_for(std::chrono::microseconds(5800));
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
    float W = 1840.0f, H = 1010.0f;
    sf::FloatRect top, panel, header, mixer, fx, seq;
};

static Layout makeLayout(bool wide)
{
    Layout l;
    l.wide = wide;
    if (wide) {
        // panel left, channel strip / master FX / sequencer stacked on the right
        l.W = 1840.0f;
        l.H = 1010.0f;
        l.top = { 0, 0, l.W, 52 };
        l.panel = { 0, 52, 1320, 958 };
        l.header = { 1320, 52, 520, 52 };
        l.mixer = { 1320, 104, 520, 356 };
        l.fx = { 1320, 460, 520, 152 };
        l.seq = { 1320, 612, 520, 398 };
    } else {
        // panel first, then mixer / master FX / sequencer underneath
        l.W = 1320.0f;
        l.H = 1650.0f;
        l.top = { 0, 0, l.W, 52 };
        l.header = { 0, 52, 1320, 48 };
        l.panel = { 0, 100, 1320, 958 };
        l.mixer = { 0, 1058, 1320, 196 };
        l.fx = { 0, 1254, 1320, 124 };
        l.seq = { 0, 1378, 1320, 272 };
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

        // 1. the real instrument panel. It clears the target, so draw it first.
        drawInstrument(t, tr, mouse);

        // 2. our shell on top
        t.setView(baseView);
        drawTopBar(t, mouse);
        drawHeader(t, tr, mouse);
        drawMixer(t, mouse);
        drawFx(t, mouse);
        drawSequencer(t, tr, mouse);
        drawOutputScope(t, tr);
    }

    void mousePressed(sf::Vector2f p, sf::Mouse::Button button, bool shift)
    {
        Layout& l = layout;
        int sel = selectedTrack.load();
        panelDragging = false;

        if (inRect(l.top, p)) { topPressed(p, button, shift); return; }
        if (inRect(l.header, p)) { headerPressed(p, button); return; }
        if (inRect(l.mixer, p)) { mixerPressed(p, button, shift); return; }
        if (inRect(l.fx, p)) { fxPressed(p, button, shift); return; }
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
    }

    void mouseMoved(sf::Vector2f p, bool shift)
    {
        Layout& l = layout;
        int sel = selectedTrack.load();
        Track& tr = *tracks[sel];

        if (dragFx != FxNone) { fxDrag(p); return; }
        if (dragKind != DragNone) { mixerDrag(p, shift); return; }

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

        if (inRect(l.panel, p)) {
            sf::Vector2f q;
            if (toPanel(tr, p, q)) panelWheel(tr, q, delta, shift);
        } else if (inRect(l.seq, p)) {
            seqWheel(p, delta, shift);
        } else if (inRect(l.mixer, p)) {
            mixerWheel(p, delta, shift);
        } else if (inRect(l.fx, p)) {
            fxWheel(p, delta, shift);
        } else if (inRect(l.top, p)) {
            topWheel(p, delta, shift);
        }
    }

    void key(sf::Keyboard::Key k)
    {
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

    Layout layout { makeLayout(true) };
    sf::View baseView;
    sf::FloatRect baseViewport { 0, 0, 1, 1 };

    void setWide(bool wide)
    {
        if (layout.wide != wide) layout = makeLayout(wide);
    }

private:
    enum DragKind { DragNone, DragVolume, DragPan, DragDelaySend, DragReverbSend };
    DragKind dragKind = DragNone;
    int dragTrack = -1;
    float dragStart = 0.0f;
    bool panelDragging = false;

    enum FxKnob { FxNone, FxDelayTime, FxDelayFeedback, FxDelayLevel, FxReverbSize, FxReverbLevel };
    FxKnob dragFx = FxNone;
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
        b.scale = std::min(layout.panel.width / b.cw, (layout.panel.height - 12.0f) / b.ch);
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
    void drawPanelView(sf::RenderTarget& t, UI& panelUi, sf::Vector2f mouse)
    {
        PanelBox<UI> b = panelBox<UI>();
        sf::Vector2f q;
        bool inside = mapPanelCoords<UI>(mouse, q);

        sf::View v(sf::FloatRect(0, UI::PANEL_TOP, b.cw, b.ch));
        sf::FloatRect vp {
            baseViewport.left + (b.ox / layout.W) * baseViewport.width,
            baseViewport.top + (b.oy / layout.H) * baseViewport.height,
            (b.vw / layout.W) * baseViewport.width,
            (b.vh / layout.H) * baseViewport.height
        };
        v.setViewport(vp);
        t.setView(v);
        panelUi.draw(t, inside ? q : sf::Vector2f(-10000, -10000));
        t.setView(baseView);
    }

    void drawInstrument(sf::RenderTarget& t, Track& tr, sf::Vector2f mouse)
    {
        if (tr.type == ENGINE_KICK) drawPanelView(t, *tr.kickUi, mouse);
        else if (tr.type == ENGINE_ACID) drawPanelView(t, *tr.acidUi, mouse);
        else drawPanelView(t, *tr.fm4Ui, mouse);
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

    void drawTopBar(sf::RenderTarget& t, sf::Vector2f mouse)
    {
        rrect(t, layout.top, 0, sf::Color(14, 16, 21));
        sf::RectangleShape underline({ layout.W, 1 });
        underline.setPosition(0, layout.top.height - 1);
        underline.setFillColor(Theme::cardLine);
        t.draw(underline);

        label(t, "zic", 16, 9, 22, Theme::text, false, true, 0.4f);
        label(t, "Studio", 54, 9, 22, Theme::accent, false, true, 0.4f);
        label(t, "8 TRACKS / 3 ENGINES", 16, 34, 7, Theme::faint, false, false, 1.2f);

        for (int i = 0; i < TRACK_COUNT; ++i) {
            auto r = trackTab(i);
            bool sel = selectedTrack.load() == i;
            bool hot = r.contains(mouse);
            Track& tr = *tracks[i];
            sf::Color c = tr.color;
            rrect(t, r, 9.0f, sel ? c : (hot ? sf::Color(38, 42, 52) : Theme::card), sel ? c : Theme::cardLine);

            char nb[8];
            snprintf(nb, sizeof(nb), "%02d", i + 1);
            label(t, nb, r.left + 8, r.top + 4, 12, sel ? sf::Color(12, 12, 14) : c, false, true, 0.4f);
            label(t, engineShort(tr.type), r.left + 8, r.top + 21, 8, sel ? sf::Color(12, 14, 18) : Theme::muted, false, true, 1.2f);

            // activity / mute dot on the right edge
            if (tr.mute) {
                rrect(t, { r.left + r.width - 12, r.top + 8, 5, 5 }, 2.5f, sf::Color(230, 70, 80));
            } else {
                float lvl = std::clamp(tr.meter.load(), 0.0f, 1.0f);
                rrect(t, { r.left + r.width - 12, r.top + 8, 5, 5 }, 2.5f,
                    sel ? sf::Color(12, 14, 18) : (lvl > 0.02f ? c : Theme::cell));
            }
        }

        auto r = playBtn();
        bool playing = transportPlaying.load();
        rrect(t, r, 10.0f, playing ? sf::Color(70, 210, 130) : Theme::card, playing ? sf::Color(70, 210, 130) : Theme::cardLine);
        if (playing) {
            sf::RectangleShape stop({ 12, 12 });
            stop.setPosition(r.left + 21, r.top + 12);
            stop.setFillColor(sf::Color(12, 14, 16));
            t.draw(stop);
        } else {
            sf::ConvexShape pl(3);
            pl.setPoint(0, { r.left + 21, r.top + 10 });
            pl.setPoint(1, { r.left + 38, r.top + 18 });
            pl.setPoint(2, { r.left + 21, r.top + 26 });
            pl.setFillColor(Theme::text);
            t.draw(pl);
        }

        char buf[32];
        snprintf(buf, sizeof(buf), "%.0f", transportBpm.load());
        rrect(t, bpmBox(), 9.0f, Theme::card, Theme::cardLine);
        label(t, buf, bpmBox().left + bpmBox().width * 0.5f, 11, 17, Theme::text, true, true, 0.5f);
        label(t, "BPM", bpmBox().left + bpmBox().width * 0.5f, 31, 7, Theme::muted, true, false, 1.4f);
        rrect(t, bpmMinus(), 9.0f, Theme::card, Theme::cardLine);
        rrect(t, bpmPlus(), 9.0f, Theme::card, Theme::cardLine);
        label(t, "-", bpmMinus().left + 15, 12, 17, Theme::muted, true);
        label(t, "+", bpmPlus().left + 15, 12, 17, Theme::muted, true);

        // master volume
        auto mb = masterBox();
        rrect(t, mb, 9.0f, Theme::card, Theme::cardLine);
        label(t, "MASTER", mb.left + 9, mb.top + 4, 7, Theme::muted, false, false, 1.6f);
        sf::FloatRect bar { mb.left + 9, mb.top + 21, mb.width - 18, 7 };
        rrect(t, bar, 3, Theme::cell);
        float mv = masterVolume.load();
        rrect(t, { bar.left, bar.top, bar.width * mv, bar.height }, 3, Theme::accent);
        label(t, std::to_string((int)std::round(mv * 100)) + "%", mb.left + mb.width - 9, mb.top + 4, 7, Theme::text, false);

        // master meter
        auto me = meterBox();
        rrect(t, me, 9.0f, Theme::card, Theme::cardLine);
        label(t, "OUT", me.left + 8, me.top + 4, 7, Theme::muted, false, false, 1.6f);
        sf::FloatRect mbar { me.left + 8, me.top + 21, me.width - 16, 7 };
        rrect(t, mbar, 3, Theme::cell);
        float lvl = std::clamp(masterMeter.load() * 0.82f, 0.0f, 1.0f);
        if (lvl > 0.01f) {
            sf::Color mc = lvl > 0.92f ? sf::Color(255, 86, 98) : (lvl > 0.7f ? sf::Color(255, 200, 60) : Theme::accent);
            rrect(t, { mbar.left, mbar.top, mbar.width * lvl, mbar.height }, 3, mc);
        }

        if (layout.wide) {
            // transport position for the selected track, in the gap between
            // the transport and the master section
            Track& tr = *tracks[selectedTrack.load()];
            int cur = 0, total = Sequencer::STEPS;
            if (tr.type == ENGINE_KICK) cur = tr.kickSeq->current.load();
            else if (tr.type == ENGINE_ACID) { cur = tr.acidSeq->currentStep.load(); total = tr.acidSeq->patternLength; }
            else { cur = tr.fm4Seq->currentStep.load(); total = tr.fm4Seq->patternLength; }
            if (cur < 0) cur = 0;

            sf::FloatRect pb { transportBase() + 238.0f, 8.0f, 150.0f, 36.0f };
            rrect(t, pb, 9.0f, Theme::card, Theme::cardLine);
            label(t, "TRANSPORT", pb.left + 10, pb.top + 4, 7, Theme::muted, false, false, 1.4f);
            snprintf(buf, sizeof(buf), "STEP %02d / %02d", cur + 1, total);
            label(t, buf, pb.left + 10, pb.top + 17, 11, Theme::text, false, false, 0.6f);

            // one tick per step
            float tx = pb.left + 90.0f, ty = pb.top + 24.0f;
            float tw = pb.width - 102.0f;
            int ticks = std::min(total, 32);
            for (int i = 0; i < ticks; ++i) {
                float x = tx + tw * i / std::max(1, ticks - 1);
                bool now = (i == cur % std::max(1, ticks));
                rrect(t, { x - 1, ty - (now ? 7.0f : 4.0f), 2, now ? 7.0f : 4.0f }, 1,
                    now ? tr.color : sf::Color(255, 255, 255, 34));
            }
        }
    }

    void topPressed(sf::Vector2f p, sf::Mouse::Button button, bool shift)
    {
        for (int i = 0; i < TRACK_COUNT; ++i)
            if (trackTab(i).contains(p)) { selectedTrack = i; return; }

        if (playBtn().contains(p)) { transportPlaying = !transportPlaying.load(); return; }
        if (bpmMinus().contains(p)) { transportBpm = std::max(60.0f, transportBpm.load() - (shift ? 10.0f : 1.0f)); return; }
        if (bpmPlus().contains(p)) { transportBpm = std::min(300.0f, transportBpm.load() + (shift ? 10.0f : 1.0f)); return; }
        if (masterBox().contains(p)) {
            float tv = std::clamp((p.x - (masterBox().left + 9)) / (masterBox().width - 18), 0.0f, 1.0f);
            masterVolume = tv;
            return;
        }
        (void)button;
    }

    void topWheel(sf::Vector2f p, float delta, bool shift)
    {
        if (masterBox().contains(p)) {
            masterVolume = std::clamp(masterVolume.load() + (delta > 0 ? 0.03f : -0.03f) * (shift ? 5.0f : 1.0f), 0.0f, 1.0f);
        } else if (bpmBox().contains(p) || bpmMinus().contains(p) || bpmPlus().contains(p)) {
            transportBpm = std::clamp(transportBpm.load() + (delta > 0 ? 1.0f : -1.0f) * (shift ? 10.0f : 1.0f), 60.0f, 300.0f);
        }
    }

    // ---------------------------------------------------------------- header
    sf::FloatRect engineBtn(int i)
    {
        float x = layout.header.left + layout.header.width - 14.0f - 3 * 92.0f + i * 92.0f;
        return { x, layout.header.top + (layout.header.height - 34.0f) * 0.5f, 84, 34 };
    }

    void drawHeader(sf::RenderTarget& t, Track& tr, sf::Vector2f mouse)
    {
        rrect(t, layout.header, 0, sf::Color(14, 16, 21));
        label(t, "TRACK " + std::to_string(tr.index + 1), layout.header.left + 14, layout.header.top + 8, 15, tr.color, false, true, 1.4f);
        label(t, engineName(tr.type), layout.header.left + 14, layout.header.top + 27, 8, Theme::muted, false, false, 1.6f);
        for (int i = 0; i < ENGINE_COUNT; ++i) {
            auto r = engineBtn(i);
            bool sel = tr.type == i;
            bool hot = r.contains(mouse);
            rrect(t, r, 9.0f, sel ? tr.color : (hot ? sf::Color(38, 42, 52) : Theme::card), sel ? tr.color : Theme::cardLine);
            label(t, engineName(i), r.left + r.width * 0.5f, r.top + 9, 10, sel ? sf::Color(12, 12, 14) : Theme::text, true, true, 1.0f);
        }
    }

    void headerPressed(sf::Vector2f p, sf::Mouse::Button)
    {
        for (int i = 0; i < ENGINE_COUNT; ++i)
            if (engineBtn(i).contains(p)) {
                std::lock_guard<std::mutex> lock(audioMutex);
                tracks[selectedTrack.load()]->setType(i);
                return;
            }
    }

    // ---------------------------------------------------------------- mixer
    struct StripRects {
        sf::FloatRect body, meter, fader, pan, delay, reverb, mute, solo;
    };

    StripRects stripRects(int i)
    {
        float w = layout.mixer.width / TRACK_COUNT;
        float x = layout.mixer.left + i * w + 4;
        float y = layout.mixer.top + 6;
        float h = layout.mixer.height - 12;
        StripRects r;
        r.body = { x, y, w - 8, h };
        float msH = 18.0f;
        r.mute = { r.body.left + 5, r.body.top + h - msH - 2, (r.body.width - 14) * 0.5f, msH };
        r.solo = { r.mute.left + r.mute.width + 4, r.mute.top, r.mute.width, msH };
        r.reverb = { r.body.left + 19, r.mute.top - 16, r.body.width - 25, 9 };
        r.delay = { r.body.left + 19, r.reverb.top - 14, r.body.width - 25, 9 };
        r.pan = { r.body.left + 19, r.delay.top - 15, r.body.width - 25, 9 };
        float faderTop = r.body.top + 26;
        r.fader = { r.body.left + 10, faderTop, r.body.width - 16, r.pan.top - 7 - faderTop };
        r.meter = { r.body.left + 3, faderTop, 5, r.fader.height };
        return r;
    }

    void drawMixer(sf::RenderTarget& t, sf::Vector2f mouse)
    {
        rrect(t, layout.mixer, 0, sf::Color(14, 16, 21));
        for (int i = 0; i < TRACK_COUNT; ++i) {
            Track& tr = *tracks[i];
            StripRects r = stripRects(i);
            bool sel = selectedTrack.load() == i;
            bool hot = r.body.contains(mouse);
            rrect(t, r.body, 8.0f, sel ? sf::Color(24, 27, 34) : (hot ? sf::Color(22, 25, 31) : Theme::card), sel ? tr.color : Theme::cardLine);

            char nb[8];
            snprintf(nb, sizeof(nb), "%02d", i + 1);
            label(t, nb, r.body.left + r.body.width * 0.5f, r.body.top + 3, 10, tr.color, true, true, 0.4f);
            label(t, engineShort(tr.type), r.body.left + r.body.width * 0.5f, r.body.top + 14, 7, Theme::muted, true, true, 0.8f);

            // meter
            rrect(t, r.meter, 2.5f, Theme::cell);
            float lvl = std::clamp(tr.meter.load() * 0.82f, 0.0f, 1.0f);
            if (lvl > 0.01f) {
                float mh = r.meter.height * lvl;
                sf::Color mc = lvl > 0.9f ? sf::Color(255, 86, 98) : tr.color;
                rrect(t, { r.meter.left, r.meter.top + r.meter.height - mh, r.meter.width, mh }, 2.5f, mc);
            }

            // fader
            rrect(t, { r.fader.left + r.fader.width * 0.5f - 2, r.fader.top, 4, r.fader.height }, 2, Theme::cell);
            float fy = r.fader.top + (1.0f - tr.volume) * r.fader.height;
            rrect(t, { r.fader.left + 1, fy - 4, r.fader.width - 2, 9 }, 4, tr.color, Theme::cardLine);

            // pan
            rrect(t, r.pan, 3, Theme::cell);
            float px = r.pan.left + (tr.pan + 1.0f) * 0.5f * r.pan.width;
            rrect(t, { px - 1.5f, r.pan.top - 3, 3, r.pan.height + 6 }, 1.5f, Theme::text);
            label(t, "PAN", r.body.left + 3, r.pan.top - 1, 6, Theme::faint, false, false, 0.4f);

            // FX sends
            auto sendBar = [&](sf::FloatRect rr, float v, sf::Color c, const char* tag) {
                label(t, tag, r.body.left + 3, rr.top - 1, 6, Theme::muted, false, false, 0.4f);
                rrect(t, rr, 2, Theme::cell);
                if (v > 0.01f) rrect(t, { rr.left, rr.top, rr.width * std::clamp(v, 0.0f, 1.0f), rr.height }, 2, c);
            };
            sendBar(r.delay, tr.delaySend, Theme::accent, "DLY");
            sendBar(r.reverb, tr.reverbSend, sf::Color(172, 128, 255), "REV");

            // mute / solo
            rrect(t, r.mute, 4, tr.mute ? sf::Color(230, 70, 80) : Theme::cell, Theme::cardLine);
            rrect(t, r.solo, 4, tr.solo ? sf::Color(255, 200, 60) : Theme::cell, Theme::cardLine);
            label(t, "M", r.mute.left + r.mute.width * 0.5f, r.mute.top + 3, 8, tr.mute ? sf::Color(12, 12, 14) : Theme::muted, true, true);
            label(t, "S", r.solo.left + r.solo.width * 0.5f, r.solo.top + 3, 8, tr.solo ? sf::Color(12, 12, 14) : Theme::muted, true, true);
        }
    }

    void mixerPressed(sf::Vector2f p, sf::Mouse::Button, bool shift)
    {
        std::lock_guard<std::mutex> lock(audioMutex);
        for (int i = 0; i < TRACK_COUNT; ++i) {
            StripRects r = stripRects(i);
            if (!r.body.contains(p)) continue;
            Track& tr = *tracks[i];
            selectedTrack = i;
            if (r.mute.contains(p)) { tr.mute = !tr.mute; return; }
            if (r.solo.contains(p)) { tr.solo = !tr.solo; return; }
            if (r.pan.contains(p)) {
                dragKind = DragPan;
                dragTrack = i;
                dragStart = tr.pan;
                tr.pan = std::clamp((p.x - r.pan.left) / r.pan.width * 2.0f - 1.0f, -1.0f, 1.0f);
                return;
            }
            if (r.delay.contains(p)) {
                dragKind = DragDelaySend;
                dragTrack = i;
                tr.delaySend = std::clamp((p.x - r.delay.left) / r.delay.width, 0.0f, 1.0f);
                return;
            }
            if (r.reverb.contains(p)) {
                dragKind = DragReverbSend;
                dragTrack = i;
                tr.reverbSend = std::clamp((p.x - r.reverb.left) / r.reverb.width, 0.0f, 1.0f);
                return;
            }
            if (r.fader.contains(p)) {
                dragKind = DragVolume;
                dragTrack = i;
                dragStart = tr.volume;
                tr.volume = std::clamp(1.0f - (p.y - r.fader.top) / r.fader.height, 0.0f, 1.0f);
                return;
            }
            return;
        }
        (void)shift;
    }

    void mixerDrag(sf::Vector2f p, bool shift)
    {
        if (dragTrack < 0) return;
        std::lock_guard<std::mutex> lock(audioMutex);
        Track& tr = *tracks[dragTrack];
        StripRects r = stripRects(dragTrack);
        if (dragKind == DragVolume) {
            tr.volume = std::clamp(1.0f - (p.y - r.fader.top) / r.fader.height, 0.0f, 1.0f);
        } else if (dragKind == DragPan) {
            tr.pan = std::clamp((p.x - r.pan.left) / r.pan.width * 2.0f - 1.0f, -1.0f, 1.0f);
        } else if (dragKind == DragDelaySend) {
            tr.delaySend = std::clamp((p.x - r.delay.left) / r.delay.width, 0.0f, 1.0f);
        } else if (dragKind == DragReverbSend) {
            tr.reverbSend = std::clamp((p.x - r.reverb.left) / r.reverb.width, 0.0f, 1.0f);
        }
        (void)shift;
    }

    void mixerWheel(sf::Vector2f p, float delta, bool shift)
    {
        std::lock_guard<std::mutex> lock(audioMutex);
        for (int i = 0; i < TRACK_COUNT; ++i) {
            StripRects r = stripRects(i);
            if (!r.body.contains(p)) continue;
            Track& tr = *tracks[i];
            float step = (delta > 0 ? 1.0f : -1.0f) * (shift ? 0.1f : 0.02f);
            if (r.fader.contains(p)) tr.volume = std::clamp(tr.volume + step * 1.5f, 0.0f, 1.0f);
            else if (r.pan.contains(p)) tr.pan = std::clamp(tr.pan + step * 2.0f, -1.0f, 1.0f);
            else if (r.delay.contains(p)) tr.delaySend = std::clamp(tr.delaySend + step, 0.0f, 1.0f);
            else if (r.reverb.contains(p)) tr.reverbSend = std::clamp(tr.reverbSend + step, 0.0f, 1.0f);
            return;
        }
    }

    // ---------------------------------------------------------------- master FX
    static constexpr int FX_KNOBS = 5;

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

    struct FxGeom {
        sf::FloatRect knob[FX_KNOBS];
        float r = 22.0f;
    };

    FxGeom fxGeom()
    {
        FxGeom g;
        g.r = layout.wide ? 22.0f : 20.0f;
        float spacing = layout.wide ? 92.0f : 118.0f;
        float cy = layout.wide ? layout.fx.top + 76.0f : layout.fx.top + 54.0f;
        float cx0 = layout.fx.left + layout.fx.width * 0.5f - spacing * (FX_KNOBS - 1) * 0.5f;
        for (int i = 0; i < FX_KNOBS; ++i) {
            float cx = cx0 + i * spacing;
            g.knob[i] = { cx - g.r - 7, cy - g.r - 7, 2 * g.r + 14, 2 * g.r + 14 };
        }
        return g;
    }

    void arc(sf::RenderTarget& t, float cx, float cy, float r, float a0, float a1, float th, sf::Color c)
    {
        const int seg = 30;
        sf::VertexArray va(sf::TriangleStrip);
        for (int i = 0; i <= seg; ++i) {
            float a = (a0 + (a1 - a0) * i / seg) * 3.14159265f / 180.0f;
            float ux = std::cos(a), uy = std::sin(a);
            va.append(sf::Vertex({ cx + ux * (r - th), cy + uy * (r - th) }, c));
            va.append(sf::Vertex({ cx + ux * r, cy + uy * r }, c));
        }
        t.draw(va);
    }

    void drawFxKnob(sf::RenderTarget& t, int i, sf::Vector2f mouse)
    {
        FxGeom g = fxGeom();
        sf::FloatRect box = g.knob[i];
        float cx = box.left + box.width * 0.5f, cy = box.top + box.height * 0.5f;
        float r = g.r;
        bool hot = box.contains(mouse);
        float v = std::clamp(fxParam(i).load(), 0.0f, 1.0f);
        sf::Color acc = (i == 3 || i == 4) ? sf::Color(172, 128, 255) : Theme::accent;
        if (i == 1) acc = sf::Color(255, 176, 64);

        arc(t, cx, cy, r, 135, 405, 4.0f, Theme::cell);
        if (v > 0.005f) arc(t, cx, cy, r, 135, 135 + 270 * v, 4.0f, acc);

        sf::CircleShape face(r - 8);
        face.setOrigin({ r - 8, r - 8 });
        face.setPosition(cx, cy);
        face.setFillColor(hot ? sf::Color(32, 36, 45) : sf::Color(23, 26, 33));
        face.setOutlineColor(Theme::cardLine);
        face.setOutlineThickness(1);
        t.draw(face);

        sf::RectangleShape ind({ r - 10, 2.0f });
        ind.setOrigin({ 0, 1 });
        ind.setPosition(cx, cy);
        ind.setRotation(135 + 270 * v);
        ind.setFillColor(acc);
        t.draw(ind);

        const char* names[FX_KNOBS] = { "DELAY", "FDBK", "DLY LVL", "REVERB", "REV LVL" };
        char val[24];
        if (i == 0) snprintf(val, sizeof(val), "%.0f ms", 40.0f + v * 760.0f);
        else if (i == 1) snprintf(val, sizeof(val), "%.0f %%", v * 90.0f);
        else snprintf(val, sizeof(val), "%.0f %%", v * 100.0f);

        label(t, names[i], cx, cy + r + 4, 7, Theme::muted, true, true, 1.1f);
        label(t, val, cx, cy + r + 15, 7, Theme::text, true, false, 0.6f);
    }

    void drawFx(sf::RenderTarget& t, sf::Vector2f mouse)
    {
        rrect(t, layout.fx, 0, sf::Color(14, 16, 21));
        label(t, "MASTER FX", layout.fx.left + 12, layout.fx.top + 7, 9, Theme::text, false, true, 1.6f);
        label(t, "delay + reverb bus  -  fed by the mixer sends", layout.fx.left + 12, layout.fx.top + 20, 7, Theme::faint, false, false, 0.4f);

        for (int i = 0; i < FX_KNOBS; ++i) drawFxKnob(t, i, mouse);
    }

    int fxAt(sf::Vector2f p)
    {
        FxGeom g = fxGeom();
        for (int i = 0; i < FX_KNOBS; ++i)
            if (g.knob[i].contains(p)) return i;
        return -1;
    }

    void fxPressed(sf::Vector2f p, sf::Mouse::Button, bool shift)
    {
        int i = fxAt(p);
        if (i < 0) return;
        dragFx = (FxKnob)(FxDelayTime + i);
        dragFxStart = fxParam(i).load();
        dragFxY = p.y;
        (void)shift;
    }

    void fxDrag(sf::Vector2f p)
    {
        int i = (int)dragFx - (int)FxDelayTime;
        if (i < 0 || i >= FX_KNOBS) return;
        float v = std::clamp(dragFxStart + (dragFxY - p.y) / 150.0f, 0.0f, 1.0f);
        fxParam(i) = v;
    }

    void fxWheel(sf::Vector2f p, float delta, bool shift)
    {
        int i = fxAt(p);
        if (i < 0) return;
        float step = (delta > 0 ? 1.0f : -1.0f) * (shift ? 0.1f : 0.02f);
        fxParam(i) = std::clamp(fxParam(i).load() + step, 0.0f, 1.0f);
    }

    // ---------------------------------------------------------------- sequencer
    struct SeqGeom {
        sf::FloatRect card, len16, len32, trMinus, trBox, trPlus, clear;
        float left, top, w, h, colW, rowH;
        int cols, lo, hi;
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

        g.top = layout.seq.top + 58.0f;
        g.h = layout.seq.height - 66.0f;
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
            g.rowH = g.h / 4.0f;
            g.lo = -24;
            g.hi = 24;
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
            g.lo = std::max(0, g.lo - 1);
            g.hi = std::min(127, g.hi + 1);
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

        const char* hint = card.width > 600
            ? "click toggle  /  wheel pitch  /  shift+wheel velocity  /  shift+click accent  /  right click clear"
            : "wheel pitch  /  shift+wheel vel  /  right clear";
        label(t, hint, card.left + card.width - 12, card.top + 8, 7, Theme::faint, false);

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
                        char sn[8];
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
                sf::FloatRect r { g.left + c * g.colW + 2, g.top + y * g.rowH + (g.rowH - bh) - 1, g.colW - 4, bh };
                sf::Color ccol = accent ? sf::Color(255, 176, 64) : (slide ? sf::Color(56, 212, 255) : tr.color);
                bool hot = r.contains(mouse);
                rrect(t, r, 3.0f, ccol, hot ? Theme::text : (accent ? sf::Color(255, 220, 150, 90) : sf::Color(0, 0, 0, 50)));
                if (slide)
                    rrect(t, { r.left, r.top, 2, r.height }, 1, sf::Color(56, 212, 255));
                if (accent)
                    rrect(t, { r.left + r.width - 5, r.top + 2, 3, 3 }, 1.5f, sf::Color(90, 40, 10));
            }

            // step numbers
            for (int c = 0; c < g.cols; c += 4) {
                char sn[8];
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

        int c = seqStepAt(tr, g, p);
        if (c < 0) return;

        if (tr.type == ENGINE_KICK) {
            auto& s = tr.kickSeq->steps[c];
            if (button == sf::Mouse::Right) { s.on = false; s.note = 0; }
            else s.on = !s.on;
            return;
        }

        int note = std::clamp(g.hi - (int)((p.y - g.top) / g.rowH), 0, 127);
        if (tr.type == ENGINE_ACID) {
            auto& s = tr.acidSeq->steps[c];
            if (button == sf::Mouse::Right) { s.on = false; return; }
            if (s.on && (int)s.note == note) {
                if (shift) s.accent = !s.accent;
                else s.on = false;
            } else {
                s.on = true;
                s.note = (uint8_t)note;
                s.velocity = 0.85f;
                s.length = 0.85f;
                s.accent = shift;
                s.slide = false;
            }
        } else {
            auto& s = tr.fm4Seq->steps[c];
            if (button == sf::Mouse::Right) { s.on = false; return; }
            if (s.on && (int)s.note == note) {
                if (shift) s.accent = !s.accent;
                else s.on = false;
            } else {
                s.on = true;
                s.note = (uint8_t)note;
                s.velocity = 0.85f;
                s.length = 0.85f;
                s.accent = shift;
                s.slide = false;
            }
        }
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

    Layout initial = makeLayout(wide);
    float scale = std::min({ 1.0f, (desktop.width - 40.0f) / initial.W, (desktop.height - 100.0f) / initial.H });
    unsigned winWidth = (unsigned)(initial.W * scale);
    unsigned winHeight = (unsigned)(initial.H * scale);
    if (!screenshot && winW >= 640 && winH >= 480) {
        winWidth = std::min((unsigned)winW, desktop.width);
        winHeight = std::min((unsigned)winH, desktop.height);
    }

    sf::ContextSettings ctx;
    ctx.antialiasingLevel = 8;
    sf::RenderWindow window(sf::VideoMode(winWidth, winHeight), "zicStudio", sf::Style::Default, ctx);
    window.setFramerateLimit(60);
    window.setKeyRepeatEnabled(false);
    if (!screenshot && winX != -1 && winY != -1)
        window.setPosition({ winX, winY });
    else
        window.setPosition({ 60, 40 });

    for (auto& t : tracks) t->createUis();

    auto fitView = [&](unsigned w, unsigned h) {
        bool wideNow = w / (float)h >= 1.4f;
        ui.setWide(wideNow);
        const Layout& l = ui.layout;
        sf::View view(sf::FloatRect(0, 0, l.W, l.H));
        float winRatio = (float)w / h, uiRatio = l.W / l.H;
        float sx = 1, sy = 1, px = 0, py = 0;
        if (winRatio > uiRatio) { sx = uiRatio / winRatio; px = (1 - sx) * 0.5f; }
        else { sy = winRatio / uiRatio; py = (1 - sy) * 0.5f; }
        sf::FloatRect vp(px, py, sx, sy);
        view.setViewport(vp);
        ui.baseView = view;
        ui.baseViewport = vp;
        window.setView(view);
    };
    fitView(window.getSize().x, window.getSize().y);
    auto logical = [&](int x, int y) { return window.mapPixelToCoords({ x, y }); };

    if (demo) {
        for (int i = 0; i < TRACK_COUNT; ++i) tracks[i]->type = i % ENGINE_COUNT;
        if (!std::getenv("ZIC_TRACK")) selectedTrack = 0;
        transportPlaying = true;
    }

    int autosave = 0;
    int frame = 0;
    bool shotDone = false;
    while (window.isOpen()) {
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

        auto mp = sf::Mouse::getPosition(window);
        sf::Vector2f uiMouse = screenshot ? sf::Vector2f(-10000.0f, -10000.0f) : logical(mp.x, mp.y);
        ui.draw(window, uiMouse);
        window.display();

        if (screenshot) {
            if (++frame == (demo ? 70 : 10)) {
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
