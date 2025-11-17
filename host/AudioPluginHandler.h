#pragma once

using namespace std;

#include <fstream>
#include <functional>
#include <stdexcept>
#include <string>
#include <thread>

#include <alsa/asoundlib.h>

#include "Track.h"
#include "def.h"
#include "helpers/clamp.h"
#include "helpers/getExecutableDirectory.h"
#include "helpers/trim.h"
#include "log.h"
#include "midiMapping.h"
#include "plugins/audio/audioPlugin.h"
#include "audio/lookupTable.h"

/*#md
## Global and generic config

> [!NOTE]
> Generic config are configs that can be used by any plugin. This must be passed after a plugin has been defined.

*/

class AudioPluginHandler : public AudioPluginHandlerInterface {
protected:
    LookupTable lookupTable;

    AudioPlugin::Props pluginProps = { SAMPLE_RATE, AUDIO_CHANNELS, this, MAX_TRACKS, &lookupTable };

    std::vector<MidiMapping> midiMapping;

    // When reached the maximum, there might be a tempo issue
    // However, to reach it, it is almost impossible...
    uint64_t clockCounter = 0;
    bool playing = false;

    struct MidiNoteEvent {
        uint8_t channel;
        NoteTarget target;
    };
    std::vector<MidiNoteEvent> midiNoteEvents;

    std::thread autoSaveThread;

    static AudioPluginHandler* instance;
    AudioPluginHandler()
    {
        listMidiDevices();
    }

    std::vector<uint8_t> getTrackIds()
    {
        std::set<uint8_t> trackIdsSet;
        std::vector<uint8_t> trackIds; // for whatever reason unsorted_set is not working as expected
        for (AudioPlugin* plugin : plugins) {
            if (plugin->track < MAX_TRACKS && trackIdsSet.find(plugin->track) == trackIdsSet.end()) {
                trackIdsSet.insert(plugin->track);
                trackIds.push_back(plugin->track);
            }
        }
        return trackIds;
    }

    Track* createTrack(uint8_t id, float* buffer, std::condition_variable& masterCv)
    {
        Track* track = new Track(id, buffer, masterCv, MAX_TRACKS);
        for (AudioPlugin* plugin : plugins) {
            if (plugin->track == id && plugin->getType() != AudioPlugin::Type::TEMPO) {
                track->plugins.push_back(plugin);
            }
        }
        return track;
    }

    AudioPlugin* getTempoPlugin()
    {
        for (AudioPlugin* plugin : plugins) {
            if (plugin->getType() == AudioPlugin::Type::TEMPO) {
                return plugin;
            }
        }
        return NULL;
    }

    std::vector<Track*> createTracks(float* buffer, std::condition_variable& masterCv)
    {
        std::vector<Track*> tracks;
        std::vector<uint8_t> trackIds = getTrackIds();
        for (auto trackId : trackIds) {
            // printf("----------------- Track %d\n", trackId);
            Track* track = createTrack(trackId, buffer, masterCv);
            tracks.push_back(track);
            if (track->plugins.size() == 0) {
                tracks.pop_back();
            }
        }
        return tracks;
    }

    // Check wether all dependencies are are available in the sortedTracks
    // However, if track dependencies are missing in the origin tracks, we don't count them
    // example: mixer expecting to get 4 tracks, but only 2 of them are used...
    uint8_t countMissingTracks(std::set<uint8_t> dependencies, std::set<uint8_t> target, std::set<uint8_t> origin)
    {
        uint8_t missing = 0;
        for (auto s : dependencies) {
            if (target.find(s) == target.end() && origin.find(s) != origin.end()) {
                missing++;
            }
        }
        return missing;
    }

