#pragma once

#ifndef AUDIO_FOLDER
#include "host/constants.h"
#endif

#include "audio/effects/applyDrive.h"
#include "audio/engines/EngineBase.h"
#include "audio/filterSVF.h"
#include "audio/utils/applySampleGain.h"
#include "audio/utils/math.h"
#include "helpers/clamp.h"

#include <sndfile.h>

#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <cstring>
#include <dirent.h>
#include <string>
#include <vector>

// ─────────────────────────────────────────────────────────────────────────────
// Sample23  —  chromatic sample player
//
// NOTE MAPPING
//   N files occupy exactly N consecutive MIDI notes, centered on C4 (MIDI 60).
//   Example: 12 files → MIDI 54–65.  All other notes produce silence.
//   No pitch-shifting to fill gaps — each note triggers exactly one file.
//   Pitch Shift: if On, playback rate is adjusted so the sample sounds at
//   the played note's pitch relative to its root note.
//
// RANDOMISER
//   Rnd Mode = Any:      any file in the pack, at its native pitch.
//   Rnd Mode = Adjacent: ±1 file from the mapped one, at its native pitch.
//
// LOOP
//   Loop Length = 0 → one-shot; note-off has no effect.
//   Loop Length > 0:
//     • Sample plays from the beginning toward loopStart.
//     • Once loopStart is reached the voice enters loop mode.
//     • It loops [loopStart, loopStart+loopLength) until note-off.
//     • If note-off arrives before loopStart, the loop is skipped and the
//       sample plays straight to the end.
//     • After note-off the voice exits the loop and plays to the end.
//
// GRANULAR  (density = 0 → plain playback)
//   Density > 0 fires short grains from the read position.
//   Each grain is windowed with a Hann envelope (click-free).
//   Grain Size   ms    duration of each grain
//   Density      g/s   grains per second (0–10)
//   Spread       %     random scatter of grain start position
//   Grain Delay  ms    delay before cloud begins after note-on
//
// PACK SWITCHING
//   New pack loads into pendingSlots[].  Audio thread commits once all
//   active voices have finished (MAX_FADE_SAMPLES grace period).
// ─────────────────────────────────────────────────────────────────────────────

class Sample23 : public EngineBase<Sample23> {

public:
    static constexpr int MAX_SAMPLES = 64;
    static constexpr int MAX_VOICES = 8;
    static constexpr int MAX_GRAINS = 32;
    static constexpr int DELAY_BUF_SIZE = 48000;
    static constexpr int REVERB_BUF_SIZE = 16384;
    static constexpr int MAX_FADE_SAMPLES = 2048; // ~43 ms @ 48 kHz

    // ── Sample slot ───────────────────────────────────────────────────────────
    struct SampleSlot {
        float* data = nullptr;
        uint64_t frameCount = 0;
        int rootNote = 60;
        bool loaded = false;

        void free()
        {
            delete[] data;
            data = nullptr;
            frameCount = 0;
            loaded = false;
        }
    };

    // ── Grain sub-voice ───────────────────────────────────────────────────────
    struct Grain {
        bool active = false;
        double pos = 0.0;
        double rate = 1.0;
        uint64_t endFrame = 0;
        float sizeFrames = 0.0f; // for Hann window calculation
    };

    // ── Playback voice ────────────────────────────────────────────────────────
    struct Voice {
        bool active = false;
        int slotIdx = 0;
        double pos = 0.0; // main read head
        double rate = 1.0;
        uint8_t midiNote = 60;
        float velocity = 1.0f;

        // Loop
        bool looping = false; // loop region was configured
        bool inLoop = false; // read head has passed loopStart
        bool releasing = false; // note-off received while in loop
        uint64_t loopStart = 0;
        uint64_t loopEnd = 0; // exclusive

        // Granular
        bool granular = false;
        Grain grains[MAX_GRAINS];
        float grainTimer = 0.0f; // samples until next grain spawn
        float grainDelayCnt = 0.0f; // samples of initial delay remaining
    };

protected:
    const float sampleRate;
    const float sampleRateDiv;

    SampleSlot slots[MAX_SAMPLES];
    int loadedCount = 0;

    // Pending pack (double-buffer swap)
    SampleSlot pendingSlots[MAX_SAMPLES];
    int pendingCount = 0;
    bool swapPending = false;
    int swapCountdown = 0;

