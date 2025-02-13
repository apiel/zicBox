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

/*//md
## Global and generic config

> [!NOTE]
> Generic config are configs that can be used by any plugin. This must be passed after a plugin has been defined.

*/

class AudioPluginHandler : public AudioPluginHandlerInterface {
public:
    struct PluginAlias {
        string name;
        std::function<AudioPlugin*(AudioPlugin::Props& props, char* name)> allocator;
    };
    std::vector<PluginAlias> pluginAliases;

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

    bool assignMidiPluginChannel(char* value)
    {
        uint8_t channel = atoi(value);
        if (channel < 1 || channel > 16) {
            logInfo("Invalid midi note channel, set to 1");
            channel = 1;
        }
        midiNoteEvents.push_back({ (uint8_t)(channel - 1), { .plugin = plugins.back() } });
        logInfo("[%s] Midi plugin channel set to %d", plugins.back()->name, channel);
        return true;
    }

    bool assignMidiTrackChannel(char* value)
    {
        int16_t track = atoi(strtok(value, " "));
        uint8_t channel = atoi(strtok(NULL, " "));
        if (channel < 1 || channel > 16) {
            logInfo("Invalid midi note channel, set to 1");
            channel = 1;
        }
        midiNoteEvents.push_back({ (uint8_t)(channel - 1), { track } });
        logInfo("[%s] Midi track channel set to %d", plugins.back()->name, channel);
        return true;
    }

    bool assignMidiMapping(char* value)
    {
        // split value by space
        char* pluginKey = strtok(value, " ");
        char* msg0 = strtok(NULL, " ");
        char* msg1 = strtok(NULL, " ");
        char* msg2 = strtok(NULL, " ");

        if (msg0 == NULL || msg1 == NULL) {
            logInfo("Invalid midi mapping");
            return false;
        }

        uint8_t size = msg2 == NULL ? 2 : 3;
        uint8_t valuePosition = msg1[0] == 'x' && msg1[1] == 'x' ? 2 : 3;
        uint8_t msg0Int = strtol(msg0, NULL, 16);
        uint8_t msg1Int = strtol(msg1, NULL, 16);

        // try to assign value to last plugin
        int valueIndex = plugins.back()->getValueIndex(pluginKey);
        if (valueIndex != -1) {
            midiMapping.push_back({ plugins.back(), valueIndex, size, valuePosition, msg0Int, msg1Int });
            logInfo("[%s] Midi mapping assigned: %s", plugins.back()->name, pluginKey);
            return true;
        }
        return false;
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

    AudioPlugin* getPluginPtr(const char* name, int16_t track = -1)
    {
        for (AudioPlugin* plugin : plugins) {
            if (strcmp(plugin->name, name) == 0 && (track == -1 || plugin->track == track)) {
                return plugin;
            }
        }
        return NULL;
    }

    AudioPlugin& getPlugin(const char* name, int16_t track = -1)
    {
        AudioPlugin* plugin = getPluginPtr(name, track);
        if (!plugin) {
            logInfo("Could not find plugin %s on track %d. List of available plugins:", name, track);
            for (AudioPlugin* plugin : plugins) {
                logInfo("- Found plugin %s on track %d", plugin->name, plugin->track);
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

    void loadPluginAlias(char* value, const char* filename)
    {
        PluginAlias pluginAllocator;

        char* name = strtok(value, " ");
        char* path = strtok(NULL, " ");

        void* handle = dlopen(getFullpath(path, filename).c_str(), RTLD_LAZY);
        if (!handle) {
            logError("Cannot load audio library %s [%s]: %s", path, name, dlerror());
            return;
        }

        dlerror();
        pluginAllocator.name = name;
        pluginAllocator.allocator = [handle](AudioPlugin::Props& props, char* name) {
            void* allocator = dlsym(handle, "allocator");
            return ((AudioPlugin * (*)(AudioPlugin::Props & props, char* name)) allocator)(props, name);
        };
        const char* dlsym_error = dlerror();
        if (dlsym_error) {
            logError("Cannot load symbol: %s", dlsym_error);
            dlclose(handle);
            return;
        }
        pluginAliases.push_back(pluginAllocator);

        logInfo("audio plugin alias loaded: %s", name);
    }

    void loadPlugin(char* value)
    {
        char* name = strtok(value, " ");
        char* pathPtr = strtok(NULL, " ");
        string path = pathPtr ? pathPtr : name;

        // if path end by .so load it as plugin
        if (path.substr(path.length() - 3) == ".so") {
            loadPlugin(name, path.c_str());
        } else {
            // look to alias if plugins is already loaded and allocate it
            for (PluginAlias& pluginAllocator : pluginAliases) {
                if (pluginAllocator.name == path) {
                    plugins.push_back(pluginAllocator.allocator(pluginProps, name));
                    logInfo("audio plugin loaded: %s", plugins.back()->name);
                    return;
                }
            }
            logWarn("Cannot find audio plugin alias %s [%s]", path.c_str(), name);
        }
    }

    void loadPlugin(char* name, const char* path)
    {
        void* handle = dlopen(path, RTLD_LAZY);

        if (!handle) {
            logWarn("Cannot open audio library %s [%s]: %s", path, name, dlerror());
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

        AudioPlugin* instance = ((AudioPlugin * (*)(AudioPlugin::Props & props, char* name)) allocator)(pluginProps, name);
        logInfo("audio plugin loaded: %s", instance->name);

        // plugin.instance->set(0, 0.1f);
        // printf("---> getParamKey: %d\n", plugin.instance->getParamKey("volume"));

        plugins.push_back(instance);
    }

    bool config(char* key, char* value)
    {
        if (plugins.size() > 0) {
            if (plugins.back()->config(key, value)) {
                return true;
                /*//md - `MIDI_CC: CUTOFF b0 4c xx` assign a midi CC command to a given plugin value (this is a generic config). */
            } else if (strcmp(key, "MIDI_CC") == 0) {
                return assignMidiMapping(value);
                /*//md - `MIDI_CHANNEL: 1` assign a midi channel to a given plugin for a note on/off (this is a generic config). */
            } else if (strcmp(key, "MIDI_CHANNEL") == 0) {
                return assignMidiPluginChannel(value);
                /*//md - `TRACK_MIDI_CHANNEL: track channel` assign a midi channel to a given track for a note on/off, e.g. `TRACK_MIDI_CHANNEL: 1 2` */
            } else if (strcmp(key, "TRACK_MIDI_CHANNEL") == 0) {
                return assignMidiTrackChannel(value);
            }
        }
        return false;
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

    void noteOn(uint8_t note, float velocity, NoteTarget target)
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

    void noteOff(uint8_t note, float velocity, NoteTarget target)
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

    void noteOn(uint8_t channel, uint8_t note, float velocity)
    {
        // printf("-------------- noteOn %d %d %f\n", channel, note, velocity);
        for (MidiNoteEvent& target : midiNoteEvents) {
            if (target.channel == channel) {
                if (velocity == 0) {
                    noteOff(note, velocity, target.target);
                } else {
                    noteOn(note, velocity, target.target);
                }
            }
        }
    }

    void noteOff(uint8_t channel, uint8_t note, float velocity)
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

    void start()
    {
        sendEvent(AudioEventType::START);
    }

    void stop()
    {
        sendEvent(AudioEventType::STOP);
    }

    void pause()
    {
        sendEvent(AudioEventType::PAUSE);
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
};

AudioPluginHandler* AudioPluginHandler::instance = NULL;

#endif