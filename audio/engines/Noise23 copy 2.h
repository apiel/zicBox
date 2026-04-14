#pragma once

#include "audio/Lfo.h"
#include "audio/MultiFx.h"
#include "audio/NoiseGenerator.h"
#include "audio/engines/EngineBase.h"
#include "audio/filterSVF.h"
#include "audio/utils/math.h"

#include <algorithm>
#include <cmath>
#include <cstring>

// ─────────────────────────────────────────────────────────────────────────────
// Noise23  —  noise-sculpting synthesiser
//
// Signal chain overview:
//
//   [NoiseGen A]──┐
//                 ├─ mix ─► [BandPass A] ─► [Comb Bank] ─►─┐
//   [NoiseGen B]──┘                                          │
//                                                            ├─► [Amp Env]
//   [NoiseGen C (grain)] ─► [BandPass B] ─► [AM ring] ─────►│
//                                                            │
//                           [HP trim] ◄─────────────────────┘
//                               │
//                           [Gain + Vel]
//                               │
//                       [MultiFx → Delay → Reverb]
//
// Three independent noise generators (each with different spectral colour)
// are mixed and then shaped by:
//   • Dual bandpass / resonant filters (BP-A for the "body", BP-B for texture)
//   • A bank of four parallel comb filters for tonal/pitched resonance
//   • Amplitude modulation with a dedicated AM noise source (creates trembling,
//     gurgling, or buzzing textures)
//   • A global amplitude ADSR
//   • A second "texture" ADSR that drives BP-A centre frequency sweep
//   • LFO routable to: pitch of comb bank, BP centre, AM depth, overall level
//   • HP trim, drive (soft clip), gain
//   • MultiFx, delay, reverb
// ─────────────────────────────────────────────────────────────────────────────

class Noise23 : public EngineBase<Noise23> {

public:
    static constexpr int DELAY_BUF_SIZE = 48000;
    static constexpr int REVERB_BUF_SIZE = 16384;

    MultiFx multiFx;

protected:
    const float sampleRate;
    const float sampleRateDiv;

    // ── Three noise generators ────────────────────────────────────────────────
    // NoiseGenerator already supports colour (0=white … 1=brown via LP)
    NoiseGenerator noiseA; // "body" noise
    NoiseGenerator noiseB; // secondary layer / blend
    NoiseGenerator noiseC; // AM / grain modulator

    // ── Filters ───────────────────────────────────────────────────────────────
    FilterSVF bpA; // bandpass on main mix
    FilterSVF bpB; // bandpass on noiseB

    // ── Comb filter bank (4 combs, for tonal resonance) ──────────────────────
    // Heap-allocated to avoid stack overflow (4 * 2048 * 4 bytes = 32 KB)
    static constexpr int MAX_COMB = 2048;
    float* combBuf[4] = {};
    int combIdx[4] = {};
    float combFb[4] = {}; // per-comb feedback state (IIR damp)

    // ── AM modulator state ────────────────────────────────────────────────────
    float amSmooth = 0.0f; // one-pole LP on noiseC — must be a member, NOT static local

    // ── Amplitude / texture envelopes ─────────────────────────────────────────
    float ampEnv = 0.0f;
    int ampStage = 0;
    float ampAttRate = 0.0f;
    float ampDecRate = 0.0f;
    float ampRelRate = 0.0f;

    float texEnv = 0.0f; // drives BP-A centre sweep
    int texStage = 0;
    float texAttRate = 0.0f;
    float texDecRate = 0.0f;
    float texRelRate = 0.0f;

    // ── LFO ───────────────────────────────────────────────────────────────────
    Lfo lfo;

    // ── Misc state ────────────────────────────────────────────────────────────
    float hpState = 0.0f;
    float velocity = 1.0f;
    bool gateOpen = false;

    float* delayBuf = nullptr;
    int delayWrite = 0;
    float dlyFbSmooth = 0.0f;

    float* reverbBuf = nullptr;

    static constexpr int COMB_LEN[4] = { 1559, 1617, 1685, 1751 };
    static constexpr int AP_LEN[3] = { 347, 113, 37 };
    int rvCombOff[4] = {};
    int rvApOff[3] = {};
    int rvCombIdx[4] = {};
    int rvApIdx[3] = {};
    float rvCombFb[4] = {};

