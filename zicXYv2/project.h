#pragma once

#include "zicXYv2/studio.h"

#include "libs/nlohmann/json.hpp"

#include <fstream>
#include <iostream>

#define PROJECT_FOLDER std::string("../data/workspaces/rack")
#define CURRENT_FILE std::string(".current")

using json = nlohmann::json;

std::string getCurrentLoadedProject()
{
    std::string currentLoadedFile = "";

    std::ifstream currentFile(PROJECT_FOLDER + "/" + CURRENT_FILE);

    if (currentFile.good()) {
        std::getline(currentFile, currentLoadedFile);
    }

    return currentLoadedFile;
}

void setCurrentLoadedProject(const std::string& filename)
{
    std::ofstream currentFile(PROJECT_FOLDER + "/" + CURRENT_FILE);
    currentFile << filename;
}

void saveClip(Track& trk, int clipIdx)
{
    Clip& c = trk.clips[clipIdx];
    c.paramValues.clear();
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
    c.saved = true;
    c.sequence = trk.sequence;
    c.engineId = trk.currentEngineIdx;
    trk.activeClipIdx = clipIdx;
}

void saveClip(int trackIdx, int clipIdx)
{
    Track& trk = *studio.tracks[trackIdx];
    saveClip(trk, clipIdx);
}

void loadClip(Track& trk, int clipIdx)
{
    if (trk.activeClipIdx != clipIdx) {
        saveClip(trk, trk.activeClipIdx);
    }

    Clip& c = trk.clips[clipIdx];

    trk.setEngine(c.engineId);

    Param* params = trk.engine->getParams();
    if (!c.validated) {
        size_t paramCount = trk.engine->getParamCount();
        std::vector<ParamValue> validatedParams;
        validatedParams.reserve(trk.engine->getParamCount());
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
    trk.sequence = c.sequence;
    trk.activeClipIdx = clipIdx;
}

void loadClip(int trackIdx, int clipIdx)
{
    Track& trk = *studio.tracks[trackIdx];
    loadClip(trk, clipIdx);
}

void saveAllClips()
{
    for (int t = 0; t < MAX_TRACKS; t++) {
        Track& trk = *studio.tracks[t];
        saveClip(t, trk.activeClipIdx);
    }
}

void to_json(json& j, const Step& s)
{
    j = json { { "note", s.note }, { "vel", s.velocity }, { "prob", s.condition }, { "len", s.len }, { "active", s.active } };
}

void from_json(const json& j, Step& s)
{
    s.note = j.value("note", 60);
    s.velocity = j.value("vel", 0.8f);
    s.condition = j.value("prob", 1.0f);
    s.len = j.value("len", 1.0f);
    s.active = j.value("active", false);
}

void loadProject(std::string path)
{
    path.erase(std::remove(path.begin(), path.end(), '\n'), path.end());

    std::ifstream in(path);
    if (!in.is_open()) return;

    json project;
    try {
        in >> project;
    } catch (json::parse_error& e) {
        // showMessage("Load Failed: Invalid JSON");
        return;
    }

    std::lock_guard<std::mutex> lock(studio.audioMutex);

    if (project.contains("bpm")) {
        studio.updateClock(project["bpm"].get<float>());
    }

    auto jTracks = project["tracks"];
    for (int t = 0; t < MAX_TRACKS && t < jTracks.size(); t++) {
        Track& trk = *studio.tracks[t];
        auto jTrk = jTracks[t];

        trk.activeClipIdx = jTrk.value("activeClipIdx", 0);
        auto jClips = jTrk["clips"];

        for (int c = 0; c < MAX_CLIP_COUNT && c < jClips.size(); c++) {
            Clip& clip = trk.clips[c];
            clip.validated = false;
            auto jClip = jClips[c];
            clip.engineId = 0;
            std::string engineName = jClip.value("engine", "");
            for (int e = 0; e < ENGINE_REGISTRY_COUNT; e++) {
                if (engineRegistry[e].name == engineName) {
                    clip.engineId = e;
                    break;
                }
            }
            clip.saved = jClip.value("saved", false);
            if (clip.saved) {
                clip.sequence = jClip["sequence"].get<std::vector<Step>>();
                clip.paramValues.clear();
                if (jClip.contains("params") && jClip["params"].is_object()) {
                    json jParams = jClip["params"];
                    // for (size_t i = 0; i < paramCount; i++) {
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
        loadClip(t, trk.activeClipIdx);
    }

    studio.projectPath = path;
    // showMessage("Project Loaded");
}

void saveProject(std::string path)
{
    saveAllClips();

    json project;
    project["bpm"] = studio.bpm.load();
    project["tracks"] = json::array();

    for (int t = 0; t < MAX_TRACKS; t++) {
        Track& trk = *studio.tracks[t];
        json jTrk;
        jTrk["activeClipIdx"] = trk.activeClipIdx;
        jTrk["clips"] = json::array();
        for (int c = 0; c < MAX_CLIP_COUNT; c++) {
            Clip& clip = trk.clips[c];
            json jClip;
            jClip["saved"] = clip.saved;
            jClip["engine"] = engineRegistry[trk.currentEngineIdx].name;
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
        // showMessage("Project Saved");
    }
}

bool saveProject()
{
    std::string current = getCurrentLoadedProject();
    if (current.empty()) {
        return false;
    }
    std::string filepath = PROJECT_FOLDER + "/" + current;
    saveProject(filepath);
    return true;
}
