/**
 * zicStudio: an 8 track rack hosting the Kick32v5, Acid32 and Fm4 engines.
 *
 * - one shared transport (play / stop / BPM) drives every track's sequencer
 * - every track has its own engine, panel, sequencer, mixer strip and colour
 * - one master FX bus (delay + reverb)
 * - the selected track is edited with its real instrument panel (reused from
 *   the standalone studios) and its sequencer, below the panel
 * - one project file remembers everything
 */

#include <alsa/asoundlib.h>
#include <algorithm>
#include <atomic>
#include <dirent.h>
#include <map>
#include <cctype>
#include <chrono>
#include <cmath>
#include <condition_variable>
#include <cstdio>
#include <cstring>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <memory>
#include <mutex>
#include <pthread.h>
#include <sched.h>
#include <sys/prctl.h>
#include <sys/resource.h>
#include <unistd.h>
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

// Tape history for the output strip: a rolling min/max envelope of the last
// 30 seconds of every track. One bucket is 256 samples (~5.8 ms), which is
// roughly one pixel column at a full-screen window.
static constexpr int TAPE_SECONDS = 30;
static constexpr int TAPE_BUCKET = 256;
static constexpr int TAPE_BUCKETS = (int)(SAMPLE_RATE * TAPE_SECONDS) / TAPE_BUCKET;

// globals shared with the reused instrument UIs
std::mutex audioMutex;
std::atomic<bool> keep_running(true);
std::atomic<bool> transportPlaying(false);
std::atomic<float> transportBpm(140.0f);
std::atomic<int> selectedTrack(0);
std::atomic<bool> masterView(false);
std::atomic<int> kickTrigger { 0 };
std::atomic<int> auditionNote { 0 };
std::atomic<bool> kickMuted { false };
std::atomic<bool> acidMuted { false };

// master section (UI <-> audio, all normalised 0..1)
std::atomic<float> masterVolume(0.8f);
std::atomic<float> cpuUsage(0.0f);
std::atomic<float> uiFps(0.0f);
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

struct ThreadInfo {
    int tid = 0;
    std::string name;
    float cpu = 0.0f;
    int core = 0;
};

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

    // rolling 30 s tape of the engine output (oldest .. newest is
    // tapeWrite .. tapeWrite + TAPE_BUCKETS - 1 modulo TAPE_BUCKETS)
    float tapeLo[TAPE_BUCKETS] {};
    float tapeHi[TAPE_BUCKETS] {};
    int tapeWrite = 0;
    int tapeFill = 0;
    float tapeCurLo = 0.0f;
    float tapeCurHi = 0.0f;

    std::atomic<bool> tapeRecording { false };
    std::atomic<bool> tapeFading { false };
    std::atomic<float> tapeFade { 1.0f };

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
    int activeSamples = 0; // engine still in its release tail
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
    t.activeSamples = (int)(SAMPLE_RATE * 3.0f);
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
    t.activeSamples = (int)(SAMPLE_RATE * 3.0f);
    t.acidCap.note = note;
    t.acidCap.hitId++;
}

static void triggerFm4(Track& t, int note, float vel, bool slide)
{
    t.fm4->noteOn((uint8_t)std::clamp(note, 0, 127), vel, slide);
    t.activeSamples = (int)(SAMPLE_RATE * 3.0f);
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

// Fold one sample into the track's tape envelope. Called from the audio
// thread only; the UI reads the buckets while they are being written, which
// is harmless because a torn bucket only shifts one pixel column by a frame.
static void tapeSample(Track& t, float out)
{
    t.tapeCurLo = std::min(t.tapeCurLo, out);
    t.tapeCurHi = std::max(t.tapeCurHi, out);
    if (++t.tapeFill < TAPE_BUCKET) return;
    t.tapeLo[t.tapeWrite] = t.tapeCurLo;
    t.tapeHi[t.tapeWrite] = t.tapeCurHi;
    t.tapeWrite = (t.tapeWrite + 1) % TAPE_BUCKETS;
    t.tapeFill = 0;
    t.tapeCurLo = t.tapeCurHi = 0.0f;
}

// ---------------------------------------------------------------- audio rendering
// Eight tracks of real DSP are too much for one core alongside the UI, so the
// per-track rendering is split over a few workers; the audio thread only mixes
// the finished track buffers and writes them to ALSA.
static constexpr int AUDIO_WORKERS = 3;
static float trackBuf[TRACK_COUNT][NUM_FRAMES];
static std::mutex jobMutex;
static std::condition_variable jobCv, jobDoneCv;
static int jobGen = 0, jobDone = 0;
static float jobBpm = 140.0f;
static int jobSel = 0;

static void renderTrackBlock(Track& t, float bpm, int sel)
{
    const bool playNow = transportPlaying.load();
    for (size_t f = 0; f < NUM_FRAMES; ++f) {
        t.kickSeq->bpm = bpm;
        t.acidSeq->bpm = bpm;
        t.fm4Seq->bpm = bpm;

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
            if (t.activeSamples > 0) {
                out = t.kick->sample();
                captureKick(t, out);
                --t.activeSamples;
            }
        } else if (t.type == ENGINE_ACID) {
            if (acidEv.noteOff) t.acid->noteOff();
            if (acidEv.noteOn) triggerAcid(t, acidEv.note, acidEv.velocity, acidEv.slide, acidEv.accent);
            if (t.activeSamples > 0) {
                out = t.acid->sample();
                captureAcid(t, out);
                --t.activeSamples;
            }
        } else {
            if (fm4Ev.noteOff) t.fm4->noteOff();
            if (fm4Ev.noteOn) triggerFm4(t, fm4Ev.note, fm4Ev.velocity, fm4Ev.slide);
            if (t.activeSamples > 0) {
                out = t.fm4->sample();
                captureFm4(t, out);
                --t.activeSamples;
            }
        }

        float peak = std::fabs(out);
        t.peak = std::max(peak, t.peak * 0.9995f);
        t.meter = t.peak;
        trackBuf[t.index][f] = out;
        // the tape runs while the transport does (or while an auditioned note
        // is still ringing) and freezes on stop, so the take stays readable
        if (t.tapeRecording.load() && (playNow || t.activeSamples > 0)) tapeSample(t, out);
        else if (t.tapeFading.load()) tapeSample(t, 0.0f);
    }
}

static void audioTrackWorker(int wid)
{
    char nm[16];
    std::snprintf(nm, sizeof(nm), "zic-work%d", wid);
    prctl(PR_SET_NAME, (unsigned long)nm, 0, 0, 0);
    int seen = 0;
    while (keep_running) {
        float bpm;
        int sel;
        {
            std::unique_lock<std::mutex> lk(jobMutex);
            jobCv.wait(lk, [&] { return jobGen != seen || !keep_running; });
            if (!keep_running) break;
            seen = jobGen;
            bpm = jobBpm;
            sel = jobSel;
        }
        for (int ti = wid; ti < TRACK_COUNT; ti += AUDIO_WORKERS)
            renderTrackBlock(*tracks[ti], bpm, sel);
        {
            std::lock_guard<std::mutex> lk(jobMutex);
            if (++jobDone >= AUDIO_WORKERS) jobDoneCv.notify_one();
        }
    }
}

void audioWorker(snd_pcm_t* pcm)
{
    prctl(PR_SET_NAME, (unsigned long)"zic-audio", 0, 0, 0);
    std::vector<int16_t> buffer(NUM_FRAMES * 2);
    StudioFx delayFx, reverbFx;
    delayFx.reverbAmount = 0.0f;
    delayFx.delayLevel = 1.0f;
    reverbFx.delayLevel = 0.0f;
    bool wasPlaying = false;
    float meter = 0.0f;
    // auditioned notes are one-shot: release them after a short gate so a
    // click in the sequencer never leaves a note hanging
    int auditionLeft = 0;
    int auditionTrack = -1;
    int auditionEngine = -1;
    int idleBlocks = 0;

    auto writeBlock = [&]() {
        if (!pcm) {
            std::this_thread::sleep_for(std::chrono::microseconds(11600));
            return;
        }
        size_t off = 0;
        while (off < NUM_FRAMES) {
            snd_pcm_sframes_t w = snd_pcm_writei(pcm, buffer.data() + off * 2, NUM_FRAMES - off);
            if (w < 0) {
                if (snd_pcm_recover(pcm, (int)w, 0) < 0) {
                    std::this_thread::sleep_for(std::chrono::milliseconds(10));
                    break;
                }
            } else if (w == 0) {
                break;
            } else {
                off += (size_t)w;
            }
        }
    };

    while (keep_running) {
        // Idle bypass: once stopped with no voices or tape tails left (and after
        // a short hold for FX tails), do not run the DSP just to emit silence.
        bool idle = !transportPlaying.load() && auditionLeft <= 0 &&
                    auditionNote.load() == 0 && kickTrigger.load() == 0;
        if (idle) {
            for (auto& tp : tracks) {
                if (tp->activeSamples > 0 || tp->tapeFading.load()) { idle = false; break; }
            }
        }
        if (!idle) idleBlocks = 0;
        else if (idleBlocks < 1000) idleBlocks++;
        if (idle && idleBlocks > 120) {
            for (auto& tp : tracks) tp->meter = 0.0f;
            masterMeter = 0.0f;
            for (auto& s : buffer) s = 0;
            writeBlock();
            continue;
        }

        {
            std::lock_guard<std::mutex> lock(audioMutex);
            bool play = transportPlaying.load();
            float bpm = transportBpm.load();

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
                        for (int i = 0; i < TAPE_BUCKETS; ++i) { t.tapeLo[i] = 0.0f; t.tapeHi[i] = 0.0f; }
                        t.tapeWrite = 0;
                        t.tapeFill = 0;
                        t.tapeCurLo = t.tapeCurHi = 0.0f;
                        t.tapeRecording.store(true);
                        t.tapeFading.store(false);
                        t.tapeFade.store(1.0f);
                    } else {
                        t.kickSeq->stop();
                        t.acidSeq->stop();
                        t.fm4Seq->stop();
                        t.acid->noteOff();
                        t.fm4->noteOff();
                        t.tapeRecording.store(false);
                        t.tapeFading.store(true);
                        t.tapeFade.store(1.0f);
                    }
                }
                wasPlaying = play;
            }

            int sel = selectedTrack.load();
            int trig = auditionNote.exchange(0);
            int kTrig = kickTrigger.exchange(0);

            bool anySolo = false;
            for (auto& tp : tracks) anySolo = anySolo || tp->solo;

            if (kTrig) {
                Track& t = *tracks[sel];
                if (t.type == ENGINE_KICK) triggerKick(t, 60);
            }
            if (trig > 0) {
                if (auditionLeft > 0 && auditionTrack >= 0) {
                    if (auditionEngine == ENGINE_ACID) tracks[auditionTrack]->acid->noteOff();
                    else if (auditionEngine == ENGINE_FM4) tracks[auditionTrack]->fm4->noteOff();
                }
                Track& t = *tracks[sel];
                if (t.type == ENGINE_ACID) {
                    triggerAcid(t, trig, 1.0f, false, true);
                    auditionTrack = sel; auditionEngine = ENGINE_ACID;
                    auditionLeft = (int)(SAMPLE_RATE * (play ? 1.2f : 0.45f));
                } else if (t.type == ENGINE_FM4) {
                    triggerFm4(t, trig, 1.0f, false);
                    auditionTrack = sel; auditionEngine = ENGINE_FM4;
                    auditionLeft = (int)(SAMPLE_RATE * (play ? 1.2f : 0.45f));
                }
            }

            // precompute the per-track mix gains once per block
            bool audible[TRACK_COUNT] = {};
            float gl[TRACK_COUNT] = {}, gr[TRACK_COUNT] = {};
            float gdL[TRACK_COUNT] = {}, gdR[TRACK_COUNT] = {};
            float gvL[TRACK_COUNT] = {}, gvR[TRACK_COUNT] = {};
            for (int i = 0; i < TRACK_COUNT; ++i) {
                Track& t = *tracks[i];
                audible[i] = !t.mute && (!anySolo || t.solo);
                float ang = (t.pan + 1.0f) * 0.25f * 3.14159265f;
                float l = std::cos(ang), r = std::sin(ang);
                gl[i] = t.volume * l;
                gr[i] = t.volume * r;
                gdL[i] = gl[i] * t.delaySend;
                gdR[i] = gr[i] * t.delaySend;
                gvL[i] = gl[i] * t.reverbSend;
                gvR[i] = gr[i] * t.reverbSend;
            }

            // render the eight tracks on the worker pool, then mix their buffers
            {
                std::lock_guard<std::mutex> jl(jobMutex);
                jobBpm = bpm;
                jobSel = sel;
                jobDone = 0;
                jobGen++;
            }
            jobCv.notify_all();
            {
                std::unique_lock<std::mutex> jl(jobMutex);
                // A shutdown between notify_all and the workers picking the job
                // would otherwise leave this wait pending forever.
                jobDoneCv.wait(jl, [] { return jobDone >= AUDIO_WORKERS || !keep_running; });
                if (!keep_running) break;
            }

            for (size_t f = 0; f < NUM_FRAMES; ++f) {
                float dryL = 0.0f, dryR = 0.0f;
                float delayL = 0.0f, delayR = 0.0f;
                float reverbL = 0.0f, reverbR = 0.0f;
                for (int i = 0; i < TRACK_COUNT; ++i) {
                    if (!audible[i]) continue;
                    float out = trackBuf[i][f];
                    dryL += out * gl[i];
                    dryR += out * gr[i];
                    delayL += out * gdL[i];
                    delayR += out * gdR[i];
                    reverbL += out * gvL[i];
                    reverbR += out * gvR[i];
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

                buffer[f * 2] = (int16_t)(outL * 32767.0f);
                buffer[f * 2 + 1] = (int16_t)(outR * 32767.0f);
            }

            // release the auditioned note after the block's render, never while
            // a worker is touching that engine
            if (auditionLeft > 0) {
                auditionLeft -= (int)NUM_FRAMES;
                if (auditionLeft <= 0 && auditionTrack >= 0) {
                    auditionLeft = 0;
                    if (auditionEngine == ENGINE_ACID) tracks[auditionTrack]->acid->noteOff();
                    else if (auditionEngine == ENGINE_FM4) tracks[auditionTrack]->fm4->noteOff();
                }
            }

            masterMeter = meter;
            meter *= 0.90f;
        }
        // smooth fade-out of the tape after stop
        {
            const float dt = (float)NUM_FRAMES / (float)SAMPLE_RATE;
            for (auto& tp : tracks) {
                Track& t = *tp;
                if (!t.tapeFading.load()) continue;
                float f = t.tapeFade.load() - dt * 0.6f;
                if (f <= 0.0f) {
                    f = 0.0f;
                    t.tapeFading.store(false);
                    for (int i = 0; i < TAPE_BUCKETS; ++i) { t.tapeLo[i] = 0.0f; t.tapeHi[i] = 0.0f; }
                    t.tapeWrite = 0;
                    t.tapeFill = 0;
                    t.tapeCurLo = t.tapeCurHi = 0.0f;
                }
                t.tapeFade.store(f);
            }
        }
        writeBlock();
    }
}