    char fxName[24] = "Off";

    // ── Helpers ───────────────────────────────────────────────────────────────
    static float lerp(float a, float b, float t) { return a + t * (b - a); }
    float tau(float ms) const { return (ms < 0.01f) ? 0.0f : Math::exp(-1.0f / (sampleRate * ms * 0.001f)); }
    float linearRate(float ms) const { return (ms < 0.01f) ? 1.0f : 1.0f / (sampleRate * ms * 0.001f); }

    static float softClip(float x)
    {
        // Soft saturation — keeps noise from becoming harsh at high drive
        if (x > 1.0f) return 1.0f - 1.0f / (1.0f + (x - 1.0f));
        if (x < -1.0f) return -1.0f + 1.0f / (1.0f + (-x - 1.0f));
        return x;
    }

    // ── Generic ADSR tick (same logic as Synth23) ─────────────────────────────
    float adsrTick(float& env, int& stage,
        float attackRate, float decayRate,
        float sustainLevel, float releaseRate,
        bool gate)
    {
        if (!gate && stage != 0 && stage != 4) stage = 4;
        switch (stage) {
        case 0:
            env = 0.0f;
            break;
        case 1:
            env += attackRate;
            if (env >= 1.0f) {
                env = 1.0f;
                stage = 2;
            }
            break;
        case 2:
            env -= decayRate;
            if (env <= sustainLevel) {
                env = sustainLevel;
                stage = 3;
            }
            break;
        case 3:
            env = sustainLevel;
            break;
        case 4:
            env -= releaseRate;
            if (env < 0.0001f) {
                env = 0.0f;
                stage = 0;
            }
            break;
        }
        return env;
    }

    // ── Comb bank (tonal resonance) ───────────────────────────────────────────
    // baseFreq controls the fundamental; spread widens/closes the harmonics.
    // Each comb is tuned to a harmonic of baseFreq; feedback amount sets tone.
    float combProcess(float in, float baseFreq, float spread, float fbAmt, float damp)
    {
        if (fbAmt < 0.001f) return in;

        float wet = 0.0f;
        for (int c = 0; c < 4; ++c) {
            // harmonic multiplier: 1x, 2x, 3x, 4x — spread scales the interval
            float harmonic = 1.0f + (float)c * spread;
            float freq = baseFreq * harmonic;
            freq = std::max(20.0f, std::min(freq, sampleRate * 0.45f));
            int delaySmp = std::max(1, (int)(sampleRate / freq));
            delaySmp = std::min(delaySmp, MAX_COMB - 1);

            int readIdx = (combIdx[c] - delaySmp + MAX_COMB) % MAX_COMB;
            float delayed = combBuf[c][readIdx];

            // One-pole LP damp inside comb loop
            combFb[c] = delayed * (1.0f - damp) + combFb[c] * damp;

            combBuf[c][combIdx[c]] = in + combFb[c] * fbAmt;
            combIdx[c] = (combIdx[c] + 1) % MAX_COMB;

            wet += delayed;
        }
        wet *= 0.25f;
        return lerp(in, wet, fbAmt);
    }

    // ── Reverb (Schroeder, same topology as Synth23) ──────────────────────────
    float reverbProcess(float in, float mix, float damp)
    {
        if (mix < 0.001f) return in;
        const float decay = 0.88f;
        float d = 0.2f + damp * 0.7f;
        float invD = 1.0f - d;
        float wet = 0.0f;
        for (int c = 0; c < 4; ++c) {
            float* buf = &reverbBuf[rvCombOff[c]];
            int idx = rvCombIdx[c];
            float del = buf[idx];
            rvCombFb[c] = del * invD + rvCombFb[c] * d;
            buf[idx] = in + rvCombFb[c] * decay;
            if (++idx >= COMB_LEN[c]) idx = 0;
            rvCombIdx[c] = idx;
            wet += del;
        }
        wet *= 0.25f;
        for (int a = 0; a < 3; ++a) {
            float* buf = &reverbBuf[rvApOff[a]];
            int idx = rvApIdx[a];
            float del = buf[idx];
            float v = wet + del * 0.5f;
            buf[idx] = v;
            wet = del - v * 0.5f;
            if (++idx >= AP_LEN[a]) idx = 0;
            rvApIdx[a] = idx;
        }
        return in + wet * mix;
    }

