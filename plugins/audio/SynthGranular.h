#ifndef _SYNTH_GRANULAR_H_
#define _SYNTH_GRANULAR_H_

#include <math.h>
#include <sndfile.h>
#include <time.h>

#include "audioPlugin.h"
#include "fileBrowser.h"
#include "mapping.h"

#include "utils/ValSerializeSndFile.h"

#define GRANULAR_BUFFER_SECONDS 30
#define MAX_GRAINS_PER_VOICE 24
#define MAX_GRAIN_VOICES 4
#define MIN_GRAIN_SIZE_MS 20

class SynthGranular : public Mapping {
protected:
    uint64_t sampleRate;
    uint16_t minGrainSampleCount;
    // We hardcode it to 48000, no matter the sample rate
    static const uint64_t bufferSize = 48000 * GRANULAR_BUFFER_SECONDS; // 30sec at 48000Hz, 32sec at 44100Hz...
    uint64_t bufferSampleCount = 0;
    float bufferSamples[bufferSize];

    FileBrowser fileBrowser = FileBrowser("./samples");
    // FileBrowser fileBrowser;

    uint64_t voicePosition = 0;
    float attackStep = 0.0f;
    float releaseStep = 0.0f;

    uint8_t densityUint8 = 4;
    int8_t pitchSemitone = 0;

    struct Grain {
        float pos;
        int64_t start;
        int64_t sampleCount;
        int64_t delay;
        float sampleStep;
    };

    struct Voice {
        int8_t note = -1;
        Grain grains[MAX_GRAINS_PER_VOICE];
        float (SynthGranular::*envelop)(Voice& voice) = &SynthGranular::envelopSustain;
        float env = 0.0f;
        uint64_t position = 0;
    } voices[MAX_GRAIN_VOICES];

    uint8_t baseNote = 60;
    float getSampleStep(uint8_t note)
    {
        // https://gist.github.com/YuxiUx/ef84328d95b10d0fcbf537de77b936cd
        // pow(2, ((note - 0) / 12.0)) = 1 for 0 semitone
        // pow(2, ((note - 1) / 12.0)) = 1.059463094 for 1 semitone
        // pow(2, ((note - 2) / 12.0)) = 1.122462048 for 2 semitone
        // ...
        return pow(2, ((note - baseNote) / 12.0));
    }

    void setSampleRate(uint64_t _sampleRate)
    {
        sampleRate = _sampleRate;
        minGrainSampleCount = MIN_GRAIN_SIZE_MS * sampleRate * 0.001f;
    }

    void initGrain(Grain& grain)
    {
        initGrain(grain, grain.sampleStep);
    }

    int randCounter = 0;
    int getRand()
    {
        // could create a lookup table ?
        srand(time(NULL) + randCounter++);
        return rand();
    }

    float getRandPct()
    {
        return getRand() / (float)RAND_MAX;
    }

    void initGrain(Grain& grain, float sampleStep)
    {
        grain.sampleStep = sampleStep;
        grain.pos = 0.0f;

        // sprayToAdd is a random value between 0 and spray from starting point till end of file
        float sprayToAdd = spray.pct() ? (getRandPct() * spray.pct() * (1 - start.pct())) : 0.0;
        grain.start = (start.pct() + sprayToAdd) * bufferSampleCount;

        // we deduct minGrainSampleCount to avoid grainSize to be too small
        grain.sampleCount = (bufferSampleCount - (grain.start + minGrainSampleCount)) * grainSize.pct() + minGrainSampleCount;

        // delayInt = delay.get() * SAMPLE_RATE * 0.001f * 1000;
        // can be simplified to:
        // delayInt = delay.get() * SAMPLE_RATE;
        grain.delay = delay.pct() > 0 ? (getRand() % (int)(delay.pct() * sampleRate)) : 0;

        // debug("initGrain: grain.start %d grain.sampleCount %d grain.delay %d\n", grain.start, grain.sampleCount, grain.delay);
    }

