#ifndef _SYNTH_SAMPLE_H_
#define _SYNTH_SAMPLE_H_

#include <math.h>
#include <sndfile.h>
#include <time.h>

#include "audioPlugin.h"
#include "fileBrowser.h"
#include "mapping.h"

#include "../../helpers/random.h"
#include "utils/ValSerializeSndFile.h"

// Should all sample be converted to the same format when zicBox starts?
// Sequencer can play different wave files on each step
//          There would be 4 instance SynthSample for the sequencer
//          + 1 instance SynthSample for patch editing
// There is 2 modulations...
// Could add density step by step on every loop

#ifndef MAX_SAMPLE_VOICES
#define MAX_SAMPLE_VOICES 4
#endif

#ifndef MAX_SAMPLE_DENSITY
#define MAX_SAMPLE_DENSITY 12
#endif

class SynthSample : public Mapping {
protected:
    // Hardcoded to 48000, no matter the sample rate
    static const uint64_t bufferSize = 48000 * 30; // 30sec at 48000Hz, 32sec at 44100Hz...
    float sampleData[bufferSize];
    struct SampleBuffer {
        uint64_t count = 0;
        float* data;
    } sampleBuffer;

    FileBrowser fileBrowser = FileBrowser("./samples");

    // Use to restore sustain in case it was move by another parameter
    float sustainPositionOrigin = 0.0f;
    float sustainLengthOrigin = -1.0f;
    bool skipOrigin = false; // Used as point to skip set origin

    int densityDelaySampleCount = 48000 * 0.1; // 100ms at 48000Hz

    Random random;

    struct Voice {
        int8_t note = -1;
        uint64_t index = 0;
        bool release = false;
        float velocity = 1.0f;
        int sustainReleaseLoopCount = 0;
        float step = 0.0f;
        bool finished = true;
        struct SubVoice {
            float position = 0.0f;
            int delay = 0;
        } sub[MAX_SAMPLE_DENSITY];
    } voices[MAX_SAMPLE_VOICES];
    uint64_t voiceIndexCounter = 0;
    bool voiceAllowSameNote = true;

    void voiceStart(Voice& voice)
    {
        voice.release = false;
        for (int d = 0; d < MAX_SAMPLE_DENSITY; d++) {
            int randValue = random.get();
            int randDirection = randValue % 2 ? 1 : -1;
            int randMod = getRandomDensity(randValue) * randDirection;
            voice.sub[d].position = sampleProps.start - (d * densityDelaySampleCount) + randMod;
            voice.sub[d].delay = 0;
        }
    }

    float getRandomDensity(int randValue)
    {
        return random.toPct(randValue) * densityRandomize.pct() * densityDelaySampleCount;
    }

    float stepMultiplier = 1.0f;

    int sustainReleaseLoopCount = 0;
    void setSustainReleaseLoopCount()
    {
        // uint64_t releaseSamples = sustainRelease.pct() * SAMPLE_RATE * 0.001f * 5000;
        // can be simplified to:
        uint64_t releaseSamples = sustainRelease.pct() * props.sampleRate * 5;
        if (sampleProps.sustainSampleCount) {
            sustainReleaseLoopCount = releaseSamples / (float)(sampleProps.sustainSampleCount);
        } else {
            sustainReleaseLoopCount = 0;
        }
    }

    struct SampleProps {
        float start = 0.0f;
        float end = 0.0f;
        float sampleCount = 0.0f;
        bool sustainActive = false;
        float sustainPos = 0.0f;
        float sustainEndPos = 0.0f;
        int64_t sustainSampleCount = 0;
    } sampleProps;

    void setSampleProps()
    {
        sampleProps.start = start.pct() * sampleBuffer.count;
        sampleProps.end = end.pct() * sampleBuffer.count;
        sampleProps.sampleCount = sampleProps.end - sampleProps.start;
        sampleProps.sustainActive = sustainLength.get() > 0.0f;
        sampleProps.sustainPos = sustainPosition.pct() * sampleBuffer.count;
        sampleProps.sustainSampleCount = sustainLength.pct() * sampleBuffer.count;
        sampleProps.sustainEndPos = sampleProps.sustainPos + sampleProps.sustainSampleCount;
    }