    float delayProcess(float sig)
    {
        if (dlyMix.value < 0.001f) return sig;
        int delaySmp = std::max(1, std::min((int)(dlyTime.value * 0.001f * sampleRate), DELAY_BUF_SIZE - 1));
        int readIdx = (delayWrite - delaySmp + DELAY_BUF_SIZE) % DELAY_BUF_SIZE;
        float delayed = delayBuf[readIdx];
        float fbTarget = dlyFdbk.value * 0.01f * 0.85f;
        dlyFbSmooth += 0.001f * (fbTarget - dlyFbSmooth);
        delayBuf[delayWrite] = sig + delayed * dlyFbSmooth;
        delayWrite = (delayWrite + 1) % DELAY_BUF_SIZE;
        return lerp(sig, sig + delayed * 0.7f, dlyMix.value * 0.01f);
    }

    float bufferedFxProcess(float sig)
    {
        sig = multiFx.apply(sig, fxAmt.value * 0.01f);
        sig = delayProcess(sig);
        sig = reverbProcess(sig, reverbMix.value * 0.01f, reverbDamp.value * 0.01f);
        return sig;
    }

    // ── Helper: map 0-1 normalised cutoff to Hz for SVF ──────────────────────
    // SVF setCutoff typically expects 0-1 (normalised). We expose Hz-ish params
    // and convert internally so the musician thinks in familiar terms.
    // Mapping: 0→20 Hz, 1→20 kHz (log scale)
    static float hzToNorm(float hz, float sr)
    {
        hz = std::max(20.0f, std::min(hz, sr * 0.49f));
        return hz / (sr * 0.5f); // simple linear norm — adjust if SVF expects log
    }

public:
    char filterAName[32] = "BP-A";
    char filterBName[32] = "BP-B";

    enum ParamTarget { NONE,
        PG_NOISE,
        PG_TONE,
        PG_COMB,
        PG_MOD,
        PG_ENV,
        PG_FX };

    Param params[37];

    // ── Master ────────────────────────────────────────────────────────────────
    Param& gain = addParam({ .key = "gain", .label = "Gain", .unit = "%", .value = 60.0f });

    // ── Amplitude ADSR ────────────────────────────────────────────────────────
    Param& ampAttack = addParam({ .key = "ampAttack", .label = "Amp Attack", .unit = "ms", .value = 500.0f, .min = 5.0f, .max = 8000.0f, .step = 5.0f, .target = PG_ENV, .module = MODULE_ENV_ADSR });
    Param& ampDecay = addParam({ .key = "ampDecay", .label = "Amp Decay", .unit = "ms", .value = 1000.0f, .min = 10.0f, .max = 8000.0f, .step = 5.0f, .target = PG_ENV, .module = MODULE_ENV_ADSR });
    Param& ampSustain = addParam({ .key = "ampSustain", .label = "Amp Sustain", .unit = "%", .value = 80.0f, .target = PG_ENV, .module = MODULE_ENV_ADSR });
    Param& ampRelease = addParam({ .key = "ampRelease", .label = "Amp Release", .unit = "ms", .value = 2000.0f, .min = 10.0f, .max = 12000.0f, .step = 5.0f, .target = PG_ENV, .module = MODULE_ENV_ADSR });

    // ── Noise sources ─────────────────────────────────────────────────────────
    Param& colourA = addParam({ .key = "colourA", .label = "Noise A Colour", .unit = "%", .value = 30.0f, .target = PG_NOISE });
    Param& colourB = addParam({ .key = "colourB", .label = "Noise B Colour", .unit = "%", .value = 70.0f, .target = PG_NOISE });
    Param& noiseBlend = addParam({ .key = "noiseBlend", .label = "A/B Blend", .unit = "%", .value = 30.0f, .target = PG_NOISE });

    // ── BP-A  (body bandpass) ─────────────────────────────────────────────────
    // Centre expressed in Hz; resonance 0-100 %
    Param& bpAFreq = addParam({ .key = "bpAFreq", .label = "BP-A Freq", .unit = "Hz", .value = 800.0f, .min = 20.0f, .max = 18000.0f, .step = 10.0f, .target = PG_TONE });
    Param& bpARes = addParam({ .key = "bpARes", .label = "BP-A Res", .unit = "%", .value = 40.0f, .target = PG_TONE });
    Param& bpAWidth = addParam({ .key = "bpAWidth", .label = "BP-A Width", .unit = "%", .value = 50.0f, .target = PG_TONE });

