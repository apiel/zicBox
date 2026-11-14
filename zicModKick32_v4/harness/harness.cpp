/**
 * Offline test harness for the Kick32 engines (no audio device, no UI).
 *
 *   make -C zicModKick32_v4/harness            # Kick32v4
 *   make -C zicModKick32_v4/harness v3         # same tests on Kick32v3 (reference numbers)
 *
 * Tests:
 *   1. render a hit with the defaults: NaN / inf, peak, DC, length
 *   2. 300 random settings x 4 fast hits on random notes 36..84: NaN / peak <= 0.95 / DC
 *   3. everything at max (and at min): silence 3.5 s after the hit
 *   4. per pot impact: 0 / 50 / 100 % with the other pots at default, then in a "busy" patch.
 *      Impact = spectral distance (1/3 octave bands, dB) and envelope distance (10 ms RMS frames, dB).
 *   5. timing per sample
 */

#include <algorithm>
#include <chrono>
#include <cmath>
#include <complex>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>
#include <vector>

#ifndef ENGINE_HEADER
#define ENGINE_HEADER "audio/engines/Kick32v4.h"
#define ENGINE Kick32v4
#endif
#include ENGINE_HEADER

static constexpr float SR = 44100.0f;
static constexpr int NP = ENGINE::PARAM_COUNT;

struct Rng {
    uint32_t s = 12345;
    float next() { s = s * 1664525u + 1013904223u; return (s >> 8) * (1.0f / 16777216.0f); }
};

static void setNorm(ENGINE& e, int i, float n)
{
    Param& p = e.params[i];
    p.set(p.min + std::clamp(n, 0.0f, 1.0f) * (p.max - p.min));
}
static float getNorm(ENGINE& e, int i)
{
    Param& p = e.params[i];
    return (p.value - p.min) / (p.max - p.min);
}

// render `n` samples, triggering the notes at the given sample positions
static std::vector<float> render(ENGINE& e, int n, const std::vector<std::pair<int, int>>& hits)
{
    std::vector<float> out(n);
    size_t h = 0;
    for (int i = 0; i < n; ++i) {
        while (h < hits.size() && hits[h].first == i) { e.noteOn((uint8_t)hits[h].second, 1.0f); ++h; }
        out[i] = e.sample();
    }
    return out;
}

struct Stats { bool finite = true; float peak = 0, dc = 0, rms = 0; int lastLoud = 0; };
static Stats stats(const std::vector<float>& x, int tailFrom = 0)
{
    Stats s;
    double sum = 0, sq = 0;
    for (size_t i = 0; i < x.size(); ++i) {
        float v = x[i];
        if (!std::isfinite(v)) s.finite = false;
        s.peak = std::max(s.peak, std::abs(v));
        if ((int)i >= tailFrom) { sum += v; }
        sq += (double)v * v;
        if (std::abs(v) > 1e-4f) s.lastLoud = (int)i;
    }
    int tailN = std::max(1, (int)x.size() - tailFrom);
    s.dc = (float)(sum / tailN) / std::max(1e-6f, s.peak);
    s.rms = (float)std::sqrt(sq / x.size());
    return s;
}

// ---- FFT (radix 2, in place)
static void fft(std::vector<std::complex<float>>& a)
{
    int n = (int)a.size();
    for (int i = 1, j = 0; i < n; ++i) {
        int bit = n >> 1;
        for (; j & bit; bit >>= 1) j ^= bit;
        j ^= bit;
        if (i < j) std::swap(a[i], a[j]);
    }
    for (int len = 2; len <= n; len <<= 1) {
        float ang = -2.0f * (float)M_PI / len;
        std::complex<float> wl(std::cos(ang), std::sin(ang));
        for (int i = 0; i < n; i += len) {
            std::complex<float> w(1, 0);
            for (int j = 0; j < len / 2; ++j) {
                auto u = a[i + j], v = a[i + j + len / 2] * w;
                a[i + j] = u + v;
                a[i + j + len / 2] = u - v;
                w *= wl;
            }
        }
    }
}