    float envelopAttack(Voice& voice)
    {
        voice.env += attackStep;
        if (voice.env >= 1.0f) {
            voice.env = 1.0f;
            voice.envelop = &SynthGranular::envelopSustain;
            // debug("envelopAttack finished, set env to %f\n", voice.env);
        }
        return voice.env;
    }

    float envelopSustain(Voice& voice)
    {
        return voice.env;
    }

    float envelopRelease(Voice& voice)
    {
        voice.env -= releaseStep;
        if (voice.env <= 0.0f) {
            voice.env = 0.0f;
            voice.note = -1;
            // debug("envelopRelease finished, set env to %f\n", voice.env);
        }
        return voice.env;
    }

    float sample(Voice& voice)
    {
        float sample = 0.0f;
        float env = (this->*voice.envelop)(voice);
        if (env <= 0.0f) {
            return sample;
        }

        uint8_t grainOff = 0;
        for (uint8_t d = 0; d < densityUint8; d++) {
            Grain& grain = voice.grains[d];
            if (grain.delay > 0) {
                grain.delay--;
            } else {
                int64_t samplePos = (uint64_t)grain.pos + grain.start;
                if ((int64_t)grain.pos < grain.sampleCount) {
                    grain.pos += grain.sampleStep; // randomize? + (getRandPct() * grain.sampleStep - (grain.sampleStep * 0.5f));
                    sample += bufferSamples[samplePos] * env;
                } else if (repeat.get()) {
                    initGrain(grain);
                } else {
                    grainOff++;
                }
            }
        }
        if (grainOff == densityUint8) {
            voice.note = -1;
        }
        return sample;
    }

    Voice& getNextVoice(uint8_t note)
    {
        // First, we should look if the voice is not already running, due to the envelopRelease
        for (uint8_t v = 0; v < MAX_GRAIN_VOICES; v++) {
            Voice& voice = voices[v];
            if (voice.note == note) {
                debug("getNextVoice: voice already running %d\n", note);
                return voice;
            }
        }

        // Else, we should look for a free voice
        uint8_t voiceToSteal = 0;
        for (uint8_t v = 0; v < MAX_GRAIN_VOICES; v++) {
            Voice& voice = voices[v];
            if (voice.note == -1) {
                return voice;
            }
            if (voice.position < voices[voiceToSteal].position) {
                voiceToSteal = v;
            }
        }

        debug("getNextVoice: no voice available. Steal voice %d.\n", voiceToSteal);
        return voices[voiceToSteal];
    }

public:
    Val& start = val(0.0f, "START", [&](float value) { setStart(value); }, { "Start", .unit = "%" });
    Val& spray = val(0.0f, "SPRAY", [&](float value) { setSpray(value); }, { "Spray", .unit = "%" });
    Val& grainSize = val(100.0f, "GRAIN_SIZE", [&](float value) { setGrainSize(value); }, { "Size", .unit = "%" });
    Val& density = val((float)densityUint8, "DENSITY", [&](float value) { setDensity(value); }, { "Density", .min = 1.0, .max = MAX_GRAINS_PER_VOICE });
    Val& attack = val(20, "ATTACK", [&](float value) { setAttack(value); }, { "Attack", .min = 20.0, .max = 5000.0, .step = 20.0, .unit = "ms" });
    Val& release = val(50, "RELEASE", [&](float value) { setRelease(value); }, { "Release", .min = 50.0, .max = 10000.0, .step = 50.0, .unit = "ms" });
    Val& delay = val(0.0f, "DELAY", [&](float value) { setDelay(value); }, { "Delay", .max = 1000.0f, .step = 10.0f, .unit = "ms" });
    Val& pitch = val(0.0f, "PITCH", [&](float value) { setPitch(value); }, { "Pitch", VALUE_CENTERED, .min = -12.0, .max = 12.0 });
    Val& browser = val(0.0f, "BROWSER", [&](float value) { open(value); }, { "Browser", VALUE_STRING, .max = (float)fileBrowser.count });
    Val& repeat = val(1.0f, "REPEAT", [&](float value) { setRepeat(value); }, { "Repeat", VALUE_STRING, .max = 1.0 });