    // ── BP-B  (texture bandpass on noise B) ───────────────────────────────────
    Param& bpBFreq = addParam({ .key = "bpBFreq", .label = "BP-B Freq", .unit = "Hz", .value = 3000.0f, .min = 20.0f, .max = 18000.0f, .step = 10.0f, .target = PG_TONE });
    Param& bpBRes = addParam({ .key = "bpBRes", .label = "BP-B Res", .unit = "%", .value = 20.0f, .target = PG_TONE });

    // ── Comb bank ─────────────────────────────────────────────────────────────
    // Tuning: 20–2000 Hz fundamental resonant frequency
    Param& combFreq = addParam({ .key = "combFreq", .label = "Comb Freq", .unit = "Hz", .value = 200.0f, .min = 20.0f, .max = 2000.0f, .step = 1.0f, .target = PG_COMB });
    Param& combFbAmt = addParam({ .key = "combFbAmt", .label = "Comb Amount", .unit = "%", .value = 0.0f, .target = PG_COMB });
    Param& combSpread = addParam({ .key = "combSpread", .label = "Comb Spread", .value = 1.0f, .min = 0.5f, .max = 3.0f, .step = 0.05f, .target = PG_COMB });
    Param& combDamp = addParam({ .key = "combDamp", .label = "Comb Damp", .unit = "%", .value = 50.0f, .target = PG_COMB });

    // ── LFO ───────────────────────────────────────────────────────────────────
    Param& lfoType = addParam({ .key = "lfoType", .label = "LFO Type", .string = lfo.typeName, .value = 0.0f, .max = Lfo::COUNT - 1, .target = PG_MOD, .module = MODULE_LFO, .onUpdate = [](void* c, float v) { ((Noise23*)c)->lfo.setType((int)v); }, .graph = [](void* ctx, float val) { return ((Noise23*)ctx)->lfo.graph(val); } });
    Param& lfoRate = addParam({ .key = "lfoRate", .label = "LFO Rate", .unit = "Hz", .value = 0.3f, .min = 0.01f, .max = 40.0f, .step = 0.01f, .incType = INC_SCALED, .target = PG_MOD, .module = MODULE_LFO, .onUpdate = [](void* c, float v) { ((Noise23*)c)->lfo.rateHz = v; } });
    Param& lfoToBpA = addParam({ .key = "lfoToBpA", .label = "LFO > BP-A Freq", .unit = "%", .value = 0.0f, .min = -100.0f, .max = 100.0f, .target = PG_MOD });
    Param& lfoToComb = addParam({ .key = "lfoToComb", .label = "LFO > Comb Freq", .unit = "%", .value = 0.0f, .min = -100.0f, .max = 100.0f, .target = PG_MOD });
    Param& lfoToLevel = addParam({ .key = "lfoToLevel", .label = "LFO > Level", .unit = "%", .value = 0.0f, .target = PG_MOD });

    // ── Texture ADSR (drives BP-A centre sweep) ───────────────────────────────
    Param& bpBLevel = addParam({ .key = "bpBLevel", .label = "Texture Lvl", .unit = "%", .value = 30.0f, .target = PG_TONE });
    Param& texEnvMod = addParam({ .key = "texEnvMod", .label = "Tex > BP-A", .unit = "oct", .value = 1.0f, .min = 0.0f, .max = 4.0f, .step = 0.05f, .target = PG_ENV });
    Param& texAttack = addParam({ .key = "texAttack", .label = "Tex Attack", .unit = "ms", .value = 800.0f, .min = 5.0f, .max = 8000.0f, .step = 5.0f, .target = PG_ENV, .module = MODULE_ENV_ADSR });
    Param& texDecay = addParam({ .key = "texDecay", .label = "Tex Decay", .unit = "ms", .value = 1200.0f, .min = 10.0f, .max = 8000.0f, .step = 5.0f, .target = PG_ENV, .module = MODULE_ENV_ADSR });
    Param& texSustain = addParam({ .key = "texSustain", .label = "Tex Sustain", .unit = "%", .value = 60.0f, .target = PG_ENV, .module = MODULE_ENV_ADSR });
    Param& texRelease = addParam({ .key = "texRelease", .label = "Tex Release", .unit = "ms", .value = 3000.0f, .min = 10.0f, .max = 12000.0f, .step = 5.0f, .target = PG_ENV, .module = MODULE_ENV_ADSR });