    float sample(Voice& voice, Voice::SubVoice& sub)
    {
        float out = 0.0f;
        if (sub.delay > 0) {
            sub.delay--;
            voice.finished = false;
            return out;
        }

        if (
            sampleProps.sustainActive
            && (voice.release == false || voice.sustainReleaseLoopCount < sustainReleaseLoopCount)
            && sub.position >= sampleProps.sustainEndPos) {
            sub.position = sampleProps.sustainPos;
            sub.delay = getRandomDensity(random.get());
            if (voice.release) {
                voice.sustainReleaseLoopCount++;
            }
        }

        if (sub.position < sampleProps.end) {
            if (sub.position >= sampleProps.start) {
                out = sampleBuffer.data[(uint64_t)sub.position] * voice.velocity;
            }
            voice.finished = false;
            sub.position += voice.step;
        }
        return out;
    }

    float sample(Voice& voice)
    {
        float out = 0.0f;
        uint8_t densityUint8 = density.get();
        voice.finished = true;
        for (uint8_t d = 0; d < densityUint8; d++) {
            out += sample(voice, voice.sub[d]);
        }
        if (voice.finished) {
            voice.note = -1;
        }

        return out;
    }

    Voice& getNextVoice(uint8_t note)
    {
        if (voiceAllowSameNote) {
            // First, we should look if the voice is not already running, due to the envelopRelease
            for (uint8_t v = 0; v < MAX_SAMPLE_VOICES; v++) {
                Voice& voice = voices[v];
                if (voice.note == note) {
                    debug("getNextVoice: voice already running %d\n", note);
                    return voice;
                }
            }
        }

        // Else, we should look for a free voice
        uint8_t voiceToSteal = 0;
        for (uint8_t v = 0; v < MAX_SAMPLE_VOICES; v++) {
            Voice& voice = voices[v];
            if (voice.note == -1) {
                return voice;
            }
            if (voice.index < voices[voiceToSteal].index) {
                voiceToSteal = v;
            }
        }

        debug("getNextVoice: no voice available. Steal voice %d.\n", voiceToSteal);
        return voices[voiceToSteal];
    }

    uint8_t baseNote = 60;
    float getSampleStep(uint8_t note)
    {
        // https://gist.github.com/YuxiUx/ef84328d95b10d0fcbf537de77b936cd
        // pow(2, ((0) / 12.0)) = 1 for 0 semitone
        // pow(2, ((1) / 12.0)) = 1.059463094 for 1 semitone
        // pow(2, ((2) / 12.0)) = 1.122462048 for 2 semitone
        // ...

        // printf("getSampleStep: %d >> %d = %f\n", note, note - baseNote, pow(2, (note - baseNote) / 12.0));
        return pow(2, ((note - baseNote) / 12.0)) * stepMultiplier;
    }

public:
    Val& start = val(0.0f, "START", { "Start", .unit = "%" }, [&](auto p) { setStart(p.value); });
    Val& end = val(100.0f, "END", { "End", .unit = "%" }, [&](auto p) { setEnd(p.value); });
    Val& sustainPosition = val(0.0f, "SUSTAIN_POSITION", { "Sustain position", .unit = "%" }, [&](auto p) { setSustainPosition(p.value, (bool*)p.data); });
    // Where -1 is no sustain
    Val& sustainLength = val(0.0f, "SUSTAIN_LENGTH", { "Sustain length", .unit = "%" }, [&](auto p) { setSustainLength(p.value, (bool*)p.data); });
    // Sustain release set a delay before the sustain ends when note off is triggered
    Val& sustainRelease = val(0.0f, "SUSTAIN_RELEASE", { "Sustain Release", .min = 0.0, .max = 5000.0, .step = 50.0, .unit = "ms" }, [&](auto p) { setSustainRelease(p.value); });
    // Density would is adding more voice (sub voice) with a little delay on each added sub voice
    Val& density = val(1.0f, "DENSITY", { "Density", .min = 1.0, .max = 12 });
    // Density delay is the time between each sub voice
    Val& densityDelay = val(0.0f, "DENSITY_DELAY", { "Density Delay", .min = 1.0, .max = 1000.0, .step = 1.1, .unit = "ms", .incrementationType = VALUE_INCREMENTATION_EXP }, [&](auto p) { setDensityDelay(p.value); });
    // If randomize is set, the density starting delay is random and while change on each sustain loop
    Val& densityRandomize = val(0.0f, "DENSITY_RANDOMIZE", { "Density Randomize", .unit = "%" });
    // Spray allows density in the sustain loop to get out of the boundary windows
    Val& densitySpray = val(0.0f, "SUSTAIN_SPRAY", { "Sustain Spray", .unit = "%" });