    std::vector<Track*> sortTracksByDependencies(std::vector<Track*> tracks)
    {
        std::set<uint8_t> originTrackIds;
        for (Track* track : tracks) {
            originTrackIds.insert(track->id);
        }

        std::set<uint8_t> trackIds;
        std::vector<Track*> sortedTracks;
        // First push tracks without dependencies
        for (Track* track : tracks) {
            if (!track->hasDependencies()) {
                sortedTracks.push_back(track);
                trackIds.insert(track->id);
            }
        }
        // Let's limit the number of loop to be sure we do not get stuck in an infinite loop
        for (uint8_t i = 0; i < 250 && sortedTracks.size() < tracks.size(); i++) {
            for (Track* track : tracks) {
                // Check that track is not already in the sortedTracks
                if (trackIds.find(track->id) == trackIds.end()) {
                    std::set<uint8_t> dependencies = track->getDependencies();
                    uint8_t missing = countMissingTracks(dependencies, trackIds, originTrackIds);
                    // printf("----> Track %d dependencies %ld missing %d\n", track->id, dependencies.size(), missing);
                    // now check that all dependencies are in the sortedTracks
                    if (missing == 0) {
                        sortedTracks.push_back(track);
                        trackIds.insert(track->id);
                    }
                }
            }
        }
        return sortedTracks;
    }

public:
    static AudioPluginHandler& get()
    {
        if (!instance) {
            instance = new AudioPluginHandler();
        }
        return *instance;
    }

    AudioPlugin* getPluginPtr(std::string name, int16_t track = -1) override
    {
        for (AudioPlugin* plugin : plugins) {
            if (plugin->name == name && (track == -1 || plugin->track == track)) {
                return plugin;
            }
        }
        return NULL;
    }

    AudioPlugin& getPlugin(std::string name, int16_t track = -1) override
    {
        AudioPlugin* plugin = getPluginPtr(name, track);
        if (!plugin) {
            logInfo("Could not find plugin %s on track %d. List of available plugins:", name.c_str(), track);
            // for (AudioPlugin* plugin : plugins) {
            //     logInfo("- Found plugin %s on track %d", plugin->name.c_str(), plugin->track);
            // }
            throw std::runtime_error("Could not find plugin " + std::string(name) + " on track " + std::to_string(track));
        }
        return *plugin;
    }

    std::vector<Track*> tracks;
    void loop()
    {
        int bufferSize = 128 * TOTAL_TRACKS;
        float buffer[128 * TOTAL_TRACKS] = { 0.0f };

        std::mutex masterMtx;
        std::condition_variable masterCv;
        std::unique_lock<std::mutex> lock = std::unique_lock(masterMtx);

        // Create tracks
        // Sorting the tracks would not be mandatory if plugins are instanciated in the right order in the config file
        // However, it is very easy to not think abut it, so introducing this sorting system can save from trouble (even if it introduce complexity)
        // For example, initializing the tempo plugin at the begining of the file, could just lead to not having sound
        // at all because track 0 would design at first position but also be used for audio output, meaning the that the
        // buffer would be cleaned up even before the audio output being consumed...
        tracks = sortTracksByDependencies(createTracks(buffer, masterCv));
        // std::vector<Track*> tracks = createTracks(buffer, masterCv);

        if (initActiveMidiTrack > -1) {
            logDebug("Set active track to %d for midi input.", initActiveMidiTrack);
            setActiveMidiTrack(initActiveMidiTrack, true);
        }

        Track* threadTracks[TOTAL_TRACKS];
        Track* hostTracks[TOTAL_TRACKS];
        int threadCount = 0;
        int hostCount = 0;

        AudioPlugin* tempoPlugin = getTempoPlugin();

        // Init tracks
        for (Track* track : tracks) {
            // For the moment, let's assume that last track is always master track
            bool isMaster = track->id == tracks.back()->id;
            track->init(tracks, isMaster);
            if (track->thread.joinable()) {
                threadTracks[threadCount++] = track;
            } else {
                hostTracks[hostCount++] = track;
            }
        }

        if (!tempoPlugin) {
            // Should we allow to start without tempo?? then would need if statement around the loops..
            logError("No tempo plugin loaded. There should be at one to start audio loop.");
            return;
        }

        if (threadCount == 0) {
            while (isRunning) {
                float buffer[TOTAL_TRACKS] = { 0.0f };
                tempoPlugin->sample(buffer);
                for (int i = 0; i < hostCount; i++) {
                    hostTracks[i]->process(buffer);
                }
            }
        } else {
            auto ms = std::chrono::milliseconds(10);

            while (isRunning) {
                for (uint8_t i = 0; i < 128; i++) {
                    tempoPlugin->sample(buffer + i * CLOCK_TRACK);
                }

                for (int t = 0; t < threadCount; t++) {
                    Track* track = threadTracks[t];
                    track->processing = true;
                    track->cv.notify_one();
                }

                masterCv.wait_for(lock, ms, [&] {
                    for (int t = 0; t < threadCount; t++) {
                        Track* track = threadTracks[t];
                        if (track->processing) {
                            return false;
                        }
                    }
                    return true;
                });

                // NOTE: why multiple tracks here? one should be enough...
                for (int t = 0; t < hostCount; t++) {
                    for (uint8_t i = 0; i < 128; i++) {
                        hostTracks[t]->process(i);
                    }
                }

                // cleanup buffer
                for (int i = 0; i < bufferSize; i++) {
                    buffer[i] = 0.0f;
                }
            }
            // Wait for to finish
            for (Track* track : tracks) {
                if (track->thread.joinable()) {
                    track->thread.join();
                }
            }
        }

        // Delete tracks
        for (Track* track : tracks) {
            delete track;
        }
    }

