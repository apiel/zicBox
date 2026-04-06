#pragma once

#include "zic23/studio.h"
#include "zic23/uiMessage.h"

#include "libs/nlohmann/json.hpp"

#include <fstream>
#include <iostream>

using json = nlohmann::json;

sf::IntRect clipRects[MAX_TRACKS][32];
sf::IntRect saveBtnRects[MAX_TRACKS];
sf::IntRect loadBtnRects[MAX_TRACKS];

static constexpr int CLIP_H = 12;

void saveClip(int trackIdx, int clipIdx)
{
    Track& trk = *studio.tracks[trackIdx];
    Clip& c = trk.clips[clipIdx];
    c.paramValues.clear();
    Param* params = trk.engine->getParams();
    for (size_t i = 0; i < trk.engine->getParamCount(); i++) {
        c.paramValues.push_back(params[i].value);
    }
    c.saved = true;
    c.sequence = trk.sequence;
    trk.activeClipIdx = clipIdx;
}

void loadClip(int trackIdx, int clipIdx)
{
    Track& trk = *studio.tracks[trackIdx];
    Clip& c = trk.clips[clipIdx];

    Param* params = trk.engine->getParams();
    for (size_t i = 0; i < c.paramValues.size(); i++) {
        params[i].set(c.paramValues[i]);
    }
    trk.sequence = c.sequence;
    trk.activeClipIdx = clipIdx;
}

void saveAllClips()
{
    for (int t = 0; t < MAX_TRACKS; t++) {
        Track& trk = *studio.tracks[t];
        saveClip(t, trk.activeClipIdx);
    }
}

void reloadAllClips()
{
    for (int t = 0; t < MAX_TRACKS; t++) {
        Track& trk = *studio.tracks[t];
        loadClip(t, trk.activeClipIdx);
    }
    showMessage("Reloaded");
}