    std::vector<std::string> packNames;
    int currentPack = -1;

    Voice voices[MAX_VOICES];
    FilterSVF svfFilter;

    float* delayBuf = nullptr;
    int delayWrite = 0;
    float dlyFbSmooth = 0.0f;

    float* reverbBuf = nullptr;
    static constexpr int COMB_LEN[4] = { 1559, 1617, 1685, 1751 };
    static constexpr int AP_LEN[3] = { 347, 113, 37 };
    int combOff[4] = {};
    int apOff[3] = {};
    int combIdx[8] = {};
    int apIdx[4] = {};
    float combFb[8] = {};

    // ─────────────────────────────────────────────────────────────────────────
    // Helpers
    // ─────────────────────────────────────────────────────────────────────────

    static float lerp(float a, float b, float t) { return a + t * (b - a); }

    float slotRead(const SampleSlot& slot, double pos) const
    {
        if (!slot.loaded || pos < 0.0) return 0.0f;
        uint64_t i0 = (uint64_t)pos;
        if (i0 + 1 >= slot.frameCount) return 0.0f;
        float frac = (float)(pos - (double)i0);
        return slot.data[i0] + frac * (slot.data[i0 + 1] - slot.data[i0]);
    }

    // Returns slot index for a MIDI note, or -1 if out of the mapped range.
    int noteToSlotIndex(uint8_t note) const
    {
        if (loadedCount == 0) return -1;
        int first = 60 - loadedCount / 2;
        int last = first + loadedCount - 1;
        if ((int)note < first || (int)note > last) return -1;
        return (int)note - first;
    }

    void assignRootNotes(SampleSlot* s, int count)
    {
        int anchor = count / 2;
        for (int i = 0; i < count; ++i)
            s[i].rootNote = 60 + (i - anchor);
    }

    int allocVoice()
    {
        for (int i = 0; i < MAX_VOICES; ++i)
            if (!voices[i].active) return i;
        int oldest = 0;
        double maxPos = -1.0;
        for (int i = 0; i < MAX_VOICES; ++i)
            if (voices[i].pos > maxPos) {
                maxPos = voices[i].pos;
                oldest = i;
            }
        return oldest;
    }

    // ── Pack loading ──────────────────────────────────────────────────────────

    void freeActiveSlots()
    {
        for (int i = 0; i < MAX_SAMPLES; ++i)
            slots[i].free();
        loadedCount = 0;
    }

    void freePendingSlots()
    {
        for (int i = 0; i < MAX_SAMPLES; ++i)
            pendingSlots[i].free();
        pendingCount = 0;
    }

    void tryCommitPendingPack()
    {
        if (!swapPending) return;

        bool anyActive = false;
        for (int i = 0; i < MAX_VOICES; ++i)
            if (voices[i].active) {
                anyActive = true;
                break;
            }

        if (anyActive) {
            if (swapCountdown > 0) {
                --swapCountdown;
                // Push voices into release (exit loop, play to end)
                for (int i = 0; i < MAX_VOICES; ++i)
                    if (voices[i].active) {
                        voices[i].looping = false;
                        voices[i].releasing = true;
                    }
                return;
            }
            for (int i = 0; i < MAX_VOICES; ++i)
                voices[i].active = false;
        }

        freeActiveSlots();
        for (int i = 0; i < pendingCount; ++i)
            slots[i] = pendingSlots[i];
        loadedCount = pendingCount;

        for (int i = 0; i < MAX_SAMPLES; ++i) {
            pendingSlots[i].data = nullptr;
            pendingSlots[i].frameCount = 0;
            pendingSlots[i].loaded = false;
        }
        pendingCount = 0;
        swapPending = false;
    }