    // ── Drive & HP ────────────────────────────────────────────────────────────
    Param& drive = addParam({ .key = "drive", .label = "Drive", .unit = "%", .value = 0.0f });
    Param& hpTrim = addParam({ .key = "hpTrim", .label = "HP Trim", .unit = "%", .value = 0.0f });

    // ── FX ────────────────────────────────────────────────────────────────────
    Param& fxType = addParam({ .key = "fxType", .label = "FX Type", .string = fxName, .value = 0.0f, .max = (float)MultiFx::FX_COUNT - 1, .step = 1.0f, .onUpdate = [](void* ctx, float v) { auto e = (Noise23*)ctx; e->multiFx.setEffect(v);       strcpy(e->fxName, e->multiFx.getEffectName()); }, .hydrateFn = [](void* ctx, const char* vStr) { auto e = (Noise23*)ctx; e->multiFx.setEffect(vStr); } });
    Param& fxAmt = addParam({ .key = "fxAmt", .label = "FX Amount", .unit = "%", .value = 0.0f });
    Param& reverbMix = addParam({ .key = "reverbMix", .label = "Reverb Mix", .unit = "%", .value = 20.0f, .target = PG_FX });
    Param& reverbDamp = addParam({ .key = "reverbDamp", .label = "Rvb Damp", .unit = "%", .value = 50.0f, .target = PG_FX });
    Param& dlyMix = addParam({ .key = "dlyMix", .label = "Dly Mix", .unit = "%", .value = 0.0f, .target = PG_FX });
    Param& dlyTime = addParam({ .key = "dlyTime", .label = "Dly Time", .unit = "ms", .value = 250.0f, .min = 10.0f, .max = 1000.0f, .step = 5.0f, .target = PG_FX });
    Param& dlyFdbk = addParam({ .key = "dlyFdbk", .label = "Dly Fdbk", .unit = "%", .value = 0.0f, .target = PG_FX });

    // ── Constructor ───────────────────────────────────────────────────────────
    Noise23(float sr, float* dlBuf, float* rvBuf, float* fxBuf)
        : EngineBase(Synth, "Noise23", params)
        , multiFx(sr, fxBuf)
        , sampleRate(sr)
        , sampleRateDiv(1.0f / sr)
        , noiseA(sr)
        , noiseB(sr)
        , noiseC(sr)
        , delayBuf(dlBuf)
        , reverbBuf(rvBuf)
        , lfo(sr)
    {
        if (delayBuf)
            for (int i = 0; i < DELAY_BUF_SIZE; ++i)
                delayBuf[i] = 0.0f;

        if (reverbBuf) {
            int pos = 0;
            for (int c = 0; c < 4; ++c) {
                rvCombOff[c] = pos;
                pos += COMB_LEN[c];
            }
            for (int a = 0; a < 3; ++a) {
                rvApOff[a] = pos;
                pos += AP_LEN[a];
            }
            for (int i = 0; i < REVERB_BUF_SIZE; ++i)
                reverbBuf[i] = 0.0f;
        }

        for (int c = 0; c < 4; ++c) {
            combBuf[c] = new float[MAX_COMB](); // zero-initialised
        }
    }

    ~Noise23()
    {
        for (int c = 0; c < 4; ++c)
            delete[] combBuf[c];
    }

    // ── Note on/off ───────────────────────────────────────────────────────────
    void noteOnImpl(uint8_t /*note*/, float vel)
    {
        velocity = vel;
        gateOpen = true;

        ampAttRate = linearRate(ampAttack.value);
        ampDecRate = linearRate(ampDecay.value);
        ampRelRate = linearRate(ampRelease.value);
        ampEnv = 0.0f;
        ampStage = 1;

        texAttRate = linearRate(texAttack.value);
        texDecRate = linearRate(texDecay.value);
        texRelRate = linearRate(texRelease.value);
        texEnv = 0.0f;
        texStage = 1;

        lfo.reset();
    }

    void noteOffImpl(uint8_t) { gateOpen = false; }