    void loadPlugin(nlohmann::json& config, uint8_t trackId)
    {
        std::string path = config["plugin"]; // plugin name or path
        if (path.substr(path.length() - 3) != ".so") {
            path = getExecutableDirectory() + "/libs/audio/libzic_" + path + ".so";
        }

        void* handle = dlopen(path.c_str(), RTLD_LAZY);

        if (!handle) {
            logWarn("Cannot open audio library %s: %s", path.c_str(), dlerror());
            return;
        }
        dlerror();
        void* allocator = (AudioPlugin*)dlsym(handle, "allocator");
        const char* dlsym_error = dlerror();
        if (dlsym_error) {
            logWarn("Cannot load symbol: %s", dlsym_error);
            dlclose(handle);
            return;
        }

        std::string name = config["alias"];
        AudioPlugin::Config pluginConfig = { name, config, trackId };
        AudioPlugin* instance = ((AudioPlugin * (*)(AudioPlugin::Props & props, AudioPlugin::Config & config)) allocator)(pluginProps, pluginConfig);
        logTrace("- audio plugin loaded: %s", instance->name.c_str());
        plugins.push_back(instance);
    }

    int8_t initActiveMidiTrack = -1;

    AudioPluginHandler& config(nlohmann::json& config) override
    {
        if (config.contains("midiInput")) {
            loadMidiInput(config["midiInput"].get<std::string>());
        }
        if (config.contains("midiOutput")) {
            loadMidiOutput(config["midiOutput"].get<std::string>());
        }
        debugMidi = config.value("debugMidi", debugMidi);
        if (config.contains("tracks") && config["tracks"].is_array()) {
            for (nlohmann::json& track : config["tracks"]) {
                uint8_t trackId = CLAMP(track["id"].get<uint8_t>(), 0, MAX_TRACKS - 1);
                logInfo("*** Init plugins for track %d", trackId);
                if (track.contains("plugins") && track["plugins"].is_array()) {
                    for (nlohmann::json& plugin : track["plugins"]) {
                        loadPlugin(plugin, trackId);
                    }
                }
            }
        }
        if (config.contains("autoSave")) {
            uint32_t msInterval = config["autoSave"].get<uint32_t>();
            if (msInterval > 0) {
                startAutoSave(msInterval);
            }
        } else {
            // We call AUTOSAVE only once but don't start a thread to regular interval.
            sendEvent(AudioEventType::AUTOSAVE);
        }

        // Instead to use midi channel, use active track to send midi notes
        // The value passed to `initActiveMidiTrack`, should be the active track at initialization
        initActiveMidiTrack = config.value("initActiveMidiTrack", initActiveMidiTrack);

        if (config.contains("autoLoadFirstMidiDevice")) {
            if (config["autoLoadFirstMidiDevice"].get<bool>()) {
                logInfo("Auto load first midi device");
                loadMidiDevices();
                if (midiDevices.size() > 0) {
                    logDebug("Load first midi device: %s", midiDevices[0].name.c_str());
                    loadMidiInput(midiDevices[0].name);
                    loadMidiOutput(midiDevices[0].name);
                }
            }
        }

        return *this;
    }