    void loadPack(const std::string& folderPath)
    {
        freePendingSlots();

        std::vector<std::string> files;
        DIR* dir = opendir(folderPath.c_str());
        if (!dir) return;

        struct dirent* entry;
        while ((entry = readdir(dir)) != nullptr) {
            std::string name = entry->d_name;
            if (name.size() < 4) continue;
            std::string ext = name.substr(name.rfind('.'));
            for (auto& c : ext)
                c = (char)tolower(c);
            if (ext == ".wav" || ext == ".aif" || ext == ".aiff")
                files.push_back(name);
        }
        closedir(dir);
        std::sort(files.begin(), files.end());

        int count = std::min((int)files.size(), MAX_SAMPLES);
        for (int i = 0; i < count; ++i) {
            SF_INFO info = {};
            SNDFILE* sf = sf_open((folderPath + "/" + files[i]).c_str(), SFM_READ, &info);
            if (!sf) continue;

            uint64_t frames = (uint64_t)info.frames;
            int channels = info.channels;
            std::vector<float> tmp((size_t)(frames * channels));
            sf_count_t read = sf_readf_float(sf, tmp.data(), (sf_count_t)frames);
            sf_close(sf);
            if (read <= 0) continue;
            frames = (uint64_t)read;

            float* mono = new float[frames];
            if (channels == 1) {
                std::memcpy(mono, tmp.data(), frames * sizeof(float));
            } else {
                float inv = 1.0f / (float)channels;
                for (uint64_t f = 0; f < frames; ++f) {
                    float s = 0.0f;
                    for (int c = 0; c < channels; ++c)
                        s += tmp[(size_t)(f * channels + c)];
                    mono[f] = s * inv;
                }
            }
            applySampleGain(mono, frames);

            pendingSlots[i].data = mono;
            pendingSlots[i].frameCount = frames;
            pendingSlots[i].loaded = true;
            pendingCount = i + 1;
        }

        assignRootNotes(pendingSlots, pendingCount);
        swapCountdown = MAX_FADE_SAMPLES;
        swapPending = true;
    }

    void scanPacks()
    {
        packNames.clear();
        std::string root = std::string(AUDIO_FOLDER) + "/packs";
        DIR* dir = opendir(root.c_str());
        if (!dir) return;
        struct dirent* entry;
        while ((entry = readdir(dir)) != nullptr) {
            std::string name = entry->d_name;
            if (name == "." || name == "..") continue;
            DIR* t = opendir((root + "/" + name).c_str());
            if (t) {
                closedir(t);
                packNames.push_back(name);
            }
        }
        closedir(dir);
        std::sort(packNames.begin(), packNames.end());
        packParam.max = std::max(0.0f, (float)((int)packNames.size() - 1));
    }

    // ── Morphing LP/HP filter ─────────────────────────────────────────────────
    float applyMorphFilter(float sig, float cutoffParam, float res)
    {
        float absC = std::abs(cutoffParam);
        if (absC < 0.5f) return sig;
        float normC = CLAMP(0.01f + absC * 0.0098f, 0.01f, 0.99f);
        float normR = CLAMP(res, 0.0f, 0.98f);
        float t = (cutoffParam > 0.0f) ? 1.0f : 0.0f;
        svfFilter.setCutoff(normC);
        svfFilter.setResonance(normR);
        auto out = svfFilter.process12(sig);
        return lerp(sig, lerp(out.lp, out.hp, t), absC * 0.01f);
    }

    // ── Reverb ────────────────────────────────────────────────────────────────
    float reverbProcess(float in, float mix, float damp)
    {
        if (mix < 0.001f) return in;
        const float decay = 0.88f;
        float d = 0.2f + damp * 0.7f, invD = 1.0f - d, wet = 0.0f;
        for (int c = 0; c < 4; ++c) {
            float* buf = &reverbBuf[combOff[c]];
            int idx = combIdx[c];
            float del = buf[idx];
            combFb[c] = del * invD + combFb[c] * d;
            buf[idx] = in + combFb[c] * decay;
            if (++idx >= COMB_LEN[c]) idx = 0;
            combIdx[c] = idx;
            wet += del;
        }
        wet *= 0.25f;
        for (int a = 0; a < 3; ++a) {
            float* buf = &reverbBuf[apOff[a]];
            int idx = apIdx[a];
            float del = buf[idx], v = wet + del * 0.5f;
            buf[idx] = v;
            wet = del - v * 0.5f;
            if (++idx >= AP_LEN[a]) idx = 0;
            apIdx[a] = idx;
        }
        return in + wet * mix;
    }

    // ── Delay ─────────────────────────────────────────────────────────────────
    float delayProcess(float sig)
    {
        if (dlyMix.value < 0.001f) return sig;
        int n = std::max(1, std::min((int)(dlyTime.value * 0.001f * sampleRate), DELAY_BUF_SIZE - 1));
        float del = delayBuf[(delayWrite - n + DELAY_BUF_SIZE) % DELAY_BUF_SIZE];
        dlyFbSmooth += 0.001f * (dlyFdbk.value * 0.01f * 0.85f - dlyFbSmooth);
        delayBuf[delayWrite] = sig + del * dlyFbSmooth;
        delayWrite = (delayWrite + 1) % DELAY_BUF_SIZE;
        return lerp(sig, sig + del * 0.7f, dlyMix.value * 0.01f);
    }

