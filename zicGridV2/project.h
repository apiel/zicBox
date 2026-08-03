#pragma once

#include "zicGridV2/studio.h"
#include "audio/sequencer/ProjectIO.h"

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