// 1/3 octave band levels in dB (25 Hz .. 16 kHz) over the first 32768 samples, and 10 ms RMS envelope in dB
struct Fingerprint {
    static constexpr int BANDS = 29, FRAMES = 100;
    float band[BANDS], env[FRAMES];
    float attack[BANDS]; // the same bands over the first 60 ms (clicks, noise, punch)
    float peakBand = -200.0f, peakAttack = -200.0f;
};
static Fingerprint fingerprint(const std::vector<float>& x)
{
    Fingerprint f;
    const int N = 32768;
    std::vector<std::complex<float>> a(N);
    for (int i = 0; i < N; ++i) a[i] = i < (int)x.size() ? x[i] : 0.0f;
    fft(a);
    for (int b = 0; b < Fingerprint::BANDS; ++b) {
        float fc = 25.0f * std::pow(2.0f, b / 3.0f);
        float lo = fc / 1.1225f, hi = fc * 1.1225f;
        int k0 = std::max(1, (int)(lo / SR * N)), k1 = std::min(N / 2, (int)(hi / SR * N) + 1);
        double sum = 0;
        for (int k = k0; k < k1; ++k) sum += std::norm(a[k]);
        f.band[b] = 10.0f * std::log10((float)sum / N + 1e-12f);
        f.peakBand = std::max(f.peakBand, f.band[b]);
    }
    {
        const int M = 4096; // 93 ms window, the first 60 ms with a fade
        std::vector<std::complex<float>> w(M);
        int att = (int)(SR * 0.06f);
        for (int i = 0; i < M; ++i) {
            float fade = i < att ? 1.0f : std::max(0.0f, 1.0f - (float)(i - att) / (M - att));
            w[i] = i < (int)x.size() ? x[i] * fade : 0.0f;
        }
        fft(w);
        for (int b = 0; b < Fingerprint::BANDS; ++b) {
            float fc = 25.0f * std::pow(2.0f, b / 3.0f);
            float lo = fc / 1.1225f, hi = fc * 1.1225f;
            int k0 = std::max(1, (int)(lo / SR * M)), k1 = std::min(M / 2, (int)(hi / SR * M) + 1);
            double sum = 0;
            for (int k = k0; k < k1; ++k) sum += std::norm(w[k]);
            f.attack[b] = 10.0f * std::log10((float)sum / M + 1e-12f);
            f.peakAttack = std::max(f.peakAttack, f.attack[b]);
        }
    }
    int frame = (int)(SR * 0.01f);
    for (int i = 0; i < Fingerprint::FRAMES; ++i) {
        double sq = 0;
        for (int j = 0; j < frame; ++j) {
            int k = i * frame + j;
            if (k < (int)x.size()) sq += (double)x[k] * x[k];
        }
        f.env[i] = 10.0f * std::log10((float)(sq / frame) + 1e-10f);
    }
    return f;
}

// distance between two fingerprints: mean absolute dB difference over the bands / frames that matter (above -50 dB rel. peak)
struct Dist { float spec = 0, env = 0, attack = 0, specMax = 0, envMax = 0; };
static Dist distance(const Fingerprint& a, const Fingerprint& b)
{
    Dist d;
    float floorS = std::max(a.peakBand, b.peakBand) - 50.0f;
    int n = 0;
    for (int i = 0; i < Fingerprint::BANDS; ++i) {
        if (a.band[i] < floorS && b.band[i] < floorS) continue;
        float x = std::abs(std::max(a.band[i], floorS) - std::max(b.band[i], floorS));
        d.spec += x; d.specMax = std::max(d.specMax, x); ++n;
    }
    d.spec /= std::max(1, n);
    float floorA = std::max(a.peakAttack, b.peakAttack) - 50.0f;
    n = 0;
    for (int i = 0; i < Fingerprint::BANDS; ++i) {
        if (a.attack[i] < floorA && b.attack[i] < floorA) continue;
        d.attack += std::abs(std::max(a.attack[i], floorA) - std::max(b.attack[i], floorA));
        ++n;
    }
    d.attack /= std::max(1, n);
    n = 0;
    for (int i = 0; i < Fingerprint::FRAMES; ++i) {
        if (a.env[i] < -50.0f && b.env[i] < -50.0f) continue;
        float x = std::abs(std::max(a.env[i], -50.0f) - std::max(b.env[i], -50.0f));
        d.env += x; d.envMax = std::max(d.envMax, x); ++n;
    }
    d.env /= std::max(1, n);
    return d;
}