    // TODO add pitch randomization per grain

    SynthGranular(AudioPlugin::Props& props, char* _name)
        : Mapping(props, _name)
    {
        setSampleRate(props.sampleRate);
        open(browser.get(), true);

        setAttack(attack.get());
        setRelease(release.get());

        setRepeat(repeat.get());
    }

    bool config(char* key, char* value) override
    {
        if (strcmp(key, "SAMPLES_FOLDER") == 0) {
            debug("Granular SAMPLES_FOLDER: %s\n", value);
            fileBrowser.openFolder(value);
            browser.props().max = fileBrowser.count;
            open(0.0, true);

            return true;
        }
        return AudioPlugin::config(key, value);
    }

    SynthGranular& save()
    {
        ValSerializeSndFile serialize(mapping);
        char* filepath = fileBrowser.getFilePath(browser.get());
        serialize.saveSetting(filepath);
        return *this;
    }

    SynthGranular& open(const char* filename)
    {
        SF_INFO sfinfo;
        SNDFILE* file = sf_open(filename, SFM_READ, &sfinfo);
        if (!file) {
            debug("Error: could not open file %s [%s]\n", filename, sf_strerror(file));
            return *this;
        }

        debug("Audio file %s sampleCount %ld sampleRate %d\n", filename, (long)sfinfo.frames, sfinfo.samplerate);

        bufferSampleCount = sf_read_float(file, bufferSamples, bufferSize);

        sf_close(file);

        ValSerializeSndFile serialize(mapping);
        serialize.loadSetting(filename);

        return *this;
    }

    SynthGranular& open(float value, bool force)
    {
        browser.setFloat(value);
        int position = browser.get();
        if (force || position != fileBrowser.position) {
            char* filepath = fileBrowser.getFilePath(position);
            browser.setString(fileBrowser.getFile(position));
            debug("GRANULAR_SAMPLE_SELECTOR: %f %s\n", value, filepath);
            open(filepath);
        }
        return *this;
    }

    SynthGranular& open(float value)
    {
        open(value, false);
        return *this;
    }

    SynthGranular& setRepeat(float value)
    {
        repeat.setFloat(value);
        repeat.setString(repeat.get() ? "ON" : "OFF");
        return *this;
    }

    SynthGranular& setPitch(float value)
    {
        pitch.setFloat(value);
        pitchSemitone = pitch.get();
        debug("pitch %d\n", pitchSemitone);
        for (uint8_t v = 0; v < MAX_GRAIN_VOICES; v++) {
            for (uint8_t g = 0; g < MAX_GRAINS_PER_VOICE; g++) {
                voices[v].grains[g].sampleStep = getSampleStep(voices[v].note + pitchSemitone);
            }
        }
        return *this;
    }

    /**
     * @brief Set the Grain Size meaning the length duration of the grain.
     *
     * @param grainSize
     * @return SynthGranular&
     */
    SynthGranular& setGrainSize(float value)
    {
        grainSize.setFloat(value);
        debug("grainSize %f\n", grainSize.get());
        return *this;
    }

    /**
     * @brief Set the Spray of the grain start position, giving random position to
     * the grain start position.
     *
     * @param spray
     * @return SynthGranular&
     */
    SynthGranular& setSpray(float value)
    {
        spray.setFloat(value);
        // debug("spray %f\n", spray.get());
        return *this;
    }

    /**
     * @brief Set the Density meaning the number of grains that are played at the same time.
     *
     * @param density
     * @return SynthGranular&
     */
    SynthGranular& setDensity(float value)
    {
        density.setFloat(value);
        densityUint8 = density.get();
        debug("density %d\n", densityUint8);
        return *this;
    }

    /**
     * @brief Set the Start position of the sample to play
     *
     * @param _start position from 0.0 to 1.0, where 0.0 is the start of the sample
     * and 1.0 the end of the sample
     * @return SynthGranular&
     */
    SynthGranular& setStart(float value)
    {
        start.setFloat(value);
        // debug("setStart %f\n", start.get());
        return *this;
    }