    void noteOn(uint8_t note, float velocity, NoteTarget target) override
    {
        if (target.plugin) {
            target.plugin->noteOn(note, velocity, NULL);
        } else {
            for (AudioPlugin* plugin : plugins) {
                if (target.track == -1 || target.track == plugin->track) {
                    // printf("pluginHandler noteOn %d %f %d\n", note, velocity, plugin->track);
                    plugin->noteOn(note, velocity, NULL);
                }
            }
        }
    }

    void noteOff(uint8_t note, float velocity, NoteTarget target) override
    {
        if (target.plugin) {
            target.plugin->noteOff(note, velocity, NULL);
        } else {
            for (AudioPlugin* plugin : plugins) {
                if (target.track == -1 || target.track == plugin->track) {
                    plugin->noteOff(note, velocity, NULL);
                }
            }
        }
    }

    bool midi(std::vector<unsigned char>* message)
    {
        for (MidiMapping& mapping : midiMapping) {
            if (mapping.handle(message)) {
                return true;
            }
        }
        return false;
    }

    // `b0 4c xx` assign a midi command to a given plugin value.
    void mapMidiCmd(AudioPlugin* plugin, int valueIndex, const std::string& cmd) override
    {
        std::istringstream stream(cmd);
        std::string msg0, msg1, msg2;
        stream >> msg0 >> msg1 >> msg2;

        try {
            auto msg0Int = static_cast<uint8_t>(std::stoi(msg0, nullptr, 16));
            auto msg1Int = static_cast<uint8_t>(std::stoi(msg1, nullptr, 16));

            uint8_t size = msg2.empty() ? 2 : 3;
            uint8_t valuePosition = (msg1 == "xx") ? 2 : 3;

            midiMapping.push_back({ plugin, valueIndex, size, valuePosition, msg0Int, msg1Int });
            logInfo("Assign MIDI command %s to plugin %s value %s", cmd.c_str(), plugin->name, plugin->getValue(valueIndex)->key().c_str());
        } catch (...) {
            logError("Invalid MIDI mapping " + cmd);
        }
    }

    void assignPluginToMidiChannel(uint8_t channel, AudioPlugin* plugin) override
    {
        if (channel < 1 || channel > 16) {
            logInfo("Invalid midi note channel, set to 1");
            channel = 1;
        }
        midiNoteEvents.push_back({ (uint8_t)(channel - 1), { .plugin = plugin } });
        logInfo("Assign %s to midi channel %d", plugin->name, channel);
    }

    void midiNoteOn(uint8_t channel, uint8_t note, float velocity)
    {
        if (velocity == 0) {
            midiNoteOff(channel, note, velocity);
        }

        // printf("-------------- noteOn %d %d %f\n", channel, note, velocity);
        for (MidiNoteEvent& target : midiNoteEvents) {
            if (target.channel == channel) {
                noteOn(note, velocity, target.target);
            }
        }
    }

    void midiNoteOff(uint8_t channel, uint8_t note, float velocity)
    {
        // printf("------------- noteOff %d %d %f\n", channel, note, velocity);
        for (MidiNoteEvent& target : midiNoteEvents) {
            if (target.channel == channel) {
                noteOn(note, velocity, target.target);
            }
        }
    }

    bool isPlaying()
    {
        return playing;
    }

    bool isStopped()
    {
        return !playing && clockCounter == 0;
    }