    float bufferedFxProcess(float sig)
    {
        sig = delayProcess(sig);
        sig = reverbProcess(sig, reverbMix.value * 0.01f, reverbDamp.value * 0.01f);
        return sig;
    }

    // ── Granular helpers ──────────────────────────────────────────────────────

    void spawnGrain(Voice& v, const SampleSlot& slot)
    {
        float sizeFr = grainSize.value * 0.001f * sampleRate;
        if (sizeFr < 2.0f) sizeFr = 2.0f;

        float spreadFr = spread.value * 0.01f * (float)slot.frameCount * 0.5f;
        float offset = 0.0f;
        if (spreadFr > 0.0f)
            offset = (((float)rand() / (float)RAND_MAX) * 2.0f - 1.0f) * spreadFr;

        double startPos = std::max(0.0, std::min(v.pos + (double)offset, (double)(slot.frameCount - 2)));

        for (int g = 0; g < MAX_GRAINS; ++g) {
            if (!v.grains[g].active) {
                v.grains[g].active = true;
                v.grains[g].pos = startPos;
                v.grains[g].rate = v.rate;
                v.grains[g].endFrame = (uint64_t)(startPos + (double)sizeFr);
                v.grains[g].sizeFrames = sizeFr;
                break;
            }
        }
    }

    float tickGrain(Grain& g, const SampleSlot& slot)
    {
        if (!g.active) return 0.0f;
        float raw = slotRead(slot, g.pos);
        g.pos += g.rate;

        // Hann window: smooth fade-in and fade-out, no clicks
        double grainOrigin = (double)g.endFrame - (double)g.sizeFrames;
        double progress = (g.pos - grainOrigin) / (double)g.sizeFrames;
        progress = CLAMP((float)progress, 0.0f, 1.0f);
        raw *= 0.5f * (1.0f - std::cos((float)(2.0 * M_PI) * (float)progress));

        if (g.pos >= (double)g.endFrame || (uint64_t)g.pos >= slot.frameCount)
            g.active = false;

        return raw;
    }

public:
    char packName[64] = "---";
    char rndModeName[12] = "Any";
    char pitchShiftName[4] = "On";
    char playModeName[16] = "Poly Retrig";

    // ─────────────────────────────────────────────────────────────────────────
    // Parameters (21 total)
    //
    //  PAGE 1 — SAMPLER
    //   0  Pack          folder selector
    //   1  Pitch Shift   1=On  2=Off
    //   2  Transpose     st  (-24..+24)
    //   3  Mode          1=Poly Retrig  2=Poly  3=Mono Choke  4=Mono Hold
    //
    //  PAGE 2 — RANDOM
    //   4  Random        %   trigger probability
    //   5  Rnd Mode      1=Any  2=Adjacent
    //   6  Rnd Pitch     st  random pitch offset (±N semitones)
    //
    //  PAGE 3 — LOOP
    //   7  Loop Start    %   position where the loop begins
    //   8  Loop Length   ms  length of loop region; 0 = one-shot
    //
    //  PAGE 4 — GRANULAR  (Density=0 → plain playback)
    //   9  Density       g/s  grains per second (0–10)
    //  10  Grain Size    ms   duration of each grain
    //  11  Spread        %    random scatter of grain start position
    //  12  Grain Delay   ms   delay before cloud starts after note-on
    //
    //  PAGE 5 — FILTER
    //  13  Cutoff        %   -100=LP … 0=bypass … +100=HP
    //  14  Resonance     %
    //
    //  PAGE 6 — FX
    //  15  Drive         %
    //  16  Reverb Mix    %
    //  17  Rvb Damp      %
    //  18  Dly Mix       %
    //  19  Dly Time      ms
    //  20  Dly Fdbk      %
    // ─────────────────────────────────────────────────────────────────────────

