#pragma once

#include <algorithm>
#include <fstream>
#include <iostream>
#include <string>
#include <type_traits>
#include <vector>

#include "audio/sequencer/Clip.h"
#include "audio/sequencer/ClipChain.h"
#include "audio/sequencer/Step.h"
#include "libs/nlohmann/json.hpp"

#ifndef CURRENT_FILE
#define CURRENT_FILE std::string(".current")
#endif

using json = nlohmann::json;

template <typename T, typename = void>
struct has_noteRepeat : std::false_type {};
template <typename T>
struct has_noteRepeat<T, std::void_t<decltype(std::declval<T>().noteRepeat)>> : std::true_type {};

template <typename T, typename = void>
struct has_chain : std::false_type {};
template <typename T>
struct has_chain<T, std::void_t<decltype(std::declval<T>().chain)>> : std::true_type {};

inline void to_json(json& j, const Step& s)
{
    j = json { { "note", s.note }, { "vel", s.velocity }, { "prob", s.condition }, { "len", s.len }, { "active", s.active } };
}

inline void from_json(const json& j, Step& s)
{
    s.note = j.value("note", 60);
    s.velocity = j.value("vel", 0.8f);
    s.condition = j.value("prob", 1.0f);
    s.len = j.value("len", 1.0f);
    s.active = j.value("active", false);
}

inline std::string getCurrentLoadedProject(const std::string& projectFolder)
{
    std::string currentLoadedFile = "";
    std::ifstream currentFile(projectFolder + "/" + CURRENT_FILE);
    if (currentFile.good()) {
        std::getline(currentFile, currentLoadedFile);
    }
    return currentLoadedFile;
}

inline void setCurrentLoadedProject(const std::string& filename, const std::string& projectFolder)
{
    std::ofstream currentFile(projectFolder + "/" + CURRENT_FILE);
    currentFile << filename;
}

template <typename TrackType>
inline void saveClip(TrackType& trk, int clipIdx)
{
    if (clipIdx < 0 || clipIdx >= MAX_CLIP_COUNT) return;
    Clip& c = trk.clips[clipIdx];
    c.paramValues.clear();
    if (trk.engine) {
        Param* params = trk.engine->getParams();
        for (size_t i = 0; i < trk.engine->getParamCount(); i++) {
            if (params[i].setStringFn != nullptr) {
                char str[256];
                params[i].setString(params[i].value, str);
                c.paramValues.push_back({ params[i].key, params[i].value, str });
            } else {
                c.paramValues.push_back({ params[i].key, params[i].value });
            }
        }
    }
    c.saved = true;
    c.sequence = trk.sequence;
    c.engineId = trk.currentEngineIdx;
    if constexpr (has_noteRepeat<TrackType>::value) {
        c.noteRepeat = trk.noteRepeat;
    }
    trk.activeClipIdx = clipIdx;
}

template <typename TrackType>
inline void loadClip(TrackType& trk, int clipIdx)
{
    if (clipIdx < 0 || clipIdx >= MAX_CLIP_COUNT) return;
    if (trk.activeClipIdx != clipIdx) {
        saveClip(trk, trk.activeClipIdx);
    }

    Clip& c = trk.clips[clipIdx];
    trk.setEngine(c.engineId);
    if constexpr (has_noteRepeat<TrackType>::value) {
        trk.noteRepeat = c.noteRepeat;
    }

    if (trk.engine) {
        Param* params = trk.engine->getParams();
        if (!c.validated) {
            size_t paramCount = trk.engine->getParamCount();
            std::vector<ParamValue> validatedParams;
            validatedParams.reserve(paramCount);
            for (size_t i = 0; i < paramCount; i++) {
                std::string expectedKey = params[i].key;
                auto it = std::find_if(c.paramValues.begin(), c.paramValues.end(),
                    [&expectedKey](const ParamValue& pv) {
                        return pv.key == expectedKey;
                    });

                if (it != c.paramValues.end()) {
                    ParamValue pv = *it;
                    if (params[i].stringToFloatFn != nullptr && !pv.string.empty()) {
                        pv.value = params[i].stringToFloat(pv.string.c_str());
                    }
                    validatedParams.push_back(pv);
                }
            }
            c.paramValues = std::move(validatedParams);
            c.validated = true;
        }

        for (size_t i = 0; i < c.paramValues.size(); i++) {
            params[i].set(c.paramValues[i].value);
        }
    }

    if (c.sequence.empty()) {
        c.sequence.assign(SEQ_STEPS, Step());
    }
    trk.sequence = c.sequence;
    trk.activeClipIdx = clipIdx;
}

template <typename StudioType>
inline void saveAllClips(StudioType& studio)
{
    for (int t = 0; t < MAX_TRACKS; t++) {
        auto& trk = *studio.tracks[t];
        saveClip(trk, trk.activeClipIdx);
    }
}