    void sendEvent(AudioEventType event, int16_t track = -1)
    {
        if (event == AudioEventType::SET_ACTIVE_TRACK) {
            setActiveMidiTrack(track);
            return;
        }
        if (track == -1) { // there is no point to check those events if it is a specific track event
            switch (event) {
            case AudioEventType::START:
                playing = true;
                break;

            case AudioEventType::STOP:
                playing = false;
                clockCounter = 0;
                break;

            case AudioEventType::PAUSE:
                playing = false;
                break;

            case AudioEventType::TOGGLE_PLAY_PAUSE:
                playing = !playing;
                // convert event to start or pause to avoid to have to many conditions in the plugins
                event = playing ? AudioEventType::START : AudioEventType::PAUSE;
                break;
            }
        }
        // if (event != AUTOSAVE) printf(">>> AudioPluginHandler::sendEvent %d\n", event);
        for (AudioPlugin* plugin : plugins) {
            if (track == -1 || plugin->track == track) {
                plugin->onEvent(event, playing);
            }
        }
    }

    Track* activeMidiTrack = NULL;
    void setActiveMidiTrack(int16_t trackId, bool force = false)
    {
        // logDebug("setActiveMidiTrack %d", trackId);
        if (trackId != -1 && (activeMidiTrack != NULL || force)) {
            for (Track* track : tracks) {
                if (track->id == trackId) {
                    activeMidiTrack = track;
                    break;
                }
            }
        }
    }

    void startAutoSave(uint32_t msInterval)
    {
        if (msInterval < 50) {
            msInterval = 50;
        }

        autoSaveThread = std::thread([this, msInterval]() {
            logInfo("Starting autosave thread with interval %d ms", msInterval);

            // Wait a bit first, else it crash...
            std::this_thread::sleep_for(std::chrono::milliseconds(100));

            while (isRunning) {
                sendEvent(AudioEventType::AUTOSAVE);
                std::this_thread::sleep_for(std::chrono::milliseconds(msInterval));
            }
        });
        pthread_setname_np(autoSaveThread.native_handle(), "autoSave");

        // Save a last time before to exit
        sendEvent(AudioEventType::AUTOSAVE);
    }

    struct MidiDevice {
        std::string name;
        std::string id;
    };
    std::vector<MidiDevice> midiDevices;
    void loadMidiDevices()
    {
        snd_ctl_t* handle;
        snd_rawmidi_info_t* info;
        int card = -1;

        snd_rawmidi_info_alloca(&info);

        midiDevices.clear();
        while (snd_card_next(&card) >= 0 && card >= 0) {
            char name[32];
            sprintf(name, "hw:%d", card);

            int err;
            if ((err = snd_ctl_open(&handle, name, 0)) < 0) {
                fprintf(stderr, "Control open error: %s\n", snd_strerror(err));
                continue;
            }

            int device = -1;
            while (snd_ctl_rawmidi_next_device(handle, &device) >= 0 && device >= 0) {
                snd_rawmidi_info_set_device(info, device);
                snd_rawmidi_info_set_subdevice(info, 0);
                snd_rawmidi_info_set_stream(info, SND_RAWMIDI_STREAM_INPUT);
                if (snd_ctl_rawmidi_info(handle, info) == 0) {
                    string id = "hw:" + to_string(card) + "," + to_string(device);
                    midiDevices.push_back({ std::string(snd_rawmidi_info_get_name(info)), id });

                } else {
                    snd_rawmidi_info_set_stream(info, SND_RAWMIDI_STREAM_OUTPUT);
                    if (snd_ctl_rawmidi_info(handle, info) == 0) {
                        string id = "hw:" + to_string(card) + "," + to_string(device);
                        midiDevices.push_back({ std::string(snd_rawmidi_info_get_name(info)), id });
                    }
                }
            }

            snd_ctl_close(handle);
        }
    }
    void listMidiDevices()
    {
        loadMidiDevices();
        logInfo("Available MIDI devices:");
        for (MidiDevice& device : midiDevices) {
            logInfo("[%s] %s", device.id.c_str(), device.name.c_str());
        }
    }