void drawClipSelectorUI(Draw& d, sf::Vector2u size, int currentY, int clipStartX, int trackIdx, Track& trk)
{
    int btnX = size.x - MARGIN * 2 - 15;
    saveBtnRects[trackIdx] = { btnX, currentY + 4, 15, 12 };
    d.filledRect({ saveBtnRects[trackIdx].left, saveBtnRects[trackIdx].top }, { 15, 12 }, { .color = { 60, 60, 70 } });
    d.textCentered({ saveBtnRects[trackIdx].left + 7, saveBtnRects[trackIdx].top + 1 }, "S", 8, { .color = { 200, 200, 200 }, .font = &PoppinsLight_8 });

    btnX -= 4 + 15;
    loadBtnRects[trackIdx] = { btnX, currentY + 4, 15, 12 };
    d.filledRect({ loadBtnRects[trackIdx].left, loadBtnRects[trackIdx].top }, { 15, 12 }, { .color = { 60, 60, 70 } });
    d.textCentered({ loadBtnRects[trackIdx].left + 7, loadBtnRects[trackIdx].top + 1 }, "L", 8, { .color = { 200, 200, 200 }, .font = &PoppinsLight_8 });

    int CLIP_W = (btnX - clipStartX - MARGIN) / 32 - 2;
    for (int i = 0; i < 32; i++) {
        clipRects[trackIdx][i] = { clipStartX + i * (CLIP_W + 2), currentY + 4, CLIP_W, CLIP_H };
        Color c = trk.themeColor;
        if (trk.activeClipIdx != i) {
            if (trk.clips[i].saved) {
                c.a = 100;
            } else {
                c = Color { 40, 40, 45 };
            }
        }
        d.filledRect({ clipRects[trackIdx][i].left, clipRects[trackIdx][i].top }, { CLIP_W, CLIP_H }, { .color = c });

        if (trk.selectedClipIdx == i) d.filledRect({ clipRects[trackIdx][i].left, clipRects[trackIdx][i].top + CLIP_H - 3 }, { CLIP_W, 3 }, { .color = { 255, 255, 255 } });
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

void loadProject()
{
    char filename[1024];
    FILE* f = popen("zenity --file-selection --file-filter='Zic Project | *.zic'", "r");
    if (!f || fgets(filename, 1024, f) == NULL) {
        if (f) pclose(f);
        return;
    }
    pclose(f);

    std::string path = filename;
    path.erase(std::remove(path.begin(), path.end(), '\n'), path.end());

    std::ifstream in(path);
    if (!in.is_open()) return;

    json project;
    try {
        in >> project;
    } catch (json::parse_error& e) {
        showMessage("Load Failed: Invalid JSON");
        return;
    }

    std::lock_guard<std::mutex> lock(studio.audioMutex);

    if (project.contains("bpm")) studio.bpm.store(project["bpm"].get<float>());

    auto jTracks = project["tracks"];
    for (int t = 0; t < MAX_TRACKS && t < jTracks.size(); t++) {
        Track& trk = *studio.tracks[t];
        Param* engineParams = trk.engine->getParams();
        size_t engineParamCount = trk.engine->getParamCount();
        auto jTrk = jTracks[t];

        trk.activeClipIdx = jTrk.value("activeClipIdx", 0);
        auto jClips = jTrk["clips"];

        for (int c = 0; c < 32 && c < jClips.size(); c++) {
            Clip& clip = trk.clips[c];
            auto jClip = jClips[c];
            clip.saved = jClip.value("saved", false);
            clip.sequence = jClip["sequence"].get<std::vector<Step>>();

            // Prepare the vector with current engine defaults
            clip.paramValues.clear();
            for (size_t i = 0; i < engineParamCount; i++) {
                clip.paramValues.push_back(engineParams[i].value);
            }

            // Overlay values from JSON by matching keys
            if (jClip.contains("params") && jClip["params"].is_object()) {
                json jParams = jClip["params"];
                for (size_t i = 0; i < engineParamCount; i++) {
                    const std::string& key = engineParams[i].key;
                    if (jParams.contains(key)) {
                        clip.paramValues[i] = jParams[key].get<float>();
                    }
                }
            } else if (jClip.contains("params") && jClip["params"].is_array()) {
                // Legacy support: if it's an old file, just copy the array directly
                clip.paramValues = jClip["params"].get<std::vector<float>>();
            }
        }
        loadClip(t, trk.activeClipIdx);
    }

    studio.projectPath = path;
    showMessage("Project Loaded");
}

void saveProject(std::string path)
{
    saveAllClips();

    json project;
    project["version"] = 2; // Increment version to track the format change
    project["bpm"] = studio.bpm.load();
    project["tracks"] = json::array();

    for (int t = 0; t < MAX_TRACKS; t++) {
        Track& trk = *studio.tracks[t];
        Param* engineParams = trk.engine->getParams();
        size_t paramCount = trk.engine->getParamCount();

        json jTrk;
        jTrk["activeClipIdx"] = trk.activeClipIdx;
        jTrk["clips"] = json::array();

        for (int c = 0; c < 32; c++) {
            Clip& clip = trk.clips[c];
            json jClip;
            jClip["saved"] = clip.saved;
            jClip["sequence"] = clip.sequence;

            // Map vector indices to keys for the JSON file
            json jParams = json::object();
            for (size_t i = 0; i < paramCount && i < clip.paramValues.size(); i++) {
                jParams[engineParams[i].key] = clip.paramValues[i];
            }
            jClip["params"] = jParams;

            jTrk["clips"].push_back(jClip);
        }
        project["tracks"].push_back(jTrk);
    }

    std::ofstream out(path);
    if (out.is_open()) {
        out << project.dump(2);
        out.close();
        studio.projectPath = path;
        showMessage("Project Saved");
    }
}

void saveAsProject()
{
    char filename[1024];
    FILE* f = popen("zenity --file-selection --save --confirm-overwrite --file-filter='Zic Project | *.zic'", "r");
    if (!f || fgets(filename, 1024, f) == NULL) {
        if (f) pclose(f);
        return;
    }
    pclose(f);
    std::string path = filename;
    path.erase(std::remove(path.begin(), path.end(), '\n'), path.end());

    saveProject(path);
}

void saveProject()
{
    if (studio.projectPath != "") {
        saveProject(studio.projectPath);
    } else {
        saveAsProject();
    }
}