    /**
     * @brief Set the Delay before grain start to play
     *
     * @param delay where 0 is no delay and 1 is 1000ms
     * @return SynthGranular&
     */
    SynthGranular& setDelay(float value)
    {
        delay.setFloat(value);
        debug("delay %f ms\n", delay.get());
        return *this;
    }

    /**
     * @brief Set the Attack time of the voice
     *
     * @param attack where 0 is no attack and 1 is 5000ms
     * @return SynthGranular&
     */
    SynthGranular& setAttack(float value)
    {
        attack.setFloat(value);
        // uint64_t attackSamples = attack.pct() * SAMPLE_RATE * 0.001f * 5000;
        // can be simplified to:
        uint64_t attackSamples = attack.pct() * sampleRate * 5;
        attackStep = 1.0f / attackSamples;
        debug("attack %ld samples %f step\n", attackSamples, attackStep);
        return *this;
    }

    /**
     * @brief Set the Release time of the voice
     *
     * @param release where 0 is no release and 1 is 10000ms
     * @return SynthGranular&
     */
    SynthGranular& setRelease(float value)
    {
        release.setFloat(value);
        // uint64_t releaseSamples = release.pct() * SAMPLE_RATE * 0.001f * 10000;
        // can be simplified to:
        uint64_t releaseSamples = release.pct() * sampleRate * 10;
        releaseStep = 1.0f / releaseSamples;
        debug("release %ld samples %f step\n", releaseSamples, releaseStep);
        return *this;
    }

    void sample(float* buf)
    {
        float s = 0.0f;
        for (uint8_t v = 0; v < MAX_GRAIN_VOICES; v++) {
            Voice& voice = voices[v];
            if (voice.note != -1) {
                s += sample(voice);
            }
        }
        buf[track] = s;
    }

    void noteOn(uint8_t note, uint8_t velocity) override
    {
        if (velocity == 0) {
            return noteOff(note, velocity);
        }

        Voice& voice = getNextVoice(note);
        voice.position = voicePosition++;
        voice.note = note;
        voice.envelop = &SynthGranular::envelopAttack;
        float sampleStep = getSampleStep(note + pitchSemitone);
        for (uint8_t g = 0; g < densityUint8; g++) {
            initGrain(voice.grains[g], sampleStep);
        }
        debug("noteOn: %d %d %f\n", note, velocity, sampleStep);
    }

    void noteOff(uint8_t note, uint8_t velocity) override
    {
        if (!repeat.get()) { // Play the whole sample if repeat is off
            return;
        }
        for (uint8_t v = 0; v < MAX_GRAIN_VOICES; v++) {
            Voice& voice = voices[v];
            if (voice.note == note) {
                voice.envelop = &SynthGranular::envelopRelease;
                debug("noteOff set on to false: %d %d\n", note, velocity);
                return;
            }
        }

        debug("noteOff: note not found %d %d\n", note, velocity);
    }

protected:
    struct GrainState {
        int index;
        float position;
        float release;
    };
    std::vector<GrainState> grainStates;

public:
    void* data(int id, void* userdata = NULL)
    {
        switch (id) {
        case 0:
            return &bufferSampleCount;

        case 1:
            return &bufferSamples;

        case 2:
            save();
            return NULL;

        case 3: {
            grainStates.clear();
            for (uint8_t v = 0; v < MAX_GRAIN_VOICES; v++) {
                Voice& voice = voices[v];
                if (voice.note != -1) {
                    for (uint8_t g = 0; g < densityUint8; g++) {
                        Grain& grain = voice.grains[g];
                        grainStates.push_back({ v * g + g,
                            (grain.start + grain.pos) / (float)bufferSampleCount,
                            voice.envelop == &SynthGranular::envelopRelease ? voice.env : 1.0f });
                    }
                }
            }
            return &grainStates;
        }

        case 4: {
            // This could be cache in envelopRelease
            for (Voice& voice : voices) {
                if (voice.note != -1) {
                    return &voice;
                }
            }
            return NULL;
        }
        }
        return NULL;
    }
};

#endif