    Param params[21] = {
        // PAGE 1
        { .label = "Pack", .string = packName, .value = 0.0f, .min = 0.0f, .max = 0.0f, .step = 1.0f, .onUpdate = [](void* ctx, float val) {
             auto* s = (Sample23*)ctx;
             int i = (int)val;
             if (i < 0 || i >= (int)s->packNames.size()) return;
             strncpy(s->packName, s->packNames[i].c_str(), sizeof(s->packName) - 1);
             s->loadPack(std::string(AUDIO_FOLDER) + "/packs/" + s->packNames[i]);
             s->currentPack = i;
         } }, // 0
        { .label = "Pitch Shift", .string = pitchShiftName, .value = 1.0f, .min = 1.0f, .max = 2.0f, .step = 1.0f, .onUpdate = [](void* ctx, float val) {
             strcpy(((Sample23*)ctx)->pitchShiftName, ((int)val == 2) ? "Off" : "On");
         } }, // 1
        { .label = "Transpose", .unit = "st", .value = 0.0f, .min = -24.0f, .max = 24.0f, .step = 1.0f }, // 2
        { .label = "Mode", .string = playModeName, .value = 1.0f, .min = 1.0f, .max = 4.0f, .step = 1.0f, .onUpdate = [](void* ctx, float val) {
             auto* s = (Sample23*)ctx;
             switch ((int)val) {
             case 2:
                 strcpy(s->playModeName, "Poly");
                 break;
             case 3:
                 strcpy(s->playModeName, "Mono Choke");
                 break;
             case 4:
                 strcpy(s->playModeName, "Mono Hold");
                 break;
             default:
                 strcpy(s->playModeName, "Poly Retrig");
                 break;
             }
         } }, // 3

        // PAGE 2
        { .label = "Random", .unit = "%", .value = 0.0f }, // 4
        { .label = "Rnd Mode", .string = rndModeName, .value = 1.0f, .min = 1.0f, .max = 2.0f, .step = 1.0f, .onUpdate = [](void* ctx, float val) {
             strcpy(((Sample23*)ctx)->rndModeName, ((int)val == 2) ? "Adjacent" : "Any");
         } }, // 5
        { .label = "Rnd Pitch", .unit = "st", .value = 0.0f, .min = 0.0f, .max = 12.0f, .step = 1.0f }, // 6

        // PAGE 3
        { .label = "Loop Start", .unit = "%", .value = 0.0f }, // 7
        { .label = "Loop Length", .unit = "ms", .value = 0.0f, .min = 0.0f, .max = 4000.0f, .step = 5.0f }, // 8

        // PAGE 4
        { .label = "Density", .unit = "g/s", .value = 0.0f, .min = 0.0f, .max = 10.0f, .step = 0.1f }, // 9
        { .label = "Grain Size", .unit = "ms", .value = 80.0f, .min = 5.0f, .max = 500.0f, .step = 5.0f }, // 10
        { .label = "Spread", .unit = "%", .value = 0.0f }, // 11
        { .label = "Grain Delay", .unit = "ms", .value = 0.0f, .min = 0.0f, .max = 2000.0f, .step = 5.0f }, // 12

        // PAGE 5
        { .label = "Cutoff", .unit = "%", .value = 0.0f, .min = -100.0f, .max = 100.0f }, // 13
        { .label = "Resonance", .unit = "%", .value = 0.0f }, // 14

        // PAGE 6
        { .label = "Drive", .unit = "%", .value = 0.0f }, // 15
        { .label = "Reverb Mix", .unit = "%", .value = 0.0f }, // 16
        { .label = "Rvb Damp", .unit = "%", .value = 50.0f }, // 17
        { .label = "Dly Mix", .unit = "%", .value = 0.0f }, // 18
        { .label = "Dly Time", .unit = "ms", .value = 125.0f, .min = 10.0f, .max = 1000.0f, .step = 5.0f }, // 19
        { .label = "Dly Fdbk", .unit = "%", .value = 0.0f }, // 20
    };

    Param& packParam = params[0];
    Param& pitchShift = params[1];
    Param& transpose = params[2];
    Param& retrigger = params[3]; // alias kept for internal use
    Param& randomAmt = params[4];
    Param& rndMode = params[5];
    Param& rndPitch = params[6];
    Param& loopStart = params[7];
    Param& loopLength = params[8];
    Param& density = params[9];
    Param& grainSize = params[10];
    Param& spread = params[11];
    Param& grainDelay = params[12];
    Param& cutoff = params[13];
    Param& resonance = params[14];
    Param& drive = params[15];
    Param& reverbMix = params[16];
    Param& reverbDamp = params[17];
    Param& dlyMix = params[18];
    Param& dlyTime = params[19];
    Param& dlyFdbk = params[20];