    MidiDevice* getMidiDevice(std::string name)
    {
        for (MidiDevice& device : midiDevices) {
            if (name == device.name || name == device.id) {
                return &device;
            }
        }
        return nullptr;
    }

    std::thread midiInputThread;
    bool loadMidiInput(std::string name)
    {
        MidiDevice* device = getMidiDevice(name);
        if (device == nullptr) {
            logWarn("Midi device " + name + " not found.");
            return false;
        }

        snd_rawmidi_t* handle;
        if (snd_rawmidi_open(&handle, NULL, device->id.c_str(), SND_RAWMIDI_NONBLOCK) < 0) {
            logWarn("Error opening MIDI input device %s [%s]", device->name.c_str(), device->id.c_str());
            return false;
        }

        logInfo("MIDI input device %s [%s] opened", device->name.c_str(), device->id.c_str());
        midiInputThread = std::thread([this, handle] { midiInHandler(handle); });
        pthread_setname_np(midiInputThread.native_handle(), ("midi_in " + device->name).c_str());

        return true;
    }

    bool debugMidi = false;
    void midiInHandler(snd_rawmidi_t* handle)
    {
        while (1) {
            unsigned char buffer[3];
            int n = snd_rawmidi_read(handle, buffer, sizeof(buffer));
            if (n > 0) {
                if (buffer[0] == 0xf8) {
                    // FIXME
                    // clockTick();
                    printf("midi clock tick to be implemented\n");
                } else if (buffer[0] == AudioEventType::START) {
                    sendEvent(AudioEventType::START); // Should we instead use midi number.. ?
                } else if (buffer[0] == AudioEventType::PAUSE) {
                    sendEvent(AudioEventType::PAUSE);
                } else if (buffer[0] == AudioEventType::STOP) {
                    sendEvent(AudioEventType::STOP);
                } else if (buffer[0] == 0xfe) {
                    // ignore active sensing
                } else if (buffer[0] >= 0x90 && buffer[0] < 0xa0) {
                    uint8_t channel = buffer[0] - 0x90;
                    if (activeMidiTrack != NULL) {
                        activeMidiTrack->noteOn(buffer[1], buffer[2] / 127.0);
                    } else {
                        midiNoteOn(channel, buffer[1], buffer[2] / 127.0);
                    }
                } else if (buffer[0] >= 0x80 && buffer[0] < 0x90) {
                    uint8_t channel = buffer[0] - 0x80;
                    if (activeMidiTrack != NULL) {
                        activeMidiTrack->noteOff(buffer[1], buffer[2] / 127.0);
                    } else {
                        midiNoteOff(channel, buffer[1], buffer[2] / 127.0);
                    }
                } else {
                    std::vector<unsigned char>* message = new std::vector<unsigned char>(buffer, buffer + n);
                    if (!midi(message) && debugMidi) {
                        logDebug("Midi input message: ");
                        unsigned int nBytes = message->size();
                        for (unsigned int i = 0; i < nBytes; i++) {
                            logDebug("%02x ", (int)buffer[i]);
                        }
                    }
                }
            }
        }
        snd_rawmidi_close(handle);
    }

    snd_rawmidi_t* midiOuthandle = NULL;
    bool loadMidiOutput(std::string name)
    {
        MidiDevice* device = getMidiDevice(name);
        if (device == nullptr) {
            logWarn("Midi device " + name + " not found.");
            return false;
        }

        if (snd_rawmidi_open(NULL, &midiOuthandle, device->id.c_str(), 0) < 0) {
            logWarn("Error opening MIDI output device %s [%s]", device->name.c_str(), device->id.c_str());
            return false;
        }

        logInfo("MIDI output device %s [%s] opened", device->name.c_str(), device->id.c_str());

        return true;
    }

    void sendMidi(std::vector<unsigned char>* message)
    {
        if (midiOuthandle) {
            snd_rawmidi_write(midiOuthandle, message->data(), message->size());
            snd_rawmidi_drain(midiOuthandle);
        }
    }
};

AudioPluginHandler* AudioPluginHandler::instance = NULL;
