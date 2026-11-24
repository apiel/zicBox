#include "audio/engines/Fm4.h"
#include "emu/Fm4Sequencer.h"
#include <algorithm>
#include <cassert>
#include <cmath>
#include <iostream>

int main()
{
    Fm4 fm4(44100.0f);
    std::cout << "Testing Fm4 ...\n";

    // Render a note at the default settings
    fm4.noteOn(60, 1.0f);
    float peak = 0.0f;
    for (int i = 0; i < 44100; ++i) {
        float s = fm4.sample();
        assert(std::isfinite(s));
        peak = std::max(peak, std::fabs(s));
    }
    std::cout << "Peak of a 1 s note at defaults: " << peak << "\n";
    assert(peak > 0.01f);

    // Note off: the release must decay to silence
    fm4.noteOff();
    float tailPeak = 0.0f;
    for (int i = 0; i < 44100 * 2; ++i) {
        float s = std::fabs(fm4.sample());
        if (i >= 44100 * 2 - 44100 / 2) tailPeak = std::max(tailPeak, s);
    }
    std::cout << "Tail peak after the release: " << tailPeak << "\n";
    assert(tailPeak < 0.01f);

    // Back to notes, and check a few algorithms + harder settings render without NaN
    for (int alg = 0; alg < 8; ++alg) {
        fm4.params[0].set((float)alg);
        fm4.noteOn(45, 0.9f);
        float m = 0.0f;
        for (int i = 0; i < 44100; ++i) {
            float s = fm4.sample();
            assert(std::isfinite(s));
            m = std::max(m, std::fabs(s));
        }
        fm4.noteOff();
        for (int i = 0; i < 44100 / 2; ++i) fm4.sample();
        assert(m > 0.001f);
        std::cout << "Alg " << alg + 1 << ": peak " << m << "\n";
    }

    std::cout << "Fm4 DSP test passed!\n";

    // Sequencer: four bars of the default pattern must trigger notes and make sound.
    Fm4 seqSynth(44100.0f);
    Fm4Sequencer seq(44100.0f);
    seq.play();
    int onEvents = 0, offEvents = 0, maxStep = -1;
    int visited[16] = {};
    float seqPeak = 0.0f;
    for (int i = 0; i < 44100 * 4; ++i) {
        Fm4Sequencer::TriggerEvent ev = seq.tick();
        if (ev.noteOn) { ++onEvents; seqSynth.noteOn(ev.note, ev.velocity, ev.slide); }
        if (ev.noteOff) { ++offEvents; seqSynth.noteOff(); }
        int cs = seq.currentStep.load();
        maxStep = std::max(maxStep, cs);
        if (cs >= 0 && cs < 16) visited[cs] = 1;
        seqPeak = std::max(seqPeak, std::fabs(seqSynth.sample()));
    }
    int stepsVisited = 0;
    for (int i = 0; i < 16; ++i) stepsVisited += visited[i];
    std::cout << "Sequencer: on " << onEvents << ", off " << offEvents
              << ", steps " << stepsVisited << "/16, peak " << seqPeak << "\n";
    assert(onEvents >= 14);
    assert(offEvents >= 10);
    assert(stepsVisited == 16);
    assert(maxStep >= 15);
    assert(seqPeak > 0.01f);
    std::cout << "Fm4 sequencer test passed!\n";

    return 0;
}