    // ─────────────────────────────────────────────────────────────────────────
    // Constructor / Destructor
    // ─────────────────────────────────────────────────────────────────────────

    Sample23(float sr, float* dlBuf, float* rvBuf)
        : EngineBase(Sampler, "Sample23", params)
        , sampleRate(sr)
        , sampleRateDiv(1.0f / sr)
        , delayBuf(dlBuf)
        , reverbBuf(rvBuf)
    {
        if (delayBuf)
            for (int i = 0; i < DELAY_BUF_SIZE; ++i)
                delayBuf[i] = 0.0f;

        if (reverbBuf) {
            int pos = 0;
            for (int c = 0; c < 4; ++c) {
                combOff[c] = pos;
                pos += COMB_LEN[c];
            }
            for (int a = 0; a < 3; ++a) {
                apOff[a] = pos;
                pos += AP_LEN[a];
            }
            for (int i = 0; i < REVERB_BUF_SIZE; ++i)
                reverbBuf[i] = 0.0f;
        }

        scanPacks();

        if (!packNames.empty()) {
            strncpy(packName, packNames[0].c_str(), sizeof(packName) - 1);
            loadPack(std::string(AUDIO_FOLDER) + "/packs/" + packNames[0]);
            swapCountdown = 0; // commit immediately — no voices yet
            tryCommitPendingPack();
            currentPack = 0;
        }

        init();
    }

    ~Sample23()
    {
        freeActiveSlots();
        freePendingSlots();
    }

    // ─────────────────────────────────────────────────────────────────────────
    // noteOnImpl
    // ─────────────────────────────────────────────────────────────────────────

    void noteOnImpl(uint8_t note, float vel)
    {
        if (loadedCount == 0) return;

        int transposed = CLAMP((int)note + (int)transpose.value, 0, 127);

        // Strict range check — notes outside the mapped range → silence
        int slotIdx = noteToSlotIndex((uint8_t)transposed);
        if (slotIdx < 0) return;

        // Randomiser (overrides slot, always plays at native pitch)
        bool useNativePitch = false;
        if (randomAmt.value > 0.001f && ((float)rand() / (float)RAND_MAX) < randomAmt.value * 0.01f) {
            if ((int)(rndMode.value + 0.5f) == 2)
                slotIdx = CLAMP(slotIdx + ((rand() & 1) ? 1 : -1), 0, loadedCount - 1);
            else
                slotIdx = rand() % loadedCount;
            useNativePitch = true;
        }

        // Play mode
        //  1 = Poly Retrig : always start a new voice (steal if needed)
        //  2 = Poly        : skip if this note is already playing
        //  3 = Mono Choke  : stop all playing voices immediately, then play
        //  4 = Mono Hold   : if any voice is active, do nothing
        int mode = (int)(retrigger.value + 0.5f);

        if (mode == 4) {
            // Mono Hold: any active voice blocks new notes
            for (int i = 0; i < MAX_VOICES; ++i)
                if (voices[i].active) return;
        } else if (mode == 3) {
            // Mono Choke: silence everything immediately
            for (int i = 0; i < MAX_VOICES; ++i)
                voices[i].active = false;
        } else if (mode == 2) {
            // Poly: skip if this exact note is already sounding
            for (int i = 0; i < MAX_VOICES; ++i)
                if (voices[i].active && voices[i].midiNote == (uint8_t)transposed)
                    return;
        }
        // mode == 1 (Poly Retrig): fall through, allocVoice() steals as needed

        int vi = allocVoice();
        Voice& v = voices[vi];
        v = Voice {}; // zero-init all fields including grains[]
        v.active = true;
        v.slotIdx = slotIdx;
        v.midiNote = (uint8_t)transposed;
        v.velocity = vel;
        v.pos = 0.0;

        // Playback rate
        if (!useNativePitch && (int)(pitchShift.value + 0.5f) == 1) {
            float semitones = (float)transposed - (float)slots[slotIdx].rootNote;
            v.rate = std::pow(2.0f, semitones / 12.0f);
        }
        if (rndPitch.value > 0.001f) {
            float rp = ((float)rand() / (float)RAND_MAX) * 2.0f - 1.0f;
            v.rate *= std::pow(2.0f, rp * rndPitch.value / 12.0f);
        }

        // Loop region
        const SampleSlot& slot = slots[slotIdx];
        if (loopLength.value > 0.5f && slot.frameCount > 0) {
            uint64_t lsF = (uint64_t)(loopStart.value * 0.01f * (float)slot.frameCount);
            uint64_t llF = (uint64_t)(loopLength.value * 0.001f * sampleRate);
            v.loopStart = std::min(lsF, slot.frameCount - 1);
            v.loopEnd = std::min(v.loopStart + llF, slot.frameCount);
            v.looping = (v.loopEnd > v.loopStart);
        }

        // Granular
        v.granular = (density.value > 0.001f);
        if (v.granular) {
            v.grainDelayCnt = grainDelay.value * 0.001f * sampleRate;
            v.grainTimer = 0.0f;
        }
    }