    // ── Main sample tick ──────────────────────────────────────────────────────
    float sampleImpl()
    {
        if (ampStage == 0 && !gateOpen)
            return bufferedFxProcess(0.0f);

        // ── Envelopes ─────────────────────────────────────────────────────────
        float eAmp = adsrTick(ampEnv, ampStage,
            ampAttRate, ampDecRate,
            ampSustain.value * 0.01f, ampRelRate,
            gateOpen);

        float eTex = adsrTick(texEnv, texStage,
            texAttRate, texDecRate,
            texSustain.value * 0.01f, texRelRate,
            gateOpen);

        // ── LFO ───────────────────────────────────────────────────────────────
        float lfoOut = lfo.process();

        // ── Noise sources ─────────────────────────────────────────────────────
        float nA = noiseA.get(colourA.value * 0.01f);
        float nB = noiseB.get(colourB.value * 0.01f);

        float blend = noiseBlend.value * 0.01f;
        float noiseMain = lerp(nA, nB, blend); // 0=all A, 1=all B

        // ── BP-A on main noise (with texture env sweep) ───────────────────────
        // texEnvMod shifts centre freq up by up to 4 octaves
        float bpACenter = bpAFreq.value * std::pow(2.0f, eTex * texEnvMod.value);
        // LFO modulates BP-A centre (±50% of current centre)
        bpACenter *= std::pow(2.0f, lfoOut * lfoToBpA.value * 0.01f * 2.0f);
        bpACenter = std::max(20.0f, std::min(bpACenter, sampleRate * 0.45f));

        float bpANorm = hzToNorm(bpACenter, sampleRate);
        float bpAResNorm = CLAMP(0.90f * (1.0f - Math::pow(1.0f - bpARes.value * 0.01f, 2.0f)), 0.0f, 0.98f);

        bpA.setCutoff(bpANorm);
        bpA.setResonance(bpAResNorm);
        // bpAWidth scales the balance between bp and lp output (width=0: narrow BP, width=100: wider)
        float bpAWidthMix = bpAWidth.value * 0.01f;
        auto bpAOut = bpA.process12(noiseMain);
        float sigA = lerp(bpAOut.bp, bpAOut.lp * 0.5f + bpAOut.bp * 0.5f, bpAWidthMix);

        // ── BP-B on noise B (separate texture layer) ──────────────────────────
        float bpBNorm = hzToNorm(bpBFreq.value, sampleRate);
        float bpBResNorm = CLAMP(0.90f * (1.0f - Math::pow(1.0f - bpBRes.value * 0.01f, 2.0f)), 0.0f, 0.98f);
        bpB.setCutoff(bpBNorm);
        bpB.setResonance(bpBResNorm);
        float sigB = bpB.process12(nB).bp * (bpBLevel.value * 0.01f);

        float sig = sigA + sigB;

        // ── Comb bank (tonal resonance, e.g. pitched wind tones) ──────────────
        float dynCombFreq = combFreq.value * std::pow(2.0f, lfoOut * lfoToComb.value * 0.01f * 2.0f);
        dynCombFreq = std::max(20.0f, std::min(dynCombFreq, 2000.0f));
        sig = combProcess(sig, dynCombFreq,
            combSpread.value,
            combFbAmt.value * 0.01f,
            combDamp.value * 0.01f);

        // ── Amplitude envelope + velocity ────────────────────────────────────
        float levelMod = 1.0f + lfoOut * lfoToLevel.value * 0.01f;
        sig *= eAmp * velocity * levelMod;

        // ── Drive (soft saturation) ───────────────────────────────────────────
        if (drive.value > 0.001f) {
            float drv = 1.0f + drive.value * 0.04f; // 1x … 5x pre-gain
            sig = softClip(sig * drv) / drv;
        }

        // ── HP trim ───────────────────────────────────────────────────────────
        if (hpTrim.value > 0.001f) {
            sig = CLAMP(sig, -1.0f, 1.0f);
            float hpRate = 0.0005f + hpTrim.value * 0.0005f;
            hpState += hpRate * (sig - hpState);
            sig = (sig - hpState) * (1.0f + hpTrim.value * 0.015f);
        }

        sig *= 1.0f + gain.value * 0.01f;
        return bufferedFxProcess(sig);
    }
};