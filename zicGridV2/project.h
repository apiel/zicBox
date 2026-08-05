#pragma once

#include "zicGridV2/studio.h"
#include "audio/sequencer/ProjectIO.h"

inline std::string getCurrentLoadedProject()
{
    return getCurrentLoadedProject(PROJECT_FOLDER);
}

inline void setCurrentLoadedProject(const std::string& filename)
{
    setCurrentLoadedProject(filename, PROJECT_FOLDER);
}

inline void saveClip(int trackIdx, int clipIdx)
{
    Track& trk = *studio.tracks[trackIdx];
    saveClip(trk, clipIdx);
}

inline void loadClip(int trackIdx, int clipIdx)
{
    Track& trk = *studio.tracks[trackIdx];
    loadClip(trk, clipIdx);
}

inline void loadProject(std::string path)
{
    loadProjectFromJSON(studio, path);
}

inline void saveProject(std::string path)
{
    saveProjectToJSON(studio, path);
}

inline void createEmptyProject(const std::string& path)
{
    json project;
    project["bpm"] = 125.0f;
    project["tracks"] = json::array();

    std::vector<Step> emptySeq(SEQ_STEPS); // 32 inactive steps (active=false, note=60, vel=0.8, prob=1.0, len=1.0)

    for (int t = 0; t < MAX_TRACKS; t++) {
        json jTrk;
        jTrk["activeClipIdx"] = 0;
        jTrk["chain"] = json::array();
        jTrk["chainLoopMode"] = 0;
        jTrk["clips"] = json::array();
        for (int c = 0; c < MAX_CLIP_COUNT; c++) {
            json jClip;
            jClip["name"] = "Clip " + std::to_string(c + 1);
            jClip["saved"] = true;
            jClip["engine"] = engineRegistry[t % ENGINE_REGISTRY_COUNT].name;
            jClip["noteRepeat"] = 2;
            jClip["sequence"] = emptySeq;
            jClip["params"] = json::object();
            jTrk["clips"].push_back(jClip);
        }
        project["tracks"].push_back(jTrk);
    }

    std::ofstream out(path);
    if (out.is_open()) {
        out << project.dump(2);
        out.close();
    }
}

inline bool saveProject()
{
    std::string current = getCurrentLoadedProject(PROJECT_FOLDER);
    if (current.empty()) {
        return false;
    }
    std::string filepath = PROJECT_FOLDER + "/" + current;
    saveProject(filepath);
    return true;
}

inline bool loadProject()
{
    std::string current = getCurrentLoadedProject(PROJECT_FOLDER);
    if (current.empty()) {
        return false;
    }
    std::string filepath = PROJECT_FOLDER + "/" + current;
    loadProject(filepath);
    return true;
}