    Val& browser = val(0.0f, "BROWSER", { "Browser", VALUE_STRING, .max = (float)fileBrowser.count }, [&](auto p) { open(p.value); });

    SynthSample(AudioPlugin::Props& props, char* _name)
        : Mapping(props, _name)
    {
        open(browser.get(), true);
        initValues();
    }

    bool config(char* key, char* value) override
    {
        if (strcmp(key, "SAMPLES_FOLDER") == 0) {
            fileBrowser.openFolder(value);
            browser.props().max = fileBrowser.count;
            open(0.0, true);

            return true;
        }

        if (strcmp(key, "VOICE_ALLOW_SAME_NOTE") == 0) {
            voiceAllowSameNote = strcmp(value, "true") == 0;
            return true;
        }

        if (strcmp(key, "BASE_NOTE") == 0) {
            baseNote = atoi(value);
            baseNote = range(baseNote, 0, 127);
            return true;
        }

        return AudioPlugin::config(key, value);
    }

    void sample(float* buf)
    {
        float out = 0.0f;
        for (uint8_t v = 0; v < MAX_SAMPLE_VOICES; v++) {
            Voice& voice = voices[v];
            if (voice.note != -1) {
                out += sample(voice);
            }
        }
        buf[track] = out;
    }

    void noteOn(uint8_t note, uint8_t velocity) override
    {
        if (velocity == 0) {
            return noteOff(note, velocity);
        }

        Voice& voice = getNextVoice(note);
        voice.index = voiceIndexCounter++;
        voice.note = note;
        voice.step = getSampleStep(note);
        voice.velocity = velocity / 127.0f;
        voiceStart(voice);
        // TODO attack softly if start after beginning of file
        debug("noteOn: %d %d\n", note, velocity);
    }

    void noteOff(uint8_t note, uint8_t velocity) override
    {
        for (uint8_t v = 0; v < MAX_SAMPLE_VOICES; v++) {
            Voice& voice = voices[v];
            if (voice.note == note) {
                voice.release = true;
                voice.sustainReleaseLoopCount = 0;
                // TODO release softly if release before end of file
                debug("noteOff set on to false: %d %d\n", note, velocity);
                return;
            }
        }
        debug("noteOff: note not found %d %d\n", note, velocity);
    }

    void setDensityDelay(float value)
    {
        densityDelay.setFloat(value);
        densityDelaySampleCount = props.sampleRate * densityDelay.get() * 0.001f;
    }

    void setSustainRelease(float value)
    {
        sustainRelease.setFloat(value);
        setSustainReleaseLoopCount();
    }

    void open(const char* filename)
    {
        SF_INFO sfinfo;
        SNDFILE* file = sf_open(filename, SFM_READ, &sfinfo);
        if (!file) {
            debug("Error: could not open file %s [%s]\n", filename, sf_strerror(file));
            return;
        }
        // debug("Audio file %s sampleCount %ld sampleRate %d\n", filename, (long)sfinfo.frames, sfinfo.samplerate);
        // printf(".................Audio file chan %d vs prop chan %d\n", sfinfo.channels, props.channels);

        sampleBuffer.count = sf_read_float(file, sampleData, bufferSize);
        sampleBuffer.data = sampleData;

        sf_close(file);

        if (sfinfo.channels < props.channels) {
            stepMultiplier = 0.5f;
        } else if (sfinfo.channels > props.channels) {
            stepMultiplier = 2.0f;
        } else {
            stepMultiplier = 1.0f;
        }

        // FIXME
        // ValSerializeSndFile serialize(mapping);
        // serialize.loadSetting(filename);

        setSampleProps();
    }