// ---------------------------------------------------------------- UI

namespace Theme {
inline sf::Color rgb(int r, int g, int b, int a = 255) { return sf::Color(r, g, b, a); }
const sf::Color bg = rgb(10, 12, 16);
const sf::Color card = rgb(18, 21, 27);
const sf::Color cardLine = rgb(34, 38, 48);
const sf::Color cell = rgb(27, 30, 38);
const sf::Color track = rgb(44, 47, 58);
const sf::Color cap = rgb(28, 30, 39);
const sf::Color text = rgb(234, 236, 242);
const sf::Color muted = rgb(126, 131, 148);
const sf::Color faint = rgb(68, 72, 88);
const sf::Color accent = rgb(56, 212, 255);
}

static sf::Color alpha(sf::Color c, int a) { c.a = (sf::Uint8)std::clamp(a, 0, 255); return c; }

struct Layout {
    bool wide = true;
    bool seqRight = true;
    float split = 0.0f;
    float W = 1920.0f, H = 1080.0f;
    sf::FloatRect sidebar, panel, seq;
};

static Layout makeLayout(bool wide, float winW, float winH, float split)
{
    Layout l;
    l.wide = wide;
    l.seqRight = false; // sequencer is always below the panel
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

    // True when the next frame would change something. The main loop uses this
    // to skip drawing entirely while the app is idle.
    bool wantsRedraw() const { return frameDirty || chromeDirty || panelDirty || systemModalOpen; }
    bool monitorOpen() const { return systemModalOpen; }
    void openMonitor() { systemModalOpen = true; frameDirty = true; }
    void closeMonitor() { systemModalOpen = false; frameDirty = true; }

    // Sample system-wide per-core CPU, per-thread process CPU and the process
    // RSS. Cheap enough to run from the UI loop twice a second; no extra thread.
    bool sampleSystem()
    {
        auto now = std::chrono::steady_clock::now();
        if (now - sysSampleAt < std::chrono::milliseconds(500)) return false;
        double dtSec = std::chrono::duration<double>(now - sysSampleAt).count();
        if (dtSec <= 0.0) dtSec = 0.5;
        sysSampleAt = now;

        std::vector<unsigned long long> active, total;
        FILE* stat = std::fopen("/proc/stat", "r");
        if (stat) {
            char line[256];
            while (std::fgets(line, sizeof(line), stat)) {
                if (std::strncmp(line, "cpu", 3) != 0) break;
                if (line[3] == ' ') continue; // aggregate line
                unsigned long long user = 0, nice = 0, sys = 0, idle = 0;
                unsigned long long iowait = 0, irq = 0, softirq = 0, steal = 0;
                int n = std::sscanf(line, "cpu%*d %llu %llu %llu %llu %llu %llu %llu %llu",
                    &user, &nice, &sys, &idle, &iowait, &irq, &softirq, &steal);
                if (n < 4) continue;
                unsigned long long a = user + nice + sys + irq + softirq + steal;
                active.push_back(a);
                total.push_back(a + idle + iowait);
            }
            std::fclose(stat);
        }

        if (!total.empty()) {
            std::vector<float> pct(total.size(), 0.0f);
            if (prevCoreTotal.size() == total.size()) {
                for (size_t i = 0; i < total.size(); ++i) {
                    unsigned long long dt = total[i] - prevCoreTotal[i];
                    if (dt == 0) continue;
                    double p = 100.0 * (double)(active[i] - prevCoreActive[i]) / (double)dt;
                    pct[i] = (float)std::clamp(p, 0.0, 100.0);
                }
            }
            prevCoreActive = active;
            prevCoreTotal = total;
            coreCpuPct = pct;
            float sum = 0.0f;
            for (float p : pct) sum += p;
            sysCpuAvg = pct.empty() ? 0.0f : sum / (float)pct.size();
            coreSampled = true;
        }

        FILE* statm = std::fopen("/proc/self/statm", "r");
        if (statm) {
            unsigned long size = 0, rss = 0;
            if (std::fscanf(statm, "%lu %lu", &size, &rss) == 2) {
                long ps = sysconf(_SC_PAGESIZE);
                procRamMb = (float)((double)rss * (double)ps / (1024.0 * 1024.0));
            }
            std::fclose(statm);
        }

        // per-thread CPU from /proc/self/task/*/stat
        long clk = sysconf(_SC_CLK_TCK);
        if (clk <= 0) clk = 100;
        std::map<int, unsigned long long> ticksNow;
        std::vector<ThreadInfo> sampled;
        if (DIR* td = opendir("/proc/self/task")) {
            while (dirent* de = readdir(td)) {
                if (!std::isdigit((unsigned char)de->d_name[0])) continue;
                int tid = std::atoi(de->d_name);
                char path[64];
                std::snprintf(path, sizeof(path), "/proc/self/task/%d/stat", tid);
                FILE* f = std::fopen(path, "r");
                if (!f) continue;
                char line[512];
                bool ok = std::fgets(line, sizeof(line), f) != nullptr;
                std::fclose(f);
                if (!ok) continue;

                char* lp = std::strchr(line, '(');
                char* rp = std::strrchr(line, ')');
                if (!lp || !rp || rp <= lp) continue;
                std::string name(lp + 1, rp);
                std::istringstream iss(std::string(rp + 2));
                std::vector<std::string> tok;
                std::string s;
                while (iss >> s) tok.push_back(s);
                if (tok.size() < 37) continue;

                unsigned long long ticks = std::strtoull(tok[11].c_str(), nullptr, 10) +
                                           std::strtoull(tok[12].c_str(), nullptr, 10);
                ticksNow[tid] = ticks;
                ThreadInfo ti;
                ti.tid = tid;
                ti.name = name;
                ti.core = std::atoi(tok[36].c_str());
                auto prev = prevThreadTicks.find(tid);
                if (prev != prevThreadTicks.end()) {
                    double p = 100.0 * (double)(ticks - prev->second) / ((double)clk * dtSec);
                    ti.cpu = (float)std::clamp(p, 0.0, 100.0);
                }
                sampled.push_back(std::move(ti));
            }
            closedir(td);
        }
        if (!sampled.empty()) {
            prevThreadTicks = std::move(ticksNow);
            std::sort(sampled.begin(), sampled.end(), [](const ThreadInfo& a, const ThreadInfo& b) { return a.cpu > b.cpu; });
            threads = std::move(sampled);
        }
        return true;
    }

    void draw(sf::RenderTarget& t, sf::Vector2f mouse)
    {
        int sel = selectedTrack.load();
        Track& tr = *tracks[sel];
        bool mv = masterView.load();
        bool playing = transportPlaying.load();

        // Only repaint the static shell when its contents actually change.
        // Meters and the sequencer playhead are cheap overlays drawn below.
        if (sel != chromeSel || mv != chromeMasterView || playing != chromePlaying || tr.type != chromeType) {
            chromeSel = sel;
            chromeMasterView = mv;
            chromePlaying = playing;
            chromeType = tr.type;
            chromeDirty = true;
        }
        bool mouseInChrome = inRect(layout.sidebar, mouse) ||
                             (!mv && (inRect(layout.seq, mouse) || splitterRect().contains(mouse)));
        bool mouseWasInChrome = inRect(layout.sidebar, chromeMouse) ||
                                (!chromeMasterView && (inRect(layout.seq, chromeMouse) || splitterRect().contains(chromeMouse)));
        if ((mouseInChrome || mouseWasInChrome) && (mouse.x != chromeMouse.x || mouse.y != chromeMouse.y))
            chromeDirty = true;
        chromeMouse = mouse;

        // Slow refresh for the CPU/fps text (updated at ~2 Hz by the main loop).
        auto now = std::chrono::steady_clock::now();
        if (now - chromeTextAt >= std::chrono::milliseconds(500)) {
            chromeTextAt = now;
            chromeDirty = true;
        }

        t.setView(baseView);
        t.clear(Theme::bg);
        if (chromeDirty || !chromeRTValid) drawChrome();
        // Blit only the shell's two visible regions, not the whole window.
        {
            sf::Sprite side(chromeRT.getTexture(),
                sf::IntRect(0, 0, (int)layout.sidebar.width, (int)std::ceil(layout.H)));
            t.draw(side);
            if (!mv) {
                int x = (int)layout.sidebar.width;
                int y = (int)std::max(0.0f, layout.seq.top - 4.0f);
                int w = (int)std::ceil(layout.W - (float)x);
                int h = (int)std::ceil(layout.H - (float)y);
                sf::Sprite seq(chromeRT.getTexture(), sf::IntRect(x, y, w, h));
                seq.setPosition((float)x, (float)y);
                t.draw(seq);
            }
        }

        // Lightweight live parts on top of the cached shell.
        drawLiveMeters(t);

        // The instrument panel (or master view) is the expensive content, and
        // keeps its own cache/refresh policy.
        if (mv)
            drawMasterView(t, mouse);
        else
            drawInstrument(t, tr, mouse);

        if (!mv) {
            drawTape(t, tr);
            drawSeqCursor(t, tr);
        }
        if (systemModalOpen) drawSystemMonitorModal(t, mouse);

        chromeDirty = false;
        panelDirty = false;
        frameDirty = false;
    }

    // Repaint the static shell (sidebar + sequencer + splitter) into a texture.
    // Everything that animates is drawn as a small overlay in draw().
    void drawChrome()
    {
        unsigned w = (unsigned)std::max(1.0f, std::ceil(layout.W));
        unsigned h = (unsigned)std::max(1.0f, std::ceil(layout.H));
        if (w != chromeW || h != chromeH) {
            if (!chromeRT.create(w, h)) return;
            chromeW = w;
            chromeH = h;
        }
        int sel = selectedTrack.load();
        Track& tr = *tracks[sel];
        chromeRT.setView(baseView);
        chromeRT.clear(Theme::bg);
        cachingChrome = true;
        drawSidebar(chromeRT, chromeMouse);
        if (!masterView.load()) {
            drawSequencer(chromeRT, tr, chromeMouse);
            drawSplitter(chromeRT, chromeMouse);
        }
        cachingChrome = false;
        chromeRT.display();
        chromeRTValid = true;
    }

    // The only parts of the sidebar that move every frame: the activity meters.
    void drawLiveMeters(sf::RenderTarget& t)
    {
        float lvl = std::clamp(masterMeter.load(), 0.0f, 1.0f);
        if (lvl > 0.02f)
            rrect(t, { masterViewBtnRect.left + masterViewBtnRect.width - 2, masterViewBtnRect.top + 4, 2, masterViewBtnRect.height - 8 },
                1.0f, Theme::accent);
        int sel = selectedTrack.load();
        for (int i = 0; i < TRACK_COUNT; ++i) {
            Track& tr = *tracks[i];
            float m = std::clamp(tr.meter.load(), 0.0f, 1.0f);
            if (tr.mute || m <= 0.02f) continue;
            const sf::FloatRect& row = trackRowRect[i];
            rrect(t, { row.left + row.width - 2, row.top + 4, 2, row.height - 8 }, 1.0f, sel == i ? tr.color : Theme::accent);
        }
    }

    // The playhead moves at the sequencer rate, so keep it out of the cached
    // shell and draw one cheap highlight each frame.
    void drawSeqCursor(sf::RenderTarget& t, Track& tr)
    {
        if (tr.type == ENGINE_KICK) {
            int cur = tr.kickSeq->current.load();
            if (cur < 0) return;
            SeqGeom g = seqGeom(tr);
            int row = cur / 16, col = cur % 16;
            if (row < 0 || row > 3) return;
            sf::FloatRect r { g.left + col * g.colW + 2, g.top + row * g.rowH + 3, g.colW - 4, g.rowH - 6 };
            rrect(t, r, 5.0f, sf::Color(255, 255, 255, 14), Theme::text);
            return;
        }

        int cur, page;
        if (tr.type == ENGINE_ACID) {
            cur = tr.acidSeq->currentStep.load();
            page = synthSeqState[tr.index].page;
        } else {
            cur = tr.fm4Seq->currentStep.load();
            page = synthSeqState[tr.index].page;
        }
        int col = cur - page * 16;
        if (col < 0 || col >= 16) return;
        SynthGeom g = synthGeom();
        float x = g.grid.left + col * g.cellW;
        rrect(t, { x, g.grid.top, g.cellW, g.grid.height }, 0, sf::Color(230, 255, 40, 14));
        rrect(t, { x, g.grid.top, 1, g.grid.height }, 0, sf::Color(230, 255, 40, 55));
        rrect(t, { x + g.cellW - 1, g.grid.top, 1, g.grid.height }, 0, sf::Color(230, 255, 40, 55));
    }

    void drawSystemMonitorModal(sf::RenderTarget& t, sf::Vector2f mouse)
    {
        sf::RectangleShape dim({ layout.W, layout.H });
        dim.setFillColor(sf::Color(0, 0, 0, 175));
        t.draw(dim);

        float w = std::clamp(layout.W * 0.68f, 720.0f, 1100.0f);
        float h = std::clamp(layout.H * 0.70f, 560.0f, 820.0f);
        float x = (layout.W - w) * 0.5f;
        float y = (layout.H - h) * 0.5f;
        monitorModalRect = { x, y, w, h };

        rrect(t, monitorModalRect, 16.0f, sf::Color(16, 18, 24), Theme::cardLine);
        rrect(t, { x + 24, y, w - 48, 3 }, 1.5f, alpha(Theme::accent, 170));
        label(t, "SYSTEM MONITOR", x + 30, y + 22, 22, Theme::text, false, true, 1.0f);
        label(t, "per-thread CPU   |   per-core load   |   process RSS", x + 32, y + 52, 11, Theme::muted, false, false, 0.4f);

        monitorCloseRect = { x + w - 52, y + 20, 34, 34 };
        bool closeHot = monitorCloseRect.contains(mouse);
        rrect(t, monitorCloseRect, 8.0f, closeHot ? sf::Color(230, 70, 80) : Theme::cell, Theme::cardLine);
        label(t, "X", monitorCloseRect.left + 12, monitorCloseRect.top + 9, 16,
            closeHot ? sf::Color(255, 255, 255) : Theme::text, true, true);

        // summary cards
        float sy = y + 88.0f;
        float gap = 14.0f;
        float cardW = (w - 60.0f - gap * 3.0f) / 4.0f;
        char vCpu[24], vSys[24], vRam[24], vCores[24];
        snprintf(vCpu, sizeof(vCpu), "%.0f%%", cpuUsage.load());
        snprintf(vSys, sizeof(vSys), "%.0f%%", sysCpuAvg);
        if (procRamMb >= 1024.0f) snprintf(vRam, sizeof(vRam), "%.1f GB", procRamMb / 1024.0f);
        else snprintf(vRam, sizeof(vRam), "%.0f MB", procRamMb);
        snprintf(vCores, sizeof(vCores), "%d", (int)coreCpuPct.size());
        const char* labels[4] = { "PROCESS CPU", "SYSTEM CPU", "PROCESS RAM", "CORES" };
        const char* values[4] = { vCpu, vSys, vRam, vCores };
        for (int i = 0; i < 4; ++i) {
            sf::FloatRect c { x + 30 + i * (cardW + gap), sy, cardW, 70 };
            rrect(t, c, 9.0f, Theme::card, Theme::cardLine);
            label(t, labels[i], c.left + 12, c.top + 10, 11, Theme::muted, false, false, 1.0f);
            sf::Color col = i == 1 ? sf::Color(255, 255, 255) : Theme::accent;
            label(t, values[i], c.left + 12, c.top + 30, 24, col, false, true, 0.2f);
        }

        // per-thread table
        float ty = sy + 96.0f;
        float cpuCol = x + w - 330.0f;
        label(t, "THREAD", x + 32, ty, 12, Theme::muted, false, false, 1.2f);
        label(t, "CPU", cpuCol, ty, 12, Theme::muted, false, false, 1.2f);
        label(t, "CORE", x + w - 100, ty, 12, Theme::muted, false, false, 1.2f);
        rrect(t, { x + 30, ty + 20, w - 60, 1 }, 0, Theme::cardLine);

        const float rowH = 32.0f;
        float rowY = ty + 32.0f;
        int maxRows = std::max(1, (int)((y + h - rowY - 46.0f) / rowH));
        if (threads.empty()) {
            label(t, "sampling...", x + 34, rowY + 8, 13, Theme::muted);
        } else {
            for (int i = 0; i < (int)threads.size() && i < maxRows; ++i) {
                const ThreadInfo& th = threads[i];
                float ry = rowY + i * rowH;
                rrect(t, { x + 30, ry, w - 60, rowH - 2 }, 7.0f, Theme::card);
                float lvl = std::clamp(th.cpu / 100.0f, 0.0f, 1.0f);
                sf::Color col = lvl > 0.85f ? sf::Color(255, 86, 98)
                              : (lvl > 0.6f ? sf::Color(255, 200, 60) : Theme::accent);
                rrect(t, { x + 38, ry + 11, 5, 10 }, 2.5f, col);

                std::string name = th.name + "   #" + std::to_string(th.tid);
                label(t, name, x + 52, ry + 10, 12, Theme::text, false, true, 0.2f);

                sf::FloatRect bar { cpuCol, ry + 12, 180, 8 };
                rrect(t, bar, 4.0f, Theme::cell);
                if (lvl > 0.005f)
                    rrect(t, { bar.left, bar.top, bar.width * lvl, bar.height }, 4.0f, col);
                char pct[16];
                snprintf(pct, sizeof(pct), "%.0f%%", th.cpu);
                labelR(t, pct, x + w - 118, ry + 10, 12, Theme::text, true, 0.2f);
                char core[16];
                snprintf(core, sizeof(core), "C%d", th.core);
                label(t, core, x + w - 88, ry + 10, 12, Theme::muted, false, true, 0.2f);
            }
        }

        label(t, "click outside or press X / Esc to close", x + 32, y + h - 28, 10, Theme::muted);
    }

    void mousePressed(sf::Vector2f p, sf::Mouse::Button button, bool shift)
    {
        Layout& l = layout;
        int sel = selectedTrack.load();
        panelDragging = false;
        panelDirty = true;
        chromeDirty = true;

        if (systemModalOpen) {
            if (monitorCloseRect.contains(p) || !monitorModalRect.contains(p)) closeMonitor();
            frameDirty = true;
            return;
        }
        if (cpuRect.contains(p)) { systemModalOpen = true; frameDirty = true; return; }

        if (!masterView.load() && splitterRect().contains(p)) { dragSplit = true; return; }
        if (inRect(l.sidebar, p)) { sidebarPressed(p, button, shift); return; }
        if (masterView.load()) { masterPanelPressed(p, button, shift); return; }
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
        if (systemModalOpen) return;
        chromeDirty = true;
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
        dragMasterPanelVol = false;
        dragSplit = false;
        seqPainting = false;
        synthSeqReleased();
    }

    void mouseMoved(sf::Vector2f p, bool shift)
    {
        frameDirty = true;
        if (systemModalOpen) return;
        Layout& l = layout;
        int sel = selectedTrack.load();
        Track& tr = *tracks[sel];
        if (inRect(l.sidebar, p) || (!masterView.load() && (inRect(l.seq, p) || splitterRect().contains(p))) ||
            seqPainting || synthDragStep >= 0 || synthDragScroll ||
            dragSidebarVol >= 0 || dragMasterVol || dragFx != FxNone)
            chromeDirty = true;

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
        if (dragMasterPanelVol) { masterPanelVolDrag(p); return; }
        if (seqPainting) { seqPaint(p); return; }

        if (panelDragging) { panelMouseMoved(tr, toPanelRaw(tr, p), shift); return; }

        if (synthDragStep >= 0 || synthDragScroll) {
            int ti = synthDragTrack >= 0 ? synthDragTrack : selectedTrack.load();
            Track& dt = *tracks[ti];
            if (dt.type == ENGINE_ACID) synthSeqMoved(*dt.acidSeq, dt, p, shift);
            else if (dt.type == ENGINE_FM4) synthSeqMoved(*dt.fm4Seq, dt, p, shift);
            return;
        }

        if (inRect(l.panel, p)) {
            sf::Vector2f q;
            if (toPanel(tr, p, q)) panelMouseMoved(tr, q, shift);
        }
    }

    void wheel(sf::Vector2f p, float delta, bool shift)
    {
        if (systemModalOpen) return;
        Layout& l = layout;
        int sel = selectedTrack.load();
        Track& tr = *tracks[sel];
        panelDirty = true;
        chromeDirty = true;

        if (inRect(l.sidebar, p)) {
            sidebarWheel(p, delta, shift);
        } else if (masterView.load()) {
            masterPanelWheel(p, delta, shift);
        } else if (inRect(l.panel, p)) {
            sf::Vector2f q;
            if (toPanel(tr, p, q)) panelWheel(tr, q, delta, shift);
        } else if (inRect(l.seq, p)) {
            seqWheel(p, delta, shift);
        }
    }

    void key(sf::Keyboard::Key k)
    {
        if (systemModalOpen) return;
        panelDirty = true;
        chromeDirty = true;
        if (k == sf::Keyboard::Space || k == sf::Keyboard::P || k == sf::Keyboard::Return) {
            transportPlaying = !transportPlaying.load();
            return;
        }
        if (k >= sf::Keyboard::Num1 && k <= sf::Keyboard::Num8) {
            selectedTrack = (int)k - (int)sf::Keyboard::Num1;
            return;
        }
        int sel = selectedTrack.load();
        Track& tr = *tracks[sel];
        bool seqKey = k == sf::Keyboard::Tab || k == sf::Keyboard::Up || k == sf::Keyboard::Down ||
                      k == sf::Keyboard::Left || k == sf::Keyboard::Right;
        if (seqKey && tr.type == ENGINE_ACID) { synthSeqKey(*tr.acidSeq, tr, k); return; }
        if (seqKey && tr.type == ENGINE_FM4) { synthSeqKey(*tr.fm4Seq, tr, k); return; }
        // let the instrument panel have the keys too (presets etc.)
        panelKey(tr, k);
    }

    Layout layout { makeLayout(true, 1920.0f, 1080.0f, 0.0f) };
    sf::View baseView;
    sf::FloatRect baseViewport { 0, 0, 1, 1 };

    // Static shell cache: sidebar, sequencer and splitter. Only repainted when
    // input/state changes; meters and the playhead are overlays in draw().
    sf::RenderTexture chromeRT;
    bool chromeRTValid = false;
    bool chromeDirty = true;
    bool cachingChrome = false;
    bool frameDirty = true;
    unsigned chromeW = 0, chromeH = 0;
    std::chrono::steady_clock::time_point chromeTextAt = std::chrono::steady_clock::now();
    int chromeSel = -1;
    int chromeType = -1;
    bool chromeMasterView = false;
    bool chromePlaying = false;
    sf::Vector2f chromeMouse { -10000.0f, -10000.0f };
    sf::VertexArray tapeRibbon;

    // system monitor (sampled from /proc on the UI thread)
    std::vector<float> coreCpuPct;
    std::vector<unsigned long long> prevCoreActive, prevCoreTotal;
    float sysCpuAvg = 0.0f;
    float procRamMb = 0.0f;
    bool coreSampled = false;
    std::chrono::steady_clock::time_point sysSampleAt = std::chrono::steady_clock::now();
    std::vector<ThreadInfo> threads;
    std::map<int, unsigned long long> prevThreadTicks;
    bool systemModalOpen = false;
    sf::FloatRect monitorModalRect;
    sf::FloatRect monitorCloseRect;

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
        layout = makeLayout(wide, w, h, seqSplit);
        // Keep every track's panel in the same responsive mode, so a click right
        // after switching tracks maps to the right knobs.
        for (auto& tp : tracks) {
            if (!tp->kickUi) continue;
            tp->kickUi->setWideLayout(wide, layout.panel.width - 24.0f);
            tp->acidUi->setWideLayout(wide, layout.panel.width - 24.0f);
            tp->fm4Ui->setWideLayout(wide, layout.panel.width - 24.0f);
        }
        chromeDirty = true;
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
    bool dragMasterPanelVol = false;
    bool dragSplit = false;
    int engineMenuTrack = -1;   // track whose engine cog menu is open
    sf::FloatRect seqPosRect;
    int seqSelected = -1;       // sequencer step whose edit strip is open
    bool seqPainting = false;   // dragging across the grid paints notes
    bool seqPaintOn = true;
    int synthDragStep = -1;
    int synthDragTrack = -1;
    bool synthDragLen = false;
    bool synthDragScroll = false;
    sf::FloatRect trackCogRect[TRACK_COUNT];
    sf::FloatRect trackRowRect[TRACK_COUNT];
    sf::FloatRect trackVolRect[TRACK_COUNT];
    sf::FloatRect trackMuteRect[TRACK_COUNT];
    sf::FloatRect trackSoloRect[TRACK_COUNT];
    sf::FloatRect engineDropRect;
    sf::FloatRect engineDropItemRect[ENGINE_COUNT];
    sf::FloatRect masterVolRect;
    sf::FloatRect masterViewBtnRect;
    sf::FloatRect masterPanelVolRect;
    sf::FloatRect masterPanelFxRect[FX_KNOBS];
    sf::FloatRect masterPanelMeterRect;
    sf::FloatRect playBtnRect;
    sf::FloatRect bpmBoxRect, bpmMinusRect, bpmPlusRect;
    sf::FloatRect cpuRect;
    float dragFxStart = 0.0f;
    float dragFxY = 0.0f;
    float masterAnim = 0.0f;

    sf::Font font, fontBold;
    bool haveBold = false;

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
        // A real bold face instead of sf::Text::Bold: SFML fakes bold by
        // emboldening the glyph outline at raster time, which smears small text.
        const char* boldPaths[] = {
            "/usr/share/fonts/truetype/roboto/unhinted/RobotoTTF/Roboto-Bold.ttf",
            "../../../draw/fonts/Roboto-Bold.ttf", "../../draw/fonts/Roboto-Bold.ttf",
            "../draw/fonts/Roboto-Bold.ttf", "draw/fonts/Roboto-Bold.ttf",
            "/usr/share/fonts/truetype/dejavu/DejaVuSans-Bold.ttf"
        };
        for (auto p : boldPaths)
            if (fontBold.loadFromFile(p)) { haveBold = true; break; }
    }

    void label(sf::RenderTarget& t, const std::string& s, float x, float y, int size, sf::Color c,
        bool center = false, bool bold = false, float spacing = 1.0f)
    {
        bool realBold = bold && haveBold;
        sf::Text txt(s, realBold ? fontBold : font, size);
        txt.setFillColor(c);
        txt.setLetterSpacing(spacing);
        if (bold && !realBold) txt.setStyle(sf::Text::Bold);
        auto b = txt.getLocalBounds();
        txt.setOrigin(center ? b.left + b.width * 0.5f : b.left, b.top);
        txt.setPosition(std::round(x), std::round(y));
        t.draw(txt);
    }

    void labelR(sf::RenderTarget& t, const std::string& s, float rightX, float y, int size, sf::Color c,
        bool bold = false, float spacing = 1.0f)
    {
        bool realBold = bold && haveBold;
        sf::Text txt(s, realBold ? fontBold : font, size);
        txt.setFillColor(c);
        txt.setLetterSpacing(spacing);
        if (bold && !realBold) txt.setStyle(sf::Text::Bold);
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

    void disc(sf::RenderTarget& t, float x, float y, float r, sf::Color c)
    {
        sf::CircleShape s(r, 32);
        s.setOrigin(r, r);
        s.setPosition(x, y);
        s.setFillColor(c);
        t.draw(s);
    }

    // degrees, 0 = up, clockwise
    void arc(sf::RenderTarget& t, float cx, float cy, float r, float thick, float a0, float a1, sf::Color c, bool caps = true)
    {
        if (a1 < a0) std::swap(a0, a1);
        if (a1 - a0 < 0.01f) return;
        int n = std::max(2, (int)((a1 - a0) / 4.0f) + 1);
        sf::VertexArray va(sf::TriangleStrip, 2 * (n + 1));
        for (int i = 0; i <= n; ++i) {
            float a = (a0 + (a1 - a0) * i / n) * 3.14159265f / 180.0f;
            float s = std::sin(a), co = std::cos(a);
            va[2 * i] = sf::Vertex({ cx + (r + thick * 0.5f) * s, cy - (r + thick * 0.5f) * co }, c);
            va[2 * i + 1] = sf::Vertex({ cx + (r - thick * 0.5f) * s, cy - (r - thick * 0.5f) * co }, c);
        }
        t.draw(va);
        if (caps) {
            for (float a : { a0, a1 }) {
                float rad = a * 3.14159265f / 180.0f;
                disc(t, cx + r * std::sin(rad), cy - r * std::cos(rad), thick * 0.5f, c);
            }
        }
    }

    // ---- instrument panel mapping

    template <typename UI>
    struct PanelBox {
        float scale, ox, oy, vw, vh, cw, ch;
    };

    template <typename UI>
    PanelBox<UI> panelBox(UI& ui)
    {
        PanelBox<UI> b;
        ui.setWideLayout(layout.wide, layout.panel.width - 24.0f);
        b.cw = ui.panelWidth();
        b.ch = ui.panelHeight();
        // never upscale the panel: 1:1 pixels keeps the knob labels crisp
        b.scale = std::min({ 1.0f, layout.panel.width / b.cw, (layout.panel.height - 12.0f) / b.ch });
        // Snap the displayed panel to whole pixels. A fractional destination
        // size or a half-pixel sprite offset makes SFML resample the panel
        // texture, which softens every label drawn inside it.
        b.vw = std::floor(b.cw * b.scale);
        b.vh = std::floor(b.ch * b.scale);
        b.ox = std::round(layout.panel.left + (layout.panel.width - b.vw) * 0.5f);
        // the knobs sit right above the sequencer; the tape strip takes the
        // leftover height at the top of the panel
        b.oy = std::round(layout.panel.top + layout.panel.height - b.vh - 6.0f);
        return b;
    }

    template <typename UI>
    bool mapPanelCoords(UI& ui, sf::Vector2f p, sf::Vector2f& out)
    {
        PanelBox<UI> b = panelBox(ui);
        sf::FloatRect box { b.ox, b.oy, b.vw, b.vh };
        if (!box.contains(p)) return false;
        out = { (p.x - b.ox) / b.scale, UI::PANEL_TOP + (p.y - b.oy) / b.scale };
        return true;
    }

    bool toPanel(Track& tr, sf::Vector2f p, sf::Vector2f& out)
    {
        if (tr.type == ENGINE_KICK) return mapPanelCoords(*tr.kickUi, p, out);
        if (tr.type == ENGINE_ACID) return mapPanelCoords(*tr.acidUi, p, out);
        return mapPanelCoords(*tr.fm4Ui, p, out);
    }

    // Same mapping without the bounds check, so a knob drag keeps tracking
    // when the pointer leaves the panel area.
    template <typename UI>
    sf::Vector2f panelPoint(UI& ui, sf::Vector2f p)
    {
        PanelBox<UI> b = panelBox(ui);
        return { (p.x - b.ox) / b.scale, UI::PANEL_TOP + (p.y - b.oy) / b.scale };
    }

    sf::Vector2f toPanelRaw(Track& tr, sf::Vector2f p)
    {
        if (tr.type == ENGINE_KICK) return panelPoint(*tr.kickUi, p);
        if (tr.type == ENGINE_ACID) return panelPoint(*tr.acidUi, p);
        return panelPoint(*tr.fm4Ui, p);
    }

    template <typename UI>
    void drawPanelView(sf::RenderTarget& t, UI& panelUi, sf::Vector2f mouse, int key)
    {
        PanelBox<UI> b = panelBox(panelUi);
        sf::Vector2f q;
        bool inside = mapPanelCoords(panelUi, mouse, q);
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
        // repaint on change while anything is moving; otherwise the panel is
        // static, so a rare refresh is enough (CPU/fps text lives elsewhere)
        bool animating = transportPlaying.load();
        if (!animating) {
            for (auto& tp : tracks) {
                if (tp->meter.load() > 0.02f) { animating = true; break; }
            }
        }
        int refresh = animating ? 3 : 60;
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

    void drawMasterView(sf::RenderTarget& t, sf::Vector2f mouse)
    {
        // The master view owns the whole content area (no sequencer below it).
        sf::FloatRect area { layout.sidebar.width, 0, layout.W - layout.sidebar.width, layout.H };
        masterAnim += 0.02f;

        // header
        label(t, "MASTER", area.left + 36, area.top + 26, 24, Theme::text, false, true, 0.6f);
        label(t, "master bus  -  volume, delay, reverb", area.left + 38, area.top + 58, 9, Theme::muted, false, false, 0.8f);
        rrect(t, { area.left + 36, area.top + 80, 44, 2 }, 1, Theme::accent);

        const sf::Color delayCol(255, 176, 64);
        const sf::Color reverbCol(172, 128, 255);

        // three balanced cards, centred in the remaining height
        float gap = 20.0f;
        float margin = 36.0f;
        float avail = area.width - 2 * margin;
        float cardW = std::clamp((avail - 2 * gap) / 3.0f, 220.0f, 400.0f);
        float totalW = cardW * 3 + gap * 2;
        float left = area.left + (area.width - totalW) * 0.5f;
        float contentTop = area.top + 110.0f;
        float contentBottom = area.top + area.height - 36.0f;
        float cardH = std::clamp(area.height * 0.38f, 340.0f, 400.0f);
        cardH = std::min(cardH, contentBottom - contentTop);
        float top = contentTop + std::max(0.0f, (contentBottom - contentTop - cardH) * 0.5f);

        sf::FloatRect cards[3];
        for (int i = 0; i < 3; ++i)
            cards[i] = { left + i * (cardW + gap), top, cardW, cardH };

        auto card = [&](int i, const char* title, const char* sub, sf::Color col) {
            sf::FloatRect& c = cards[i];
            rrect(t, c, 12.0f, Theme::card, Theme::cardLine);
            rrect(t, { c.left + 16, c.top, c.width - 32, 3 }, 1.5f, alpha(col, 150));
            label(t, title, c.left + 20, c.top + 16, 11, col, false, true, 1.6f);
            label(t, sub, c.left + 20, c.top + 34, 8, Theme::muted, false, false, 0.6f);
            rrect(t, { c.left + 20, c.top + 56, c.width - 40, 1 }, 0, Theme::cardLine);
        };

        // Same knob as the instrument panels: disc, sweep arc, cap and pointer.
        auto knob = [&](int idx, float cx, float cy, const char* name, sf::Color col) {
            const float R = 27.0f;
            masterPanelFxRect[idx] = { cx - 40, cy - 40, 80, 80 };
            bool hot = masterPanelFxRect[idx].contains(mouse);
            float v = fxParam(idx).load();
            auto angle = [](float x) { return -135.0f + 270.0f * x; };

            disc(t, cx, cy, 36.0f, sf::Color(20, 22, 28));
            arc(t, cx, cy, R, 5.0f, -135.0f, 135.0f, Theme::track);

            float to = angle(v);
            if (std::abs(to + 135.0f) > 0.5f) {
                arc(t, cx, cy, R, hot ? 13.0f : 10.0f, -135.0f, to, alpha(col, hot ? 80 : 36), false);
                arc(t, cx, cy, R, 5.0f, -135.0f, to, col);
            }
            disc(t, cx, cy, 19.0f, hot ? sf::Color(52, 56, 70) : Theme::cardLine);
            disc(t, cx, cy, 18.0f, Theme::cap);

            float a = to * 3.14159265f / 180.0f;
            sf::VertexArray ind(sf::Lines, 2);
            ind[0] = sf::Vertex({ cx + 7.0f * std::sin(a), cy - 7.0f * std::cos(a) }, Theme::text);
            ind[1] = sf::Vertex({ cx + 16.0f * std::sin(a), cy - 16.0f * std::cos(a) }, Theme::text);
            t.draw(ind);
            disc(t, cx + 16.0f * std::sin(a), cy - 16.0f * std::cos(a), 1.6f, hot ? col : Theme::text);

            label(t, name, cx, cy + 43.0f, 12, hot ? Theme::text : sf::Color(176, 181, 196), true, false, 0.6f);
            char vbuf[16];
            snprintf(vbuf, sizeof(vbuf), "%.0f%%", v * 100.0f);
            label(t, vbuf, cx, cy + 57.0f, 11, hot ? col : Theme::muted, true, hot, 0.4f);
        };

        auto hash01 = [](int n) {
            float x = std::sin(n * 12.9898f + 4.1414f) * 43758.5453f;
            return x - std::floor(x);
        };

        // ---- output
        card(0, "OUTPUT", "master level", Theme::accent);
        {
            sf::FloatRect& c = cards[0];
            float mv = masterVolume.load();
            char pct[16];
            snprintf(pct, sizeof(pct), "%d%%", (int)std::round(mv * 100));
            label(t, pct, c.left + 20, c.top + 76, 34, Theme::text, false, true, 0.4f);
            label(t, "MASTER VOLUME", c.left + 20, c.top + 122, 8, Theme::muted, false, true, 1.4f);

            masterPanelVolRect = { c.left + 20, c.top + 146, c.width - 40, 10 };
            rrect(t, masterPanelVolRect, 5.0f, Theme::cell);
            rrect(t, { masterPanelVolRect.left, masterPanelVolRect.top, masterPanelVolRect.width * mv, masterPanelVolRect.height }, 5.0f, Theme::accent);

            label(t, "OUTPUT", c.left + 20, c.top + 190, 8, Theme::muted, false, true, 1.4f);
            masterPanelMeterRect = { c.left + 20, c.top + 210, c.width - 40, 12 };
            rrect(t, masterPanelMeterRect, 6.0f, Theme::cell);
            float lvl = std::clamp(masterMeter.load() * 0.82f, 0.0f, 1.0f);
            if (lvl > 0.01f) {
                sf::Color mc = lvl > 0.92f ? sf::Color(255, 86, 98) : (lvl > 0.7f ? sf::Color(255, 200, 60) : Theme::accent);
                rrect(t, { masterPanelMeterRect.left, masterPanelMeterRect.top, masterPanelMeterRect.width * lvl, masterPanelMeterRect.height }, 6.0f, mc);
            }
        }

        // ---- delay: an echo train whose spacing follows TIME, whose tail
        // follows FDBK and whose height follows LEVEL
        card(1, "DELAY", "master send effect", delayCol);
        {
            sf::FloatRect& c = cards[1];
            sf::FloatRect v { c.left + 20, c.top + 80, c.width - 40, 96 };
            float baseY = v.top + v.height * 0.80f;
            float time = fxDelayTime.load();
            float fb = fxDelayFeedback.load();
            float level = fxDelayLevel.load();
            int taps = 4 + (int)std::round(time * 6.0f);
            float step = v.width / (taps + 1);
            float maxH = v.height * 0.82f * (0.35f + 0.65f * level);
            int phase = taps > 0 ? (int)(masterAnim * (2.0f + 6.0f * time)) % taps : 0;

            label(t, "ECHO TAPS", v.left, c.top + 62, 7, Theme::faint, false, true, 1.2f);
            rrect(t, { v.left, baseY, v.width, 1 }, 0, sf::Color(255, 255, 255, 12));
            for (int i = 0; i < taps; ++i) {
                float decay = std::pow(std::max(0.05f, fb), i * 0.85f);
                float h = maxH * decay;
                float x = v.left + step * (i + 0.7f);
                bool hi = (i == phase);
                sf::Color col = hi ? sf::Color(255, 220, 150) : delayCol;
                if (hi) rrect(t, { x - 8, baseY - h - 3, 16, h + 6 }, 7.0f, alpha(delayCol, 26));
                rrect(t, { x - 2.5f, baseY - h, 5, h }, 2.5f, alpha(col, (int)(60 + 175 * decay)));
                disc(t, x, baseY - h, hi ? 4.0f : 2.4f, alpha(col, (int)(90 + 160 * decay)));
            }
        }

        // ---- reverb: a diffuse decaying cloud, denser and longer with SIZE
        card(2, "REVERB", "master send effect", reverbCol);
        {
            sf::FloatRect& c = cards[2];
            sf::FloatRect v { c.left + 20, c.top + 80, c.width - 40, 96 };
            float baseY = v.top + v.height * 0.80f;
            float size = fxReverbSize.load();
            float level = fxReverbLevel.load();
            float maxH = v.height * 0.82f * (0.35f + 0.65f * level);
            int bars = std::max(28, (int)v.width / 3);

            label(t, "DIFFUSE TAIL", v.left, c.top + 62, 7, Theme::faint, false, true, 1.2f);
            rrect(t, { v.left, baseY, v.width, 1 }, 0, sf::Color(255, 255, 255, 12));
            for (int i = 0; i < bars; ++i) {
                float u = bars > 1 ? i / (float)(bars - 1) : 0.0f;
                float env = std::exp(-u * (1.0f + (1.0f - size) * 5.5f));
                float rnd = hash01(i * 17 + 3);
                float shimmer = 0.78f + 0.22f * std::sin(masterAnim * 1.7f + i * 0.55f);
                float h = maxH * env * (0.30f + 0.70f * rnd) * shimmer;
                float x = v.left + i * (v.width / std::max(1, bars - 1));
                rrect(t, { x - 2.0f, baseY - h - 2, 4, h + 4 }, 2.0f, alpha(reverbCol, (int)(12 + 38 * env)));
                rrect(t, { x - 0.75f, baseY - h, 1.5f, h }, 0.75f, alpha(reverbCol, (int)(35 + 170 * env)));
                if (rnd > 0.90f) disc(t, x, baseY - h, 1.5f, alpha(reverbCol, (int)(80 + 150 * env)));
            }
        }

        // ---- knobs
        {
            float cy = cards[1].top + cards[1].height * 0.70f;
            float step = cards[1].width / 3.0f;
            knob(0, cards[1].left + step * 0.5f, cy, "TIME", delayCol);
            knob(1, cards[1].left + step * 1.5f, cy, "FDBK", delayCol);
            knob(2, cards[1].left + step * 2.5f, cy, "LEVEL", delayCol);

            float ry = cards[2].top + cards[2].height * 0.70f;
            knob(3, cards[2].left + cards[2].width * 0.30f, ry, "SIZE", reverbCol);
            knob(4, cards[2].left + cards[2].width * 0.70f, ry, "LEVEL", reverbCol);
        }

        label(t, "click a knob or scroll to adjust  -  master volume is also on the sidebar strip", area.left + 38,
            area.top + area.height - 26, 8, Theme::faint, false, false, 0.6f);
    }

    void masterPanelPressed(sf::Vector2f p, sf::Mouse::Button button, bool shift)
    {
        if (masterPanelVolRect.contains(p)) {
            float tv = std::clamp((p.x - masterPanelVolRect.left) / masterPanelVolRect.width, 0.0f, 1.0f);
            masterVolume = tv;
            dragMasterPanelVol = true;
            return;
        }
        for (int i = 0; i < FX_KNOBS; ++i) {
            if (masterPanelFxRect[i].contains(p)) {
                dragFx = (FxKnob)(FxDelayTime + i);
                dragFxStart = fxParam(i).load();
                dragFxY = p.y;
                return;
            }
        }
        (void)button; (void)shift;
    }

    void masterPanelVolDrag(sf::Vector2f p)
    {
        float tv = std::clamp((p.x - masterPanelVolRect.left) / masterPanelVolRect.width, 0.0f, 1.0f);
        masterVolume = tv;
    }

    void masterPanelWheel(sf::Vector2f p, float delta, bool shift)
    {
        if (masterPanelVolRect.contains(p)) {
            masterVolume = std::clamp(masterVolume.load() + (delta > 0 ? 0.03f : -0.03f) * (shift ? 5.0f : 1.0f), 0.0f, 1.0f);
            return;
        }
        for (int i = 0; i < FX_KNOBS; ++i) {
            if (masterPanelFxRect[i].contains(p)) {
                float v = fxParam(i).load();
                fxParam(i) = std::clamp(v + (delta > 0 ? 0.03f : -0.03f) * (shift ? 4.0f : 1.0f), 0.0f, 1.0f);
                return;
            }
        }
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

    // The knobs are pinned to the bottom of the panel area, so the sequencer
    // starts right below them. The height that leaves on top is a minimal,
    // frameless tape of the last 30 seconds of the selected track output.
    void drawTape(sf::RenderTarget& t, Track& tr)
    {
        float contentTop;
        if (tr.type == ENGINE_KICK) contentTop = panelBox(*tr.kickUi).oy;
        else if (tr.type == ENGINE_ACID) contentTop = panelBox(*tr.acidUi).oy;
        else contentTop = panelBox(*tr.fm4Ui).oy;

        float top = layout.panel.top + 8.0f;
        float h = contentTop - 8.0f - top;
        if (h < 40.0f) return;

        float fade = tr.tapeFade.load();
        if (fade < 0.005f) return;

        sf::FloatRect r { layout.panel.left + 24, top, layout.panel.width - 48, h };

        // a 5 s ruler and the zero line: no card, no frame
        for (int i = 1; i < 6; ++i)
            rrect(t, { r.left + r.width * i / 6.0f, r.top + 4.0f, 1, r.height - 8.0f }, 0, sf::Color(255, 255, 255, (sf::Uint8)(7 * fade)));
        float mid = r.top + r.height * 0.56f;
        rrect(t, { r.left, mid, r.width, 1 }, 0, sf::Color(255, 255, 255, (sf::Uint8)(14 * fade)));

        // min/max envelope over the whole tape, oldest on the left
        int cols = std::max(32, (int)r.width);
        float amp = std::min(std::max(6.0f, r.height * 0.36f), 130.0f);
        tapeRibbon.setPrimitiveType(sf::TriangleStrip);
        tapeRibbon.clear();
        for (int i = 0; i < cols; ++i) {
            int a = (int)((long long)i * TAPE_BUCKETS / cols);
            int z = (int)((long long)(i + 1) * TAPE_BUCKETS / cols);
            if (z <= a) z = a + 1;
            float lo = 0.0f, hi = 0.0f;
            for (int k = a; k < z; ++k) {
                int q = (tr.tapeWrite + k) % TAPE_BUCKETS;
                lo = std::min(lo, tr.tapeLo[q]);
                hi = std::max(hi, tr.tapeHi[q]);
            }
            sf::Color c = tr.color;
            c.a = (sf::Uint8)(130 * fade);
            float x = r.left + i;
            tapeRibbon.append(sf::Vertex({ x, mid - std::clamp(hi, -1.5f, 1.5f) * amp }, c));
            tapeRibbon.append(sf::Vertex({ x, mid - std::clamp(lo, -1.5f, 1.5f) * amp }, c));
        }
        t.draw(tapeRibbon);

        sf::Color lc = Theme::muted; lc.a = (sf::Uint8)(lc.a * fade);
        sf::Color rc = Theme::faint; rc.a = (sf::Uint8)(rc.a * fade);
        label(t, std::string(engineName(tr.type)) + "  OUTPUT", r.left, r.top, 8, lc, false, true, 1.6f);
        labelR(t, "30 s tape", r.left + r.width, r.top, 7, rc, false, 0.8f);
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
        auto loadCol = [](float lvl) {
            return lvl > 0.85f ? sf::Color(255, 86, 98)
                 : (lvl > 0.6f ? sf::Color(255, 200, 60) : Theme::accent);
        };

        // system monitor card: process CPU + fps
        cpuRect = { x + 12, 36, w - 24, 58 };
        rrect(t, cpuRect, 6.0f, Theme::card, Theme::cardLine);
        bool cpuHot = cpuRect.contains(mouse);
        label(t, "SYSTEM", cpuRect.left + 8, cpuRect.top + 4, 9, Theme::faint, false, false, 1.6f);
        label(t, "PROC", cpuRect.left + 8, cpuRect.top + 20, 9, Theme::muted, false, true, 0.8f);
        char cpubuf[16];
        snprintf(cpubuf, sizeof(cpubuf), "%.0f%%", cpuUsage.load());
        label(t, cpubuf, cpuRect.left + 44, cpuRect.top + 14, 15, Theme::text, false, true, 0.2f);
        char fpsbuf[16];
        snprintf(fpsbuf, sizeof(fpsbuf), "%.0f fps", uiFps.load());
        labelR(t, fpsbuf, cpuRect.left + cpuRect.width - 8, cpuRect.top + 10, 9, Theme::muted);
        float cpuLvl = std::clamp(cpuUsage.load() / 100.0f, 0.0f, 1.0f);
        sf::FloatRect cpuBar { cpuRect.left + 8, cpuRect.top + 42, cpuRect.width - 16, 7 };
        rrect(t, cpuBar, 3.0f, Theme::cell);
        rrect(t, { cpuBar.left, cpuBar.top, cpuBar.width * cpuLvl, cpuBar.height }, 3.0f, loadCol(cpuLvl));
        if (cpuHot) labelR(t, "details", cpuRect.left + cpuRect.width - 8, cpuRect.top + 31, 8, Theme::accent);

        // system-wise summary: average load and process RSS
        float nextY = cpuRect.top + cpuRect.height + 7.0f;
        char sysbuf[32];
        snprintf(sysbuf, sizeof(sysbuf), "SYS %.0f%%", sysCpuAvg);
        label(t, sysbuf, cpuRect.left, nextY, 10, loadCol(std::clamp(sysCpuAvg / 100.0f, 0.0f, 1.0f)), false, true, 0.4f);
        char rambuf[32];
        if (procRamMb >= 1024.0f)
            snprintf(rambuf, sizeof(rambuf), "RAM %.1f GB", procRamMb / 1024.0f);
        else
            snprintf(rambuf, sizeof(rambuf), "RAM %.0f MB", procRamMb);
        labelR(t, rambuf, cpuRect.left + cpuRect.width, nextY, 10, Theme::muted, true, 0.2f);
        nextY += 16.0f;

        // system-wide per-core load strip: fill the width, max 8 per row
        if (coreSampled && !coreCpuPct.empty()) {
            int cores = (int)coreCpuPct.size();
            char corebuf[32];
            snprintf(corebuf, sizeof(corebuf), "%d CORES", cores);
            label(t, corebuf, cpuRect.left, nextY, 9, Theme::faint, false, false, 1.0f);
            nextY += 14.0f;

            int perRow = std::min(cores, 8);
            int rows = (cores + perRow - 1) / perRow;
            const float gap = 5.0f;
            const float cellH = 14.0f;
            const float cellW = (cpuRect.width - gap * (perRow - 1)) / perRow;
            for (int i = 0; i < cores; ++i) {
                int row = i / perRow, col = i % perRow;
                sf::FloatRect r { cpuRect.left + col * (cellW + gap), nextY + row * (cellH + gap), cellW, cellH };
                rrect(t, r, 3.0f, Theme::cell);
                float lvl = std::clamp(coreCpuPct[i] / 100.0f, 0.0f, 1.0f);
                if (lvl > 0.005f)
                    rrect(t, { r.left, r.top, r.width * lvl, r.height }, 3.0f, loadCol(lvl));
            }
            nextY += rows * (cellH + gap) + 4.0f;
        }
        label(t, "click for thread details", cpuRect.left, nextY, 8, Theme::faint, false, false, 0.4f);
        nextY += 13.0f;

        // master strip, just above track 1 and styled like a track row
        float rowH = 50.0f;
        float y = nextY + 4.0f;
        {
            bool mv = masterView.load();
            sf::FloatRect row { x + 8, y, w - 16, rowH - 4 };
            masterViewBtnRect = row;
            bool hot = row.contains(mouse);
            rrect(t, row, 7.0f, mv ? sf::Color(30, 34, 44) : (hot ? sf::Color(24, 27, 34) : Theme::card), mv ? Theme::accent : Theme::cardLine);

            // small mixer glyph instead of a track number, so the row matches
            // the track strips without a second "MASTER" label colliding
            for (int k = 0; k < 3; ++k) {
                float lx = row.left + 18 + k * 5.0f;
                rrect(t, { lx, row.top + 7, 1.5f, 13 }, 0.75f, alpha(Theme::accent, 170));
                float ky = row.top + 9 + k * 3.0f;
                rrect(t, { lx - 1.75f, ky, 5, 3 }, 1.5f, mv ? Theme::accent : Theme::muted);
            }

            // same volume bar as the track rows, scroll to change it
            sf::FloatRect vr { row.left + 40, row.top + 8, row.width - 90, 6 };
            masterVolRect = vr;
            rrect(t, vr, 3.0f, Theme::cell);
            float mvv = masterVolume.load();
            rrect(t, { vr.left, vr.top, vr.width * mvv, vr.height }, 3.0f, Theme::accent);

            char mb[32];
            snprintf(mb, sizeof(mb), "MASTER BUS  %d%%", (int)std::round(mvv * 100));
            label(t, mb, row.left + 18, row.top + 25, 9, Theme::muted, false, true, 0.8f);

            // drill-in pill, mirrors the M/S pills on the track rows
            sf::FloatRect fx { row.left + row.width - 46, row.top + 5, 42, 16 };
            rrect(t, fx, 4.0f, mv ? Theme::accent : Theme::cell, Theme::cardLine);
            label(t, "FX", fx.left + fx.width * 0.5f, fx.top + 3, 8, mv ? sf::Color(12, 14, 16) : Theme::muted, true, true, 0.8f);

            // output meter hugging the row edge, like the track activity meters
            // (drawn as a live overlay in drawLiveMeters)
            if (!cachingChrome) {
                float lvl = std::clamp(masterMeter.load(), 0.0f, 1.0f);
                if (lvl > 0.02f)
                    rrect(t, { row.left + row.width - 2, row.top + 4, 2, row.height - 8 }, 1.0f, Theme::accent);
            }
        }

        // track list, right under the master strip
        y += rowH;
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

            // activity meter hugging the row edge (live overlay in drawLiveMeters)
            if (!cachingChrome) {
                float lvl = std::clamp(tr.meter.load(), 0.0f, 1.0f);
                if (!tr.mute && lvl > 0.02f)
                    rrect(t, { row.left + row.width - 2, row.top + 4, 2, row.height - 8 }, 1.0f, sel ? tr.color : Theme::accent);
            }
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

        // master volume and FX live in the master strip / master view
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

        if (playBtnRect.contains(p)) { transportPlaying = !transportPlaying.load(); return; }
        if (bpmMinusRect.contains(p)) { transportBpm = std::max(60.0f, transportBpm.load() - (shift ? 10.0f : 1.0f)); return; }
        if (bpmPlusRect.contains(p)) { transportBpm = std::min(300.0f, transportBpm.load() + (shift ? 10.0f : 1.0f)); return; }
        if (bpmBoxRect.contains(p)) { transportBpm = std::clamp(transportBpm.load() + (shift ? 10.0f : 1.0f), 60.0f, 300.0f); return; }

        if (masterViewBtnRect.contains(p)) { masterView = true; return; }
        for (int i = 0; i < TRACK_COUNT; ++i) {
            if (trackMuteRect[i].contains(p)) { tracks[i]->mute = !tracks[i]->mute; return; }
            if (trackSoloRect[i].contains(p)) { tracks[i]->solo = !tracks[i]->solo; return; }
            if (trackVolRect[i].contains(p)) {
                dragSidebarVol = i;
                float tv = std::clamp((p.x - trackVolRect[i].left) / trackVolRect[i].width, 0.0f, 1.0f);
                tracks[i]->volume = tv;
                return;
            }
            if (trackRowRect[i].contains(p)) { selectedTrack = i; masterView = false; return; }
        }
        if (masterVolRect.contains(p)) {
            float tv = std::clamp((p.x - masterVolRect.left) / masterVolRect.width, 0.0f, 1.0f);
            masterVolume = tv;
            dragMasterVol = true;
            return;
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
        float tv = std::clamp((p.x - masterVolRect.left) / masterVolRect.width, 0.0f, 1.0f);
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
        if (masterViewBtnRect.contains(p)) {
            masterVolume = std::clamp(masterVolume.load() + (delta > 0 ? 0.03f : -0.03f) * (shift ? 5.0f : 1.0f), 0.0f, 1.0f);
            return;
        }
        if (bpmBoxRect.contains(p) || bpmMinusRect.contains(p) || bpmPlusRect.contains(p)) {
            transportBpm = std::clamp(transportBpm.load() + (delta > 0 ? 1.0f : -1.0f) * (shift ? 10.0f : 1.0f), 60.0f, 300.0f);
            return;
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

    // ---------------------------------------------------------------- synth piano roll
    // Shared grid (Acid32 look) + Fm4 sequencer interaction model. It is
    // templated over AcidSequencer/Fm4Sequencer, whose step structs share the
    // same fields (on/note/velocity/accent/slide/length).
    struct SynthSeqState { int page = 0; int edit = -1; int base = 30; };
    SynthSeqState synthSeqState[TRACK_COUNT];

    struct SynthGeom {
        sf::FloatRect card, page0, page1, clear, editNote, editLen, editVel, editSlide, editAccent, grid, scroll;
        float cellW, cellH;
    };

    SynthGeom synthGeom()
    {
        SynthGeom g;
        sf::FloatRect pr = layout.seq;
        g.card = pr;
        g.page0 = { pr.left + 150.0f, pr.top + 8.0f, 30.0f, 18.0f };
        g.page1 = { pr.left + 184.0f, pr.top + 8.0f, 30.0f, 18.0f };
        g.clear = { pr.left + pr.width - 54.0f, pr.top + 8.0f, 42.0f, 18.0f };
        float ew = (pr.width - 24.0f - 4 * 6.0f) / 5.0f;
        float ex = pr.left + 12.0f, ey = pr.top + 31.0f;
        g.editNote = { ex, ey, ew, 18 }; ex += ew + 6;
        g.editLen = { ex, ey, ew, 18 }; ex += ew + 6;
        g.editVel = { ex, ey, ew, 18 }; ex += ew + 6;
        g.editSlide = { ex, ey, ew, 18 }; ex += ew + 6;
        g.editAccent = { ex, ey, ew, 18 };
        g.grid = { pr.left + 54.0f, pr.top + 66.0f, pr.width - 74.0f, pr.height - 94.0f };
        g.scroll = { pr.left + pr.width - 12.0f, g.grid.top, 8.0f, g.grid.height };
        g.cellW = g.grid.width / 16.0f;
        g.cellH = g.grid.height / 25.0f;
        return g;
    }

    template <typename Seq>
    static int& synthPatternLength(Seq& seq) { return seq.patternLength; }

    template <typename Seq>
    void synthResetPage(Seq& seq, SynthSeqState& st)
    {
        if (synthPatternLength(seq) <= 16) st.page = 0;
    }

    static bool synthBlackKey(int midi)
    {
        int k = ((midi % 12) + 12) % 12;
        return k == 1 || k == 3 || k == 6 || k == 8 || k == 10;
    }

    template <typename Seq>
    void drawSynthSeq(sf::RenderTarget& t, Seq& seq, Track& tr, sf::Vector2f mouse)
    {
        SynthGeom g = synthGeom();
        SynthSeqState& st = synthSeqState[tr.index];
        synthResetPage(seq, st);
        const sf::Color noteCol(230, 255, 40);
        const sf::Color accentCol(255, 176, 64);
        const sf::Color slideCol(56, 212, 255);

        rrect(t, g.card, 0, sf::Color(14, 16, 21));
        rrect(t, { g.card.left + 12, g.card.top, g.card.width - 24, 3 }, 1.5f, alpha(noteCol, 110));

        label(t, "SEQUENCER", g.card.left + 12, g.card.top + 7, 10, Theme::text, false, true, 1.8f);
        char info[64];
        snprintf(info, sizeof(info), "%s  -  %d STEPS", engineName(tr.type), synthPatternLength(seq));
        label(t, info, g.card.left + 90, g.card.top + 9, 7, Theme::muted, false, false, 0.8f);

        auto tab = [&](sf::FloatRect r, const char* s, bool active) {
            bool hot = r.contains(mouse);
            rrect(t, r, 6.0f, active ? noteCol : (hot ? sf::Color(40, 43, 54) : Theme::card), Theme::cardLine);
            label(t, s, r.left + r.width * 0.5f, r.top + 4, 8, active ? sf::Color(14, 14, 18) : Theme::muted, true, true, 0.6f);
        };
        tab(g.page0, "16", st.page == 0);
        tab(g.page1, "32", st.page == 1);
        rrect(t, g.clear, 6.0f, Theme::card, Theme::cardLine);
        label(t, "CLEAR", g.clear.left + g.clear.width * 0.5f, g.clear.top + 5, 7, Theme::text, true, true, 0.6f);

        // edit strip
        auto editBox = [&](sf::FloatRect r, const char* s, bool active, sf::Color acc) {
            bool hot = r.contains(mouse);
            rrect(t, r, 6.0f, active ? acc : (hot ? sf::Color(40, 43, 54) : Theme::cell), Theme::cardLine);
            label(t, s, r.left + r.width * 0.5f, r.top + 4, 7, active ? sf::Color(12, 12, 14) : Theme::text, true, true, 0.4f);
        };
        if (st.edit >= 0 && st.edit < synthPatternLength(seq)) {
            auto& s = seq.steps[st.edit];
            char b[24];
            snprintf(b, sizeof(b), "%s", getMidiNoteName(s.note));
            editBox(g.editNote, b, s.on, tr.color);
            snprintf(b, sizeof(b), "L %.2f", s.length);
            editBox(g.editLen, b, false, noteCol);
            snprintf(b, sizeof(b), "V %d", (int)std::lround(s.velocity * 100.0f));
            editBox(g.editVel, b, false, noteCol);
            editBox(g.editSlide, "SLIDE", s.slide, slideCol);
            editBox(g.editAccent, "ACC", s.accent, accentCol);
        } else {
            editBox(g.editNote, "NOTE", false, noteCol);
            editBox(g.editLen, "LEN", false, noteCol);
            editBox(g.editVel, "VEL", false, noteCol);
            editBox(g.editSlide, "SLIDE", false, slideCol);
            editBox(g.editAccent, "ACC", false, accentCol);
        }

        // grid (Acid32 look)
        rrect(t, g.grid, 8.0f, sf::Color(13, 14, 18), Theme::cardLine);
        for (int row = 0; row < 25; ++row) {
            int midi = st.base + (25 - 1 - row);
            float y = g.grid.top + row * g.cellH;
            if (synthBlackKey(midi)) {
                sf::RectangleShape bg({ g.grid.width, g.cellH });
                bg.setPosition(g.grid.left, y);
                bg.setFillColor(sf::Color(18, 19, 25));
                t.draw(bg);
            }
            bool isC = (midi % 12) == 0;
            rrect(t, { g.grid.left, y + g.cellH, g.grid.width, 1 }, 0, isC ? sf::Color(255, 255, 255, 25) : sf::Color(255, 255, 255, 8));
            label(t, getMidiNoteName(midi), g.grid.left - 6, y + g.cellH * 0.5f - 4, isC ? 8 : 7,
                  isC ? noteCol : (synthBlackKey(midi) ? Theme::faint : Theme::muted), true, isC);
        }
        for (int col = 0; col < 16; ++col) {
            float x = g.grid.left + col * g.cellW;
            bool down = (col % 4) == 0;
            rrect(t, { x, g.grid.top, 1, g.grid.height }, 0, down ? sf::Color(255, 255, 255, 25) : sf::Color(255, 255, 255, 8));
        }

        // notes
        int startStep = st.page * 16;
        for (int i = 0; i < synthPatternLength(seq); ++i) {
            const auto& s = seq.steps[i];
            if (!s.on) continue;
            float col = (float)(i - startStep);
            if (col + s.length <= 0.0f || col >= 16.0f) continue;
            int off = (int)s.note - st.base;
            if (off < 0 || off >= 25) continue;
            float row = (float)(25 - 1 - off);
            float nx = g.grid.left + col * g.cellW + 1.0f;
            float ny = g.grid.top + row * g.cellH + 1.0f;
            float nw = std::max(6.0f, s.length * g.cellW - 2.0f);
            float nh = std::max(4.0f, g.cellH - 2.0f);
            if (nx < g.grid.left) { nw -= (g.grid.left - nx); nx = g.grid.left; }
            if (nx + nw > g.grid.left + g.grid.width) nw = (g.grid.left + g.grid.width) - nx;
            sf::Color c = s.accent ? accentCol : (s.slide ? slideCol : noteCol);
            rrect(t, { nx, ny, nw, nh }, 3.0f, alpha(c, 70));
            float vh = std::clamp(nh * s.velocity, 2.0f, nh);
            rrect(t, { nx, ny + (nh - vh), nw, vh }, 3.0f, c);
            bool sel = (st.edit == i);
            rrect(t, { nx, ny, nw, nh }, 3.0f, sf::Color::Transparent, sel ? sf::Color::White : alpha(c, 180));
            if (s.slide && nw > 16.0f)
                label(t, ">", nx + nw - 7, ny + nh * 0.5f - 5, 8, sf::Color(14, 14, 18), true, true);
        }

        // scrollbar
        rrect(t, g.scroll, 4.0f, sf::Color(20, 22, 28), Theme::cardLine);
        int total = 128, view = 25;
        int scrollable = std::max(1, total - view);
        float th = std::max(18.0f, g.scroll.height * view / (float)total);
        float tp = std::clamp((127 - view - st.base) / (float)scrollable, 0.0f, 1.0f);
        rrect(t, { g.scroll.left, g.scroll.top + tp * (g.scroll.height - th), g.scroll.width, th }, 4.0f, noteCol, noteCol);
        for (int n = 0; n <= 120; n += 12) {
            float ty = g.scroll.top + (127 - view - n) / (float)scrollable * (g.scroll.height - th) + th * 0.5f;
            rrect(t, { g.scroll.left - 2, ty, 4, 1 }, 0, sf::Color(255, 255, 255, 30));
        }
    }

    template <typename Seq>
    int synthStepAt(Seq& seq, SynthGeom& g, SynthSeqState& st, sf::Vector2f p, int& noteOut)
    {
        if (!g.grid.contains(p)) return -1;
        float cellW = g.grid.width / 16.0f;
        float cellH = g.grid.height / 25.0f;
        int col = std::clamp((int)((p.x - g.grid.left) / cellW), 0, 15);
        int step = st.page * 16 + col;
        if (step < 0 || step >= synthPatternLength(seq)) return -1;
        int fromBottom = (int)((g.grid.top + g.grid.height - p.y) / cellH);
        noteOut = std::clamp(st.base + fromBottom, 0, 127);

        // find an existing note rectangle under the cursor
        float clickCol = (p.x - g.grid.left) / cellW;
        for (int i = 0; i < synthPatternLength(seq); ++i) {
            const auto& s = seq.steps[i];
            if (!s.on || s.note != noteOut) continue;
            float s0 = (float)(i - st.page * 16);
            if (i / 16 == st.page && clickCol >= s0 && clickCol < s0 + s.length) return i;
        }
        return step;
    }

    template <typename Seq>
    bool synthNoteUnder(Seq& seq, SynthGeom& g, SynthSeqState& st, sf::Vector2f p, int& found)
    {
        if (!g.grid.contains(p)) return false;
        float cellW = g.grid.width / 16.0f;
        float cellH = g.grid.height / 25.0f;
        int fromBottom = (int)((g.grid.top + g.grid.height - p.y) / cellH);
        int note = std::clamp(st.base + fromBottom, 0, 127);
        float clickCol = (p.x - g.grid.left) / cellW;
        for (int i = 0; i < synthPatternLength(seq); ++i) {
            const auto& s = seq.steps[i];
            if (!s.on || s.note != note) continue;
            float s0 = (float)(i - st.page * 16);
            if (i / 16 == st.page && clickCol >= s0 && clickCol < s0 + s.length) { found = i; return true; }
        }
        return false;
    }

    template <typename Seq>
    void synthSeqPressed(Seq& seq, Track& tr, sf::Vector2f p, sf::Mouse::Button button, bool shift)
    {
        std::lock_guard<std::mutex> lock(audioMutex);
        SynthGeom g = synthGeom();
        SynthSeqState& st = synthSeqState[tr.index];
        synthResetPage(seq, st);

        if (button == sf::Mouse::Left) {
            if (g.page0.contains(p)) { st.page = 0; synthPatternLength(seq) = 16; return; }
            if (g.page1.contains(p)) { st.page = 1; synthPatternLength(seq) = 32; return; }
            if (g.clear.contains(p)) { for (int i = 0; i < 32; ++i) seq.steps[i].on = false; return; }
            if (g.scroll.contains(p)) { synthDragScroll = true; synthDragTrack = tr.index; return; }
            if (st.edit >= 0 && st.edit < synthPatternLength(seq)) {
                auto& s = seq.steps[st.edit];
                if (g.editSlide.contains(p)) { s.slide = !s.slide; return; }
                if (g.editAccent.contains(p)) { s.accent = !s.accent; return; }
            }
            int note = 0;
            int step = synthStepAt(seq, g, st, p, note);
            if (step >= 0) {
                int found = -1;
                if (synthNoteUnder(seq, g, st, p, found)) {
                    if (shift) { seq.steps[found].slide = !seq.steps[found].slide; return; }
                    st.edit = found;
                    synthDragStep = found;
                    synthDragTrack = tr.index;
                    float cellW = g.grid.width / 16.0f;
                    float rightX = g.grid.left + (found % 16 + seq.steps[found].length) * cellW;
                    synthDragLen = std::abs(p.x - rightX) < 12.0f;
                    if (!synthDragLen) auditionNote.store(seq.steps[found].note);
                } else {
                    int idx = st.page * 16 + (int)((p.x - g.grid.left) / g.cellW);
                    if (idx >= 0 && idx < 32) {
                        auto& s = seq.steps[idx];
                        s.on = true;
                        s.note = (uint8_t)note;
                        s.length = 0.85f;
                        s.velocity = 0.8f;
                        st.edit = idx;
                        synthDragStep = idx;
                        synthDragTrack = tr.index;
                        synthDragLen = false;
                        auditionNote.store(note);
                    }
                }
            }
        } else if (button == sf::Mouse::Right) {
            int found = 0;
            if (synthNoteUnder(seq, g, st, p, found)) {
                if (shift) seq.steps[found].accent = !seq.steps[found].accent;
                else { seq.steps[found].on = false; if (st.edit == found) st.edit = -1; }
            }
        }
    }

    template <typename Seq>
    void synthSeqMoved(Seq& seq, Track& tr, sf::Vector2f p, bool shift)
    {
        std::lock_guard<std::mutex> lock(audioMutex);
        SynthGeom g = synthGeom();
        SynthSeqState& st = synthSeqState[tr.index];
        (void)shift;
        if (synthDragScroll) {
            float t = std::clamp((p.y - g.scroll.top) / g.scroll.height, 0.0f, 1.0f);
            st.base = std::clamp(127 - 25 - (int)(t * (128 - 25)), 0, 127 - 25);
            return;
        }
        if (synthDragStep >= 0 && synthDragStep < synthPatternLength(seq)) {
            auto& s = seq.steps[synthDragStep];
            float cellW = g.grid.width / 16.0f;
            if (synthDragLen) {
                float leftX = g.grid.left + (synthDragStep % 16) * cellW;
                s.length = std::clamp((p.x - leftX) / cellW, 0.2f, 8.0f);
            } else {
                int fromBottom = (int)((g.grid.top + g.grid.height - p.y) / g.cellH);
                int note = std::clamp(st.base + fromBottom, 0, 127);
                if (s.note != note) { s.note = (uint8_t)note; auditionNote.store(note); }
            }
        }
    }

    void synthSeqReleased()
    {
        synthDragStep = -1;
        synthDragLen = false;
        synthDragScroll = false;
        synthDragTrack = -1;
    }

    template <typename Seq>
    void synthSeqWheel(Seq& seq, Track& tr, sf::Vector2f p, float delta, bool shift, bool alt)
    {
        std::lock_guard<std::mutex> lock(audioMutex);
        SynthGeom g = synthGeom();
        SynthSeqState& st = synthSeqState[tr.index];
        int dir = delta > 0 ? 1 : -1;

        if (st.edit >= 0 && st.edit < synthPatternLength(seq)) {
            auto& s = seq.steps[st.edit];
            if (g.editNote.contains(p)) { s.note = (uint8_t)std::clamp((int)s.note + dir * (shift ? 12 : 1), 0, 127); auditionNote.store(s.note); return; }
            if (g.editLen.contains(p)) { s.length = std::clamp(s.length + dir * 0.1f, 0.1f, 8.0f); return; }
            if (g.editVel.contains(p)) { s.velocity = std::clamp(s.velocity + dir * 0.05f, 0.05f, 1.0f); return; }
            if (g.editSlide.contains(p)) { s.slide = !s.slide; return; }
            if (g.editAccent.contains(p)) { s.accent = !s.accent; return; }
        }
        if (g.scroll.contains(p)) {
            st.base = std::clamp(st.base + dir * (shift ? 12 : 1), 0, 127 - 25);
            return;
        }
        if (g.grid.contains(p)) {
            int found = 0;
            if (synthNoteUnder(seq, g, st, p, found)) {
                auto& s = seq.steps[found];
                st.edit = found;
                if (alt) s.slide = !s.slide;
                else if (shift) s.length = std::clamp(s.length + dir * 0.1f, 0.1f, 8.0f);
                else s.velocity = std::clamp(s.velocity + dir * 0.05f, 0.05f, 1.0f);
                return;
            }
            st.base = std::clamp(st.base + dir * (shift ? 12 : 1), 0, 127 - 25);
        }
    }

    template <typename Seq>
    void synthSeqKey(Seq& seq, Track& tr, sf::Keyboard::Key k)
    {
        std::lock_guard<std::mutex> lock(audioMutex);
        SynthSeqState& st = synthSeqState[tr.index];
        if (k == sf::Keyboard::Tab) {
            st.page = 1 - st.page;
            synthPatternLength(seq) = (st.page + 1) * 16;
            return;
        }
        if (st.edit >= 0 && st.edit < synthPatternLength(seq)) {
            auto& s = seq.steps[st.edit];
            if (k == sf::Keyboard::Up) { s.note = (uint8_t)std::clamp((int)s.note + 1, 0, 127); auditionNote.store(s.note); }
            else if (k == sf::Keyboard::Down) { s.note = (uint8_t)std::clamp((int)s.note - 1, 0, 127); auditionNote.store(s.note); }
            else if (k == sf::Keyboard::Left) s.length = std::clamp(s.length - 0.1f, 0.1f, 8.0f);
            else if (k == sf::Keyboard::Right) s.length = std::clamp(s.length + 0.1f, 0.1f, 8.0f);
        }
    }


    void drawSequencer(sf::RenderTarget& t, Track& tr, sf::Vector2f mouse)
    {
        const sf::FloatRect card = layout.seq;
        // both synths share the Acid32 piano-roll grid and the Fm4 edit model
        if (tr.type == ENGINE_ACID) { drawSynthSeq(t, *tr.acidSeq, tr, mouse); return; }
        if (tr.type == ENGINE_FM4) { drawSynthSeq(t, *tr.fm4Seq, tr, mouse); return; }
        rrect(t, card, 0, sf::Color(14, 16, 21));

        int steps = Sequencer::STEPS;

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
                    bool hot = r.contains(mouse);
                    sf::Color fill = on ? tr.color : (hot ? sf::Color(34, 38, 47) : Theme::cell);
                    rrect(t, r, 5.0f, fill, on ? sf::Color(0, 0, 0, 40) : sf::Color::Transparent);
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
        Track& tr = *tracks[selectedTrack.load()];
        if (tr.type == ENGINE_ACID) { synthSeqPressed(*tr.acidSeq, tr, p, button, shift); return; }
        if (tr.type == ENGINE_FM4) { synthSeqPressed(*tr.fm4Seq, tr, p, button, shift); return; }
        std::lock_guard<std::mutex> lock(audioMutex);
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
        Track& tr = *tracks[selectedTrack.load()];
        bool alt = sf::Keyboard::isKeyPressed(sf::Keyboard::LAlt) || sf::Keyboard::isKeyPressed(sf::Keyboard::RAlt);
        if (tr.type == ENGINE_ACID) { synthSeqWheel(*tr.acidSeq, tr, p, delta, shift, alt); return; }
        if (tr.type == ENGINE_FM4) { synthSeqWheel(*tr.fm4Seq, tr, p, delta, shift, alt); return; }
        std::lock_guard<std::mutex> lock(audioMutex);
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
    o << "masterView " << (masterView.load() ? 1 : 0) << "\n";
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
        else if (key == "masterView") { int v; if (l >> v) masterView = (v != 0); }
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
    prctl(PR_SET_NAME, (unsigned long)"zic-ui", 0, 0, 0);
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

    std::vector<std::thread> audioWorkers;
    audioWorkers.reserve(AUDIO_WORKERS);
    for (int i = 0; i < AUDIO_WORKERS; ++i) audioWorkers.emplace_back(audioTrackWorker, i);
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
    if (std::getenv("ZIC_MONITOR")) ui.openMonitor();

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
    ui.layout.seqRight = false;
    ui.layout.split = seqSplit;
    fitView(window.getSize().x, window.getSize().y);
    auto logical = [&](int x, int y) { return window.mapPixelToCoords({ x, y }); };

    if (demo) {
        for (int i = 0; i < TRACK_COUNT; ++i) tracks[i]->type = i % ENGINE_COUNT;
        if (!std::getenv("ZIC_TRACK")) selectedTrack = 0;
        transportPlaying = true;
    }
    if (std::getenv("ZIC_PLAY")) transportPlaying = true;

    int frame = 0;
    bool shotDone = false;
    auto lastDraw = std::chrono::steady_clock::now();
    auto lastAutosave = lastDraw;
    auto fpsWall = lastDraw;
    int fpsFrames = 0;
    while (window.isOpen()) {
        // full rate while the transport runs, relaxed when idle
        window.setFramerateLimit(transportPlaying.load() ? 20 : 6);
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
                if (e.key.code == sf::Keyboard::Escape) {
                    if (ui.monitorOpen()) ui.closeMonitor();
                    else window.close();
                } else if (!screenshot) {
                    ui.key(e.key.code);
                }
                break;
            default:
                break;
            }
        }

        auto autosaveNow = std::chrono::steady_clock::now();
        if (!screenshot && std::chrono::duration<double>(autosaveNow - lastAutosave).count() >= 1.0) {
            lastAutosave = autosaveNow;
            auto wp = window.getPosition();
            winX = wp.x;
            winY = wp.y;
            winW = (int)window.getSize().x;
            winH = (int)window.getSize().y;
            static std::string last;
            std::string now = serializeProject();
            if (now != last && Persist::save(stateFile, now)) last = now;
        }

        // cpu usage (process, smoothed)
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

        ui.sampleSystem();

        auto mp = sf::Mouse::getPosition(window);
        sf::Vector2f uiMouse = screenshot ? sf::Vector2f(-10000.0f, -10000.0f) : logical(mp.x, mp.y);

        // Skip the whole frame when nothing is animating and no input changed.
        // A periodic redraw keeps the CPU/fps text and window contents fresh.
        bool animate = transportPlaying.load() || masterMeter.load() > 0.02f;
        if (!animate) {
            for (auto& t : tracks) {
                if (t->meter.load() > 0.02f || t->tapeFading.load()) { animate = true; break; }
            }
        }
        auto drawNow = std::chrono::steady_clock::now();
        bool forced = screenshot || std::chrono::duration<double>(drawNow - lastDraw).count() >= 1.0;
        if (screenshot || animate || ui.wantsRedraw() || forced) {
            ui.draw(window, uiMouse);
            window.display();
            lastDraw = drawNow;
            ++fpsFrames;
            double fpsDt = std::chrono::duration<double>(drawNow - fpsWall).count();
            if (fpsDt > 0.5) {
                uiFps = (float)(fpsFrames / fpsDt);
                fpsFrames = 0;
                fpsWall = drawNow;
            }
        } else {
            // No display() call means setFramerateLimit() would not throttle us.
            std::this_thread::sleep_for(std::chrono::milliseconds(20));
        }

        if (screenshot) {
            if (++frame == (demo ? 120 : 5)) {
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
    jobCv.notify_all();
    jobDoneCv.notify_all();
    audioThread.join();
    for (auto& th : audioWorkers) th.join();
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
