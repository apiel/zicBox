#ifndef _AUDIO_PLUGIN_HANDLER_H_
#define _AUDIO_PLUGIN_HANDLER_H_

using namespace std;

#include <functional>
#include <stdexcept>
#include <string>
#include <thread>

#include "Track.h"
#include "def.h"
#include "helpers/getFullpath.h"
#include "helpers/trim.h"
#include "log.h"
#include "midiMapping.h"
#include "plugins/audio/audioPlugin.h"
#include "plugins/audio/lookupTable.h"
#include "helpers/range.h"

/*#md
## Global and generic config

> [!NOTE]
> Generic config are configs that can be used by any plugin. This must be passed after a plugin has been defined.

*/

void midiControllerCb(double deltatime, std::vector<unsigned char>* message, void* userData);

class AudioPluginHandler : public AudioPluginHandlerInterface {
protected:
    LookupTable lookupTable;

    AudioPlugin::Props pluginProps = { SAMPLE_RATE, APP_CHANNELS, this, MAX_TRACKS, &lookupTable };

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
    AudioPluginHandler() { }

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
            for (AudioPlugin* plugin : plugins) {
                logInfo("- Found plugin %s on track %d", plugin->name.c_str(), plugin->track);
            }
            throw std::runtime_error("Could not find plugin " + std::string(name) + " on track " + std::to_string(track));
        }
        return *plugin;
    }

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
        std::vector<Track*> tracks = sortTracksByDependencies(createTracks(buffer, masterCv));
        // std::vector<Track*> tracks = createTracks(buffer, masterCv);

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
#ifdef IS_RPI
            path = "./plugins/audio/build/arm/libzic_" + path + ".so";
#else
            path = "./plugins/audio/build/x86/libzic_" + path + ".so";
#endif
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
        logInfo("- audio plugin loaded: %s", instance->name.c_str());
        plugins.push_back(instance);
    }

    void config(nlohmann::json& config)
    {
        if (config.contains("midiInput")) {
            loadMidiInput(config["midiInput"].get<std::string>());
        }
        if (config.contains("midiOutput")) {
            loadMidiOutput(config["midiOutput"].get<std::string>());
        }
        if (config.contains("tracks") && config["tracks"].is_array()) {
            for (nlohmann::json& track : config["tracks"]) {
                uint8_t trackId = range(track["id"].get<uint8_t>(), 0, MAX_TRACKS - 1);
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
        }
    }

    void noteOn(uint8_t note, float velocity, NoteTarget target) override
    {
        if (target.plugin) {
            target.plugin->noteOn(note, velocity);
        } else {
            for (AudioPlugin* plugin : plugins) {
                if (target.track == -1 || target.track == plugin->track) {
                    plugin->noteOn(note, velocity);
                }
            }
        }
    }

    void noteOff(uint8_t note, float velocity, NoteTarget target) override
    {
        if (target.plugin) {
            target.plugin->noteOff(note, velocity);
        } else {
            for (AudioPlugin* plugin : plugins) {
                if (target.track == -1 || target.track == plugin->track) {
                    plugin->noteOff(note, velocity);
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
            logError("Invalid MIDI mapping "  + cmd);
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

    RtMidiIn midiController;
    RtMidiOut midiOut;

    void midiControllerCallback(double deltatime, std::vector<unsigned char>* message, void* userData = NULL)
    {
        if (message->at(0) == 0xf8) {
            // FIXME
            // clockTick();
            printf("midi clock tick to be implemented\n");
        } else if (message->at(0) == AudioEventType::START) {
            sendEvent(AudioEventType::START); // Should we instead use midi number.. ?
        } else if (message->at(0) == AudioEventType::PAUSE) {
            sendEvent(AudioEventType::PAUSE);
        } else if (message->at(0) == AudioEventType::STOP) {
            sendEvent(AudioEventType::STOP);
        } else if (message->at(0) == 0xfe) {
            // ignore active sensing
        } else if (message->at(0) >= 0x90 && message->at(0) < 0xa0) {
            uint8_t channel = message->at(0) - 0x90;
            midiNoteOn(channel, message->at(1), message->at(2) / 127.0);
        } else if (message->at(0) >= 0x80 && message->at(0) < 0x90) {
            uint8_t channel = message->at(0) - 0x80;
            midiNoteOff(channel, message->at(1), message->at(2) / 127.0);
        } else {
            if (midi(message)) {
                return;
            }
            logDebug("Midi controller message: ");
            unsigned int nBytes = message->size();
            for (unsigned int i = 0; i < nBytes; i++) {
                logDebug("%02x ", (int)message->at(i));
            }
            logDebug("\n");
        }
    }

    int getMidiDevice(RtMidi& midi, std::string portName)
    {
        unsigned int portCount = midi.getPortCount();
        for (unsigned int i = 0; i < portCount; i++) {
            if (midi.getPortName(i).find(portName) != std::string::npos) {
                return i;
            }
        }
        return -1;
    }

    bool loadMidiInput(std::string portName)
    {
        int port = getMidiDevice(midiController, portName);
        if (port == -1) {
            logInfo("Midi input " + portName + " not found");
            return false;
        }

        midiController.openPort(port);
        midiController.setCallback(midiControllerCb, NULL);
        midiController.ignoreTypes(false, false, false);
        logInfo("Midi input loaded: " + midiController.getPortName(port));
        return true;
    }

    bool loadMidiOutput(std::string portName)
    {
        int port = getMidiDevice(midiOut, portName);
        if (port == -1) {
            logInfo("Midi output " + portName + " not found");
            return false;
        }

        midiOut.openPort(port);
        logInfo("Midi output loaded: " + midiOut.getPortName(port));
        return true;
    }
};

AudioPluginHandler* AudioPluginHandler::instance = NULL;

void midiControllerCb(double deltatime, std::vector<unsigned char>* message, void* userData = NULL)
{
    AudioPluginHandler::get().midiControllerCallback(deltatime, message, userData);
}

#endif