    void open(float value, bool force = false)
    {
        browser.setFloat(value);
        int position = browser.get();
        if (force || position != fileBrowser.position) {
            char* filepath = fileBrowser.getFilePath(position);
            browser.setString(fileBrowser.getFile(position));
            // debug("SAMPLE_SELECTOR: %f %s\n", value, filepath);
            open(filepath);
        }
    }

    void setStart(float value)
    {
        if (value >= end.get()) {
            return;
        }
        start.setFloat(value);
        setValueBoundaries();
        setSampleProps();
    }

    void setEnd(float value)
    {
        if (value <= start.get()) {
            return;
        }
        end.setFloat(value);
        setValueBoundaries();
        setSampleProps();
    }

    void setSustainPosition(float value, bool* setOrigin)
    {
        if (value < start.get() || value + sustainLength.get() > end.get()) {
            return;
        }
        sustainPosition.setFloat(value);
        if (setOrigin == NULL || *setOrigin) {
            sustainPositionOrigin = sustainPosition.get();
            setValueBoundaries();
        }
        setSampleProps();
    }

    void setSustainLength(float value, bool* setOrigin)
    {
        if (value + sustainPosition.get() > end.get()) {
            return;
        }
        sustainLength.setFloat(value);
        if (setOrigin == NULL || *setOrigin) {
            sustainLengthOrigin = sustainLength.get();
        }
        setSampleProps();
        setSustainReleaseLoopCount();
    }

    void setValueBoundaries()
    {
        float sustain = sustainPosition.get();
        if (start.get() > sustain) {
            sustainLength.set(sustainLength.get() - (start.get() - sustain), &skipOrigin);
            sustainPosition.set(start.get(), &skipOrigin);
        }

        float sustainLen = sustainLength.get();
        if (sustain + sustainLen > end.get()) {
            sustainLength.set(end.get() - sustain, &skipOrigin);
        }

        restoreSustainPosition();
        restoreSustainlength();
    }

    void restoreSustainlength()
    {
        if (sustainLengthOrigin != sustainLength.get()) {
            int sustainLen = end.get() - sustainPosition.get();
            if (sustainLen >= sustainLengthOrigin) {
                sustainLength.set(sustainLengthOrigin, &skipOrigin);
            } else {
                sustainLength.set(sustainLen, &skipOrigin);
            }
        }
    }

    void restoreSustainPosition()
    {
        if (sustainPositionOrigin != sustainPosition.get()) {
            int sustainPos = start.get();
            if (sustainPos <= sustainPositionOrigin) {
                sustainPosition.set(sustainPositionOrigin, &skipOrigin);
            } else {
                sustainPosition.set(sustainPos, &skipOrigin);
            }
        }
    }

protected:
    struct SampleState {
        float position;
        int index = 0;
        float release = 1.0f;
    };
    std::vector<SampleState> sampleStates;

public:
    void* data(int id, void* userdata = NULL)
    {
        switch (id) {
        case 0:
            return &sampleBuffer;
        case 1: {
            // This could be cache in envelopRelease
            for (Voice& voice : voices) {
                if (voice.note != -1) {
                    return &voice;
                }
            }
            return NULL;
        }
        case 2: {
            sampleStates.clear();
            for (uint8_t v = 0; v < MAX_SAMPLE_VOICES; v++) {
                Voice& voice = voices[v];
                if (voice.note != -1) {
                    uint8_t densityUint8 = density.get();
                    for (uint8_t d = 0; d < densityUint8; d++) {
                        SampleState sampleState;
                        sampleState.index = v * densityUint8 + d;
                        sampleState.position = range(voice.sub[d].position / sampleProps.end, 0.0f, 1.0f);
                        sampleState.release = voice.release ? 1 - voice.sub[d].position / sampleProps.end : 1.0f;
                        sampleStates.push_back(sampleState);
                    }
                }
            }
            return &sampleStates;
        }
        }
        return NULL;
    }
};

#endif