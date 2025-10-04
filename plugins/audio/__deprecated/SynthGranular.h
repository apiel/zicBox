#pragma once

#include <math.h>
#include <sndfile.h>

#include "audioPlugin.h"
#include "fileBrowser.h"
#include "mapping.h"
#include "utils/AsrEnvelop.h"
#include "utils/ValSerializeSndFile.h"
#include "helpers/random.h"
#include "log.h"
#include "host/constants.h"

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

    FileBrowser fileBrowser = FileBrowser(CURRENT_REPO_FOLDER + "/audio/samples");
    // FileBrowser fileBrowser;

    uint64_t voicePosition = 0;
    float attackStep = 0.0f;
    float releaseStep = 0.0f;

    uint8_t densityUint8 = 4;
    int8_t pitchSemitone = 0;

    Random random;

    struct Grain {
        float pos = 0.0f;
        int64_t start = 0;
        int64_t sampleCount = 0;
        int64_t delay = 0;
        float sampleStep = 0.0f;
    };

    struct Voice {
        AsrEnvelop envelop;
        int8_t note = -1;
        Grain grains[MAX_GRAINS_PER_VOICE];
        uint64_t position = 0;
    } voices[MAX_GRAIN_VOICES] = {
        { AsrEnvelop(&attackStep, &releaseStep, [&]() { voices[0].note = -1; }) },
        { AsrEnvelop(&attackStep, &releaseStep, [&]() { voices[1].note = -1; }) },
        { AsrEnvelop(&attackStep, &releaseStep, [&]() { voices[2].note = -1; }) },
        { AsrEnvelop(&attackStep, &releaseStep, [&]() { voices[3].note = -1; }) },
    };

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

    void initGrain(Grain& grain, float sampleStep)
    {
        grain.sampleStep = sampleStep;
        grain.pos = 0.0f;

        // sprayToAdd is a random value between 0 and spray from starting point till end of file
        float sprayToAdd = spray.pct() ? (random.pct() * spray.pct() * (1 - start.pct())) : 0.0;
        grain.start = (start.pct() + sprayToAdd) * bufferSampleCount;

        // we deduct minGrainSampleCount to avoid grainSize to be too small
        grain.sampleCount = (bufferSampleCount - (grain.start + minGrainSampleCount)) * grainSize.pct() + minGrainSampleCount;

        // delayInt = delay.get() * SAMPLE_RATE * 0.001f * 1000;
        // can be simplified to:
        // delayInt = delay.get() * SAMPLE_RATE;
        grain.delay = delay.pct() > 0 ? (random.get() % (int)(delay.pct() * sampleRate)) : 0;

        logTrace("initGrain: grain.start %d grain.sampleCount %d grain.delay %d\n", grain.start, grain.sampleCount, grain.delay);
    }

    float sample(Voice& voice)
    {
        float env = voice.envelop.next();
        if (env <= 0.0f) {
            return 0.0f;
        }

        float sample = 0.0f;
        uint8_t grainOff = 0;
        for (uint8_t d = 0; d < densityUint8; d++) {
            Grain& grain = voice.grains[d];
            if (grain.delay > 0) {
                grain.delay--;
            } else {
                int64_t samplePos = (uint64_t)grain.pos + grain.start;
                if ((int64_t)grain.pos < grain.sampleCount) {
                    grain.pos += grain.sampleStep; // randomize? + (randon.pct(() * grain.sampleStep - (grain.sampleStep * 0.5f));
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
                logDebug("getNextVoice: voice already running %d\n", note);
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

        logDebug("getNextVoice: no voice available. Steal voice %d.\n", voiceToSteal);
        return voices[voiceToSteal];
    }

public:
    Val& start = val(0.0f, "START", { "Start", .unit = "%" });
    Val& spray = val(0.0f, "SPRAY", { "Spray", .unit = "%" });
    Val& grainSize = val(100.0f, "GRAIN_SIZE", { "Size", .unit = "%" });
    Val& density = val((float)densityUint8, "DENSITY", { "Density", .min = 1.0, .max = MAX_GRAINS_PER_VOICE }, [&](auto p) { setDensity(p.value); });
    Val& attack = val(20, "ATTACK", { "Attack", .min = 20.0, .max = 5000.0, .step = 20.0, .unit = "ms" }, [&](auto p) { setAttack(p.value); });
    Val& release = val(50, "RELEASE", { "Release", .min = 50.0, .max = 10000.0, .step = 50.0, .unit = "ms" }, [&](auto p) { setRelease(p.value); });
    Val& delay = val(0.0f, "DELAY", { "Delay", .max = 1000.0f, .step = 10.0f, .unit = "ms" });
    Val& pitch = val(0.0f, "PITCH", { "Pitch", VALUE_CENTERED, .min = -12.0, .max = 12.0 }, [&](auto p) { setPitch(p.value); });
    Val& browser = val(1.0f, "BROWSER", { "Browser", VALUE_STRING, .min = 1.0f, .max = (float)fileBrowser.count }, [&](auto p) { open(p.value); });
    Val& repeat = val(1.0f, "REPEAT", { "Repeat", VALUE_STRING, .max = 1.0 }, [&](auto p) { setRepeat(p.value); });

    // TODO add pitch randomization per grain

    SynthGranular(AudioPlugin::Props& props, AudioPlugin::Config& config)
        : Mapping(props, config)
    {
        setSampleRate(props.sampleRate);
        open(browser.get(), true);

        initValues();

        auto& json = config.json;
        if (json.contains("samplesFolder")) {
            fileBrowser.openFolder(json["samplesFolder"].get<std::string>());
            browser.props().max = fileBrowser.count;
            open(0.0, true);
        }
    }

    SynthGranular& save()
    {
        ValSerializeSndFile serialize(mapping);
        std::string filepath = fileBrowser.getFilePath(browser.get());
        serialize.saveSetting(filepath.c_str());
        return *this;
    }

    SynthGranular& open(std::string filename)
    {
        SF_INFO sfinfo;
        SNDFILE* file = sf_open(filename.c_str(), SFM_READ, &sfinfo);
        if (!file) {
            logDebug("Error: could not open file %s [%s]\n", filename, sf_strerror(file));
            return *this;
        }

        logDebug("Audio file %s sampleCount %ld sampleRate %d\n", filename, (long)sfinfo.frames, sfinfo.samplerate);

        bufferSampleCount = sf_read_float(file, bufferSamples, bufferSize);

        sf_close(file);

        ValSerializeSndFile serialize(mapping);
        serialize.loadSetting(filename.c_str());

        return *this;
    }

    SynthGranular& open(float value, bool force)
    {
        browser.setFloat(value);
        int position = browser.get();
        if (force || position != fileBrowser.position) {
            browser.setString(fileBrowser.getFile(position));
            std::string filepath = fileBrowser.getFilePath(position);
            logDebug("GRANULAR_SAMPLE_SELECTOR: %f %s\n", value, filepath.c_str());
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
        logDebug("pitch %d\n", pitchSemitone);
        for (uint8_t v = 0; v < MAX_GRAIN_VOICES; v++) {
            for (uint8_t g = 0; g < MAX_GRAINS_PER_VOICE; g++) {
                voices[v].grains[g].sampleStep = getSampleStep(voices[v].note + pitchSemitone);
            }
        }
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
        logDebug("density %d\n", densityUint8);
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
        logDebug("attack %ld samples %f step\n", attackSamples, attackStep);
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
        logDebug("release %ld samples %f step\n", releaseSamples, releaseStep);
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

    void noteOn(uint8_t note, float velocity, void* userdata = NULL) override
    {
        // TODO use velocity
        if (velocity == 0) {
            return noteOff(note, velocity);
        }

        Voice& voice = getNextVoice(note);
        voice.position = voicePosition++;
        voice.note = note;
        voice.envelop.attack();
        float sampleStep = getSampleStep(note + pitchSemitone);
        for (uint8_t g = 0; g < densityUint8; g++) {
            initGrain(voice.grains[g], sampleStep);
        }
        logDebug("noteOn: %d %d %f\n", note, velocity, sampleStep);
    }

    void noteOff(uint8_t note, float velocity, void* userdata = NULL) override
    {
        if (!repeat.get()) { // Play the whole sample if repeat is off
            return;
        }
        for (uint8_t v = 0; v < MAX_GRAIN_VOICES; v++) {
            Voice& voice = voices[v];
            if (voice.note == note) {
                voice.envelop.release();
                logTrace("noteOff set on to false: %d %d\n", note, velocity);
                return;
            }
        }

        logDebug("noteOff: note not found %d %d\n", note, velocity);
    }

protected:
    struct GrainState {
        float position;
        int index;
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
                        grainStates.push_back({ (grain.start + grain.pos) / (float)bufferSampleCount,
                            v * g + g,
                            voice.envelop.isRelease() ? voice.envelop.get() : 1.0f });
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