    // ─────────────────────────────────────────────────────────────────────────
    // noteOffImpl
    // ─────────────────────────────────────────────────────────────────────────

    void noteOffImpl(uint8_t note)
    {
        int transposed = CLAMP((int)note + (int)transpose.value, 0, 127);
        for (int i = 0; i < MAX_VOICES; ++i) {
            Voice& v = voices[i];
            if (!v.active || v.midiNote != (uint8_t)transposed) continue;

            if (!v.looping) {
                // One-shot or loop not configured: note-off has no effect
            } else if (!v.inLoop) {
                // Note-off arrived before entering the loop: skip it entirely
                v.looping = false;
            } else {
                // Inside loop: finish current cycle then play out to end
                v.releasing = true;
            }
        }
    }

    // ─────────────────────────────────────────────────────────────────────────
    // sampleImpl
    // ─────────────────────────────────────────────────────────────────────────

    float sampleImpl()
    {
        tryCommitPendingPack();

        float mix = 0.0f;
        bool anyActive = false;

        for (int vi = 0; vi < MAX_VOICES; ++vi) {
            Voice& v = voices[vi];
            if (!v.active) continue;

            const SampleSlot& slot = slots[v.slotIdx];
            if (!slot.loaded) {
                v.active = false;
                continue;
            }

            float voiceSample = 0.0f;

            if (v.granular) {
                // ── GRANULAR ──────────────────────────────────────────────────

                // Advance main read head (anchors where grains are spawned)
                v.pos += v.rate;

                if (v.grainDelayCnt > 0.0f) {
                    v.grainDelayCnt -= 1.0f;
                } else {
                    v.grainTimer -= 1.0f;
                    if (v.grainTimer <= 0.0f) {
                        spawnGrain(v, slot);
                        v.grainTimer = sampleRate / std::max(0.001f, density.value);
                    }
                }

                for (int g = 0; g < MAX_GRAINS; ++g)
                    voiceSample += tickGrain(v.grains[g], slot);

                // Voice ends when head reaches end of sample and all grains done
                bool anyGrain = false;
                for (int g = 0; g < MAX_GRAINS; ++g)
                    if (v.grains[g].active) {
                        anyGrain = true;
                        break;
                    }
                if (v.pos >= (double)slot.frameCount && !anyGrain)
                    v.active = false;

            } else {
                // ── PLAIN PLAYBACK ────────────────────────────────────────────

                // Detect loop entry
                if (v.looping && !v.inLoop && v.pos >= (double)v.loopStart)
                    v.inLoop = true;

                if (v.inLoop && !v.releasing) {
                    // Wrap within loop region
                    while (v.pos >= (double)v.loopEnd)
                        v.pos -= (double)(v.loopEnd - v.loopStart);
                } else if (v.pos >= (double)slot.frameCount) {
                    v.active = false;
                    continue;
                }

                voiceSample = slotRead(slot, v.pos);
                v.pos += v.rate;
            }

            mix += voiceSample * v.velocity;
            anyActive = true;
        }

        if (!anyActive) return bufferedFxProcess(0.0f);

        mix = CLAMP(mix, -1.0f, 1.0f);
        mix = applyMorphFilter(mix, cutoff.value, resonance.value * 0.01f);
        mix = applyDrive(mix, drive.value * 0.01f);
        return bufferedFxProcess(mix);
    }
};