static void writeWav(const char* path, const std::vector<float>& x)
{
    FILE* f = fopen(path, "wb");
    if (!f) return;
    int32_t sr = (int32_t)SR, dataBytes = (int32_t)x.size() * 2, fmt = 16, riff = 36 + dataBytes;
    int16_t pcm = 1, ch = 1, bits = 16, align = 2;
    int32_t byteRate = sr * 2;
    fwrite("RIFF", 1, 4, f); fwrite(&riff, 4, 1, f); fwrite("WAVEfmt ", 1, 8, f); fwrite(&fmt, 4, 1, f);
    fwrite(&pcm, 2, 1, f); fwrite(&ch, 2, 1, f); fwrite(&sr, 4, 1, f); fwrite(&byteRate, 4, 1, f); fwrite(&align, 2, 1, f); fwrite(&bits, 2, 1, f);
    fwrite("data", 1, 4, f); fwrite(&dataBytes, 4, 1, f);
    for (float v : x) { int16_t s = (int16_t)(std::clamp(v, -1.0f, 1.0f) * 32767.0f); fwrite(&s, 2, 1, f); }
    fclose(f);
}

int main(int argc, char** argv)
{
    const char* wavDir = argc > 1 ? argv[1] : nullptr; // optional: write a few wavs there
    int failures = 0;
    printf("== %s: %d params, sample rate %.0f\n", ENGINE(SR).getName(), NP, SR);

    // defaults for later
    std::vector<float> defaults(NP);
    { ENGINE e(SR); for (int i = 0; i < NP; ++i) defaults[i] = getNorm(e, i); }
    auto fresh = [&](const std::vector<float>* norms) {
        ENGINE* e = new ENGINE(SR);
        if (norms) for (int i = 0; i < NP; ++i) setNorm(*e, i, (*norms)[i]);
        return e;
    };

    // 1. default hit
    {
        ENGINE* e = fresh(nullptr);
        auto x = render(*e, (int)SR, { { 0, 60 } });
        auto s = stats(x, (int)(SR * 0.8f));
        printf("[1] default hit: finite %d  peak %.3f  rms %.3f  dc/peak %.5f  last loud %.0f ms\n", s.finite, s.peak, s.rms, s.dc, s.lastLoud / SR * 1000);
        if (!s.finite || s.peak > 0.951f) failures++;
        if (wavDir) writeWav((std::string(wavDir) + "/default.wav").c_str(), x);
        delete e;
    }

    // 2. random settings
    {
        Rng rng;
        float worstPeak = 0, worstDc = 0;
        int nan = 0;
        for (int r = 0; r < 300; ++r) {
            std::vector<float> n(NP);
            for (auto& v : n) v = rng.next();
            ENGINE* e = fresh(&n);
            std::vector<std::pair<int, int>> hits;
            for (int h = 0; h < 4; ++h) hits.push_back({ (int)(h * SR * 0.125f), 36 + (int)(rng.next() * 48.99f) });
            auto x = render(*e, (int)(SR * 1.2f), hits);
            auto s = stats(x, 0);
            if (!s.finite) nan++;
            worstPeak = std::max(worstPeak, s.peak);
            worstDc = std::max(worstDc, std::abs(s.dc));
            if (wavDir && r < 8) writeWav((std::string(wavDir) + "/random" + std::to_string(r) + ".wav").c_str(), x);
            delete e;
        }
        printf("[2] 300 random settings x 4 fast hits: nan %d  worst peak %.3f  worst |dc/peak| %.4f\n", nan, worstPeak, worstDc);
        if (nan || worstPeak > 0.951f) failures++;
    }

    // 3. silence after a hit with everything at max / min / default
    for (int mode = 0; mode < 3; ++mode) {
        std::vector<float> n(NP, mode == 0 ? 1.0f : 0.0f);
        if (mode == 2) n = defaults;
        ENGINE* e = fresh(&n);
        auto x = render(*e, (int)(SR * 4.0f), { { 0, 60 } });
        double sq = 0;
        int from = (int)(SR * 3.5f);
        for (int i = from; i < (int)x.size(); ++i) sq += (double)x[i] * x[i];
        float db = 10.0f * std::log10(sq / (x.size() - from) + 1e-20);
        auto s = stats(x);
        printf("[3] all %s: finite %d  peak %.3f  level at 3.5 s %.1f dB  last > -80 dB at %.0f ms\n", mode == 0 ? "max" : (mode == 1 ? "min" : "default"), s.finite, s.peak, db, s.lastLoud / SR * 1000);
        if (!s.finite || db > -80.0f || s.peak > 0.951f) failures++;
        if (wavDir) writeWav((std::string(wavDir) + (mode == 0 ? "/allmax.wav" : (mode == 1 ? "/allmin.wav" : "/alldefault.wav"))).c_str(), x);
        delete e;
    }

    // 4. per pot impact
    {
        // a "busy" patch: a hardtek-ish kick where every block is engaged, so dependent pots (FM Ratio, Color, Env Decay...) can show
        std::vector<float> busy = defaults;
        {
            ENGINE e(SR);
            struct KV { const char* key; float v; };
            static const KV kv[] = { { "decay", 0.2f }, { "shape", 0.4f }, { "wave", 0.35f }, { "harm2", 0.65f }, { "skew", 0.6f }, { "sync", 0.3f },
                { "resonator", 0.3f }, { "fold", 0.2f }, { "fmDepth", 0.35f }, { "bend", 0.6f }, { "shift", 0.6f }, { "vowel", 0.3f }, { "rectify", 0.2f },
                { "drive", 0.55f }, { "character", 0.35f }, { "punch", 0.35f }, { "feedback", 0.6f }, { "click", 0.5f }, { "noise", 0.35f },
                { "cutoff", 0.75f }, { "cutoff2", 0.15f }, { "reso", 0.5f }, { "env", 0.65f }, { "filterFm", 0.3f }, { "filterDrive", 0.4f }, { "crush", 0.0f } };
            for (auto& x : kv)
                for (int i = 0; i < NP; ++i)
                    if (!strcmp(e.params[i].key, x.key)) busy[i] = x.v;
        }
        std::vector<float> hard = defaults;
        {
            ENGINE e(SR);
            struct KV { const char* key; float v; };
            static const KV kv[] = { { "decay", 0.25f }, { "shape", 0.6f }, { "drive", 0.75f }, { "character", 0.4f }, { "punch", 0.25f }, { "filterDrive", 0.3f } };
            for (auto& x : kv)
                for (int i = 0; i < NP; ++i)
                    if (!strcmp(e.params[i].key, x.key)) hard[i] = x.v;
        }
        printf("[4] per pot impact, dB (spectrum / envelope / attack spectrum (first 60 ms); max over 0-50-100 %%)  [context: default | busy | hardcore]\n");
        printf("    %-3s %-12s %6s %6s %6s   %6s %6s %6s   %6s %6s %6s   %s\n", "#", "pot", "spec", "env", "attk", "spec", "env", "attk", "spec", "env", "attk", "flag");
        float sumBest = 0;
        int weak = 0;
        for (int p = 0; p < NP; ++p) {
            Dist best[3];
            for (int ctx = 0; ctx < 3; ++ctx) {
                Fingerprint fp[3];
                for (int v = 0; v < 3; ++v) {
                    std::vector<float> n = ctx == 0 ? defaults : (ctx == 1 ? busy : hard);
                    n[p] = v * 0.5f;
                    ENGINE* e = fresh(&n);
                    auto x = render(*e, (int)SR, { { 0, 60 } });
                    fp[v] = fingerprint(x);
                    delete e;
                }
                for (int a = 0; a < 3; ++a)
                    for (int b = a + 1; b < 3; ++b) {
                        Dist d = distance(fp[a], fp[b]);
                        best[ctx].spec = std::max(best[ctx].spec, d.spec);
                        best[ctx].env = std::max(best[ctx].env, d.env);
                        best[ctx].attack = std::max(best[ctx].attack, d.attack);
                    }
            }
            float score = 0;
            for (auto& b : best) score = std::max({ score, b.spec, b.env, b.attack });
            sumBest += score;
            const char* flag = score < 1.5f ? "WEAK" : (score < 3.0f ? "soft" : "");
            if (score < 1.5f) weak++;
            ENGINE e(SR);
            printf("    %-3d %-12s", p, e.params[p].label);
            for (auto& b : best) printf(" %6.1f %6.1f %6.1f  ", b.spec, b.env, b.attack);
            printf(" %s\n", flag);
        }
        printf("    mean impact %.1f dB, weak pots (< 1.5 dB): %d\n", sumBest / NP, weak);
    }

    // 6. morph flatness: loudness over the pot range (defaults context). A hump in the middle of a morph is a problem.
    {
        printf("[6] loudness over the pot range, dB rel. position 0 (11 steps, default context); hump = a middle step louder than both ends by > 2.5 dB\n");
        int humps = 0;
        for (int p = 0; p < NP; ++p) {
            float db[11];
            for (int v = 0; v <= 10; ++v) {
                std::vector<float> n = defaults;
                n[p] = v * 0.1f;
                ENGINE* e = fresh(&n);
                auto x = render(*e, (int)(SR * 0.6f), { { 0, 60 } });
                db[v] = 20.0f * std::log10(stats(x).rms + 1e-9f);
                delete e;
            }
            float ends = std::max(db[0], db[10]), mid = -200;
            for (int v = 1; v < 10; ++v) mid = std::max(mid, db[v]);
            bool hump = mid > ends + 2.5f;
            if (hump) humps++;
            ENGINE e(SR);
            printf("    %-3d %-12s", p, e.params[p].label);
            for (int v = 0; v <= 10; ++v) printf(" %5.1f", db[v] - db[0]);
            printf("   %s\n", hump ? "HUMP" : "");
        }
        printf("    humps: %d\n", humps);
    }

    // 7. pitch track (zero crossings) at 15 / 60 / 250 ms for the drop and the bend, note 60 (52 Hz), gated shape so the tail is loud
    {
        auto f0At = [&](const std::vector<float>& x, float ms) {
            int c = (int)(SR * ms * 0.001f), w = (int)(SR * 0.035f);
            int first = -1, last = -1, n = 0;
            for (int i = std::max(1, c - w); i < std::min((int)x.size(), c + w); ++i)
                if (x[i - 1] < 0 && x[i] >= 0) { if (first < 0) first = i; last = i; n++; }
            return n > 1 ? SR * (n - 1) / (last - first) : 0.0f;
        };
        struct Case { const char* name; const char* key; float v; };
        static const Case cases[] = { { "defaults", nullptr, 0 }, { "drop 0", "drop", 0.0f }, { "drop 100", "drop", 1.0f }, { "bend -100", "bend", 0.0f }, { "bend +100", "bend", 1.0f }, { "bend +50", "bend", 0.75f } };
        printf("[7] pitch track, Hz at 15 / 60 / 250 / 600 ms (shape gated, decay 700 ms, drop time default):\n");
        for (auto& cs : cases) {
            std::vector<float> n = defaults;
            ENGINE e0(SR);
            for (int i = 0; i < NP; ++i) {
                if (!strcmp(e0.params[i].key, "shape")) n[i] = 1.0f;
                if (!strcmp(e0.params[i].key, "decay")) n[i] = (700.0f - 40.0f) / (2000.0f - 40.0f);
                if (!strcmp(e0.params[i].key, "cutoff")) n[i] = 1.0f;
                if (!strcmp(e0.params[i].key, "click") || !strcmp(e0.params[i].key, "noise")) n[i] = 0.0f;
                if (cs.key && !strcmp(e0.params[i].key, cs.key)) n[i] = cs.v;
            }
            bool has = cs.key == nullptr;
            for (int i = 0; i < NP; ++i) if (cs.key && !strcmp(e0.params[i].key, cs.key)) has = true;
            if (!has) continue;
            ENGINE* e = fresh(&n);
            auto x = render(*e, (int)SR, { { 0, 60 } });
            printf("    %-10s %7.1f %7.1f %7.1f %7.1f\n", cs.name, f0At(x, 15), f0At(x, 60), f0At(x, 250), f0At(x, 600));
            delete e;
        }
    }

    // 5. timing
    {
        Rng rng;
        rng.s = 777;
        std::vector<float> n(NP);
        for (auto& v : n) v = 0.3f + 0.5f * rng.next();
        ENGINE* e = fresh(&n);
        std::vector<std::pair<int, int>> hits;
        for (int h = 0; h < 40; ++h) hits.push_back({ (int)(h * SR * 0.25f), 48 + (h % 12) });
        auto t0 = std::chrono::steady_clock::now();
        auto x = render(*e, (int)(SR * 10), hits);
        auto t1 = std::chrono::steady_clock::now();
        double ns = std::chrono::duration<double, std::nano>(t1 - t0).count() / x.size();
        printf("[5] timing: %.0f ns / sample (%.1f%% of a 44.1 kHz sample period on this x86)   checksum %.3f\n", ns, ns / (1e9 / SR) * 100, stats(x).rms);
        delete e;
    }

    printf("== %s\n", failures ? "FAILURES" : "all checks passed");
    return failures ? 1 : 0;
}
