#pragma once

#include "zic23/studio.h"
#include "zic23/uiMessage.h"

#include <fstream>

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
        params[i].value = c.paramValues[i];
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

void saveProject(std::string path)
{
    std::ofstream out(path, std::ios::binary);
    if (!out) return;

    float bpm = studio.bpm.load();
    out.write((char*)&bpm, sizeof(float));

    saveAllClips();

    for (int t = 0; t < MAX_TRACKS; t++) {
        Track& trk = *studio.tracks[t];
        // Save All 32 Clips
        for (int c = 0; c < 32; c++) {
            Clip& clip = trk.clips[c];
            out.write((char*)&clip.saved, sizeof(bool));

            int pValSize = clip.paramValues.size();
            out.write((char*)&pValSize, sizeof(int));
            out.write((char*)clip.paramValues.data(), pValSize * sizeof(float));

            out.write((char*)clip.sequence.data(), clip.sequence.size() * sizeof(Step));
        }
    }
    out.close();
    studio.projectPath = path;
    showMessage("Project Saved");
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

void loadProject()
{
    // 1. Open Native Open Dialog
    char filename[1024];
    FILE* f = popen("zenity --file-selection --file-filter='Zic Project | *.zic'", "r");
    if (!f || fgets(filename, 1024, f) == NULL) {
        if (f) pclose(f);
        return;
    }
    pclose(f);
    std::string path = filename;
    path.erase(std::remove(path.begin(), path.end(), '\n'), path.end());

    // 2. Read from Binary File
    std::ifstream in(path, std::ios::binary);
    if (!in) return;

    std::lock_guard<std::mutex> lock(studio.audioMutex); // Stop audio glitches while loading

    float bpm;
    in.read((char*)&bpm, sizeof(float));
    studio.bpm.store(bpm);

    for (int t = 0; t < MAX_TRACKS; t++) {
        Track& trk = *studio.tracks[t];

        // Load Engine Params
        int pCount;
        in.read((char*)&pCount, sizeof(int));
        Param* params = trk.engine->getParams();
        for (int i = 0; i < pCount; i++) {
            float val;
            in.read((char*)&val, sizeof(float));
            if (i < (int)trk.engine->getParamCount()) params[i].value = val;
        }

        // Load Sequence
        int seqSize;
        in.read((char*)&seqSize, sizeof(int));
        trk.sequence.resize(seqSize);
        in.read((char*)trk.sequence.data(), seqSize * sizeof(Step));

        // Load Clips
        for (int c = 0; c < 32; c++) {
            Clip& clip = trk.clips[c];
            in.read((char*)&clip.saved, sizeof(bool));

            int pValSize;
            in.read((char*)&pValSize, sizeof(int));
            clip.paramValues.resize(pValSize);
            in.read((char*)clip.paramValues.data(), pValSize * sizeof(float));

            in.read((char*)clip.sequence.data(), clip.sequence.size() * sizeof(Step));
        }
    }
    in.close();
    studio.projectPath = path;
    showMessage("Project Loaded");
}