template <typename StudioType>
inline void loadProjectFromJSON(StudioType& studio, const std::string& path)
{
    std::string cleanPath = path;
    cleanPath.erase(std::remove(cleanPath.begin(), cleanPath.end(), '\n'), cleanPath.end());

    std::ifstream in(cleanPath);
    if (!in.is_open()) return;

    json project;
    try {
        in >> project;
    } catch (json::parse_error&) {
        return;
    }

    std::lock_guard<std::mutex> lock(studio.audioMutex);

    if (project.contains("bpm")) {
        studio.updateClock(project["bpm"].get<float>());
    }

    if (!project.contains("tracks")) return;
    auto jTracks = project["tracks"];
    for (int t = 0; t < MAX_TRACKS && t < (int)jTracks.size(); t++) {
        auto& trk = *studio.tracks[t];
        using TrackT = std::remove_reference_t<decltype(trk)>;

        auto jTrk = jTracks[t];

        for (int c = 0; c < MAX_CLIP_COUNT; c++) {
            trk.clips[c].saved = false;
            trk.clips[c].name = "Clip " + std::to_string(c + 1);
        }

        trk.activeClipIdx = jTrk.value("activeClipIdx", 0);
        if constexpr (has_chain<TrackT>::value) {
            trk.chain = jTrk.value("chain", std::vector<int>());
            trk.chainLoopMode = jTrk.value("chainLoopMode", 0);
            trk.chainActiveIdx = trk.chainPlaying ? 0 : -1;
            trk.chainMuted = (trk.chainPlaying && !trk.chain.empty() && trk.chain[0] == -1);
        }

        if (!jTrk.contains("clips")) continue;
        auto jClips = jTrk["clips"];

        for (int c = 0; c < MAX_CLIP_COUNT && c < (int)jClips.size(); c++) {
            Clip& clip = trk.clips[c];
            clip.validated = false;
            auto jClip = jClips[c];

            if (jClip.contains("name")) {
                clip.name = jClip["name"].get<std::string>();
            }

            clip.engineId = 0;
            std::string engineName = jClip.value("engine", "");
            for (int e = 0; e < ENGINE_REGISTRY_COUNT; e++) {
                if (engineRegistry[e].name == engineName) {
                    clip.engineId = e;
                    break;
                }
            }
            clip.saved = jClip.value("saved", false);
            clip.noteRepeat = jClip.value("noteRepeat", 2);

            if (clip.saved) {
                clip.sequence = jClip["sequence"].get<std::vector<Step>>();
                clip.paramValues.clear();
                if (jClip.contains("params") && jClip["params"].is_object()) {
                    json jParams = jClip["params"];
                    for (auto& [key, jParam] : jParams.items()) {
                        if (jParam.is_object()) {
                            clip.paramValues.push_back({ key, jParam["f"].get<float>(), jParam["s"].get<std::string>() });
                        } else {
                            clip.paramValues.push_back({ key, jParam.get<float>() });
                        }
                    }
                }
            }
        }
        loadClip(trk, trk.activeClipIdx);
    }

    studio.projectPath = cleanPath;
}

template <typename StudioType>
inline void saveProjectToJSON(StudioType& studio, const std::string& path)
{
    saveAllClips(studio);

    json project;
    project["bpm"] = studio.bpm.load();
    project["tracks"] = json::array();

    for (int t = 0; t < MAX_TRACKS; t++) {
        auto& trk = *studio.tracks[t];
        using TrackT = std::remove_reference_t<decltype(trk)>;

        json jTrk;
        jTrk["activeClipIdx"] = trk.activeClipIdx;
        if constexpr (has_chain<TrackT>::value) {
            jTrk["chain"] = trk.chain;
            jTrk["chainLoopMode"] = trk.chainLoopMode;
        }
        jTrk["clips"] = json::array();
        for (int c = 0; c < MAX_CLIP_COUNT; c++) {
            Clip& clip = trk.clips[c];
            json jClip;
            jClip["name"] = clip.name;
            jClip["saved"] = clip.saved;
            jClip["engine"] = engineRegistry[trk.currentEngineIdx].name;
            jClip["noteRepeat"] = clip.noteRepeat;
            if (clip.saved) {
                jClip["sequence"] = clip.sequence;
                json jParams = json::object();
                for (size_t i = 0; i < clip.paramValues.size(); i++) {
                    if (clip.paramValues[i].string.empty()) {
                        jParams[clip.paramValues[i].key] = clip.paramValues[i].value;
                    } else {
                        json sValue = json::object();
                        sValue["f"] = clip.paramValues[i].value;
                        sValue["s"] = clip.paramValues[i].string;
                        jParams[clip.paramValues[i].key] = sValue;
                    }
                }
                jClip["params"] = jParams;
                jTrk["clips"].push_back(jClip);
            }
        }
        project["tracks"].push_back(jTrk);
    }

    std::ofstream out(path);
    if (out.is_open()) {
        out << project.dump(2);
        out.close();
        studio.projectPath = path;
    }
}
