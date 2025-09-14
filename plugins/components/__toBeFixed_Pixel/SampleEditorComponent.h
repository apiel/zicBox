#ifndef _UI_PIXEL_COMPONENT_SAMPLE_EDITOR_H_
#define _UI_PIXEL_COMPONENT_SAMPLE_EDITOR_H_

#include "helpers/format.h"
#include "helpers/range.h"
#include "plugins/components/base/KeypadLayout.h"
#include "plugins/components/component.h"
#include "plugins/components/utils/color.h"

#include "plugins/audio/utils/utils.h"

#include <algorithm>
#include <cmath>
#include <sndfile.h>
#include <string>
#include <vector>

/*md
## SampleEditor

SampleEditor components to edit sample...
*/

class SampleEditorComponent : public Component {
protected:
    Color background;
    Color avgColor;
    Color rawColor;
    Color beatColor;
    Color valueColor;
    Color overlayColor;

    KeypadLayout keypadLayout;

    std::string folder = "samples";
    std::string filename = "track";
    SNDFILE* sndfile = nullptr;
    SF_INFO sfinfo;

    ValueInterface* valBpm = NULL;
    ValueInterface* valTrack = NULL;

    AudioPlugin* tapePlugin = NULL;
    uint8_t playStopDataId = 0;
    uint8_t syncDataId = 0;
    int* watcherPtr = NULL;
    int lastWatchState = -1;

    std::vector<float> avgBuffer;
    std::vector<float> rawBuffer;

    int currentBeat = 0;
    int beatLength = 4;
    int totalBeat = 0;
    int samplesPerBeat = 0;
    float beatStart = 0;
    float beatEnd = -1.0f;

    // void loadAudioFile()
    // {
    //     std::string filepath = folder + "/tape_tmp/" + filename + ".wav";
    //     sndfile = sf_open(filepath.c_str(), SFM_READ, &sfinfo);
    //     if (!sndfile) {
    //         // Handle file open error
    //         return;
    //     }

    //     // Calculate the number of samples per beat and total samples to load
    //     int sampleRate = sfinfo.samplerate;
    //     int channels = sfinfo.channels;
    //     samplesPerBeat = (sampleRate * 60) / (valBpm != NULL ? valBpm->get() : 120);
    //     int totalSamples = samplesPerBeat * beatLength;
    //     // roud up
    //     totalBeat = sfinfo.frames / samplesPerBeat + 1;
    //     if (beatEnd < 0) {
    //         beatEnd = totalBeat;
    //     }

    //     // Seek to the currentBeating point
    //     sf_seek(sndfile, currentBeat * samplesPerBeat, SEEK_SET);

    //     // Resize the buffer to match the width of the UI component
    //     avgBuffer.resize(size.w);
    //     rawBuffer.resize(size.w);

    //     // Temporary buffer to read audio samples
    //     std::vector<float> tempBuffer(totalSamples * channels);
    //     sf_read_float(sndfile, tempBuffer.data(), totalSamples * channels);

    //     // Downsample using averaging
    //     int samplesPerPixel = totalSamples / size.w;
    //     for (int i = 0; i < size.w; ++i) {
    //         float sum = 0.0f;
    //         int count = 0;

    //         for (int j = 0; j < samplesPerPixel; ++j) {
    //             int sampleIndex = (i * samplesPerPixel + j) * channels; // First channel
    //             if (sampleIndex < tempBuffer.size()) {
    //                 sum += tempBuffer[sampleIndex];
    //                 ++count;
    //             }
    //         }

    //         avgBuffer[i] = (count > 0) ? (sum / count) : 0.0f; // Avoid division by zero

    //         int sampleIndex = i * (totalSamples / size.w);
    //         float sampleValue = tempBuffer[sampleIndex * channels]; // Take the first channel
    //         rawBuffer[i] = sampleValue;
    //     }

    //     limitBuffer(avgBuffer.data(), avgBuffer.size(), 0.8);
    //     limitBuffer(rawBuffer.data(), rawBuffer.size(), 0.8);

    //     sf_close(sndfile);
    //     sndfile = nullptr;
    // }

public:
    SampleEditorComponent(ComponentInterface::Props props)
        : Component(props)
        , background(styles.colors.background)
        , avgColor(styles.colors.primary)
        , rawColor(darken(styles.colors.primary, 0.5))
        , beatColor(lighten(styles.colors.background, 0.5))
        , valueColor(applyAlphaColor(styles.colors.background, styles.colors.text, 0.8))
        , overlayColor(alpha(styles.colors.white, 0.2))
        , keypadLayout(this, [&](std::string action) {
            std::function<void(KeypadLayout::KeyMap&)> func = NULL;
            if (action == ".playStop") {
                func = [this](KeypadLayout::KeyMap& keymap) {
                    if (tapePlugin != NULL) {
                        if (KeypadLayout::isReleased(keymap)) {
                            tapePlugin->data(playStopDataId);
                        }
                    }
                };
            } else if (action == ".left") {
                func = [this](KeypadLayout::KeyMap& keymap) {
                    if (KeypadLayout::isReleased(keymap)) {
                        incCurrentBeat(-1);
                    }
                };
            } else if (action == ".right") {
                func = [this](KeypadLayout::KeyMap& keymap) {
                    if (KeypadLayout::isReleased(keymap)) {
                        incCurrentBeat(1);
                    }
                };
            }
            return func;
        })
    {
    }

    void syncData()
    {
        if (tapePlugin != NULL) {
            struct PlayData {
                sf_count_t start;
                sf_count_t end;
            };
            PlayData playData = { (sf_count_t)(beatStart * samplesPerBeat), (sf_count_t)(beatEnd * samplesPerBeat) };
            tapePlugin->data(syncDataId, &playData);
        }
    }

    void renderWavFile(int y, int h)
    {
        // Draw beat markers
        int samplesPerBeat = size.w / beatLength;
        for (int b = 0; b <= beatLength; ++b) {
            int x = relativePosition.x + b * samplesPerBeat;
            draw.line({ x, y }, { x, y + h }, { beatColor });
        }

        if (!rawBuffer.empty()) {
            std::vector<Point> waveformPoints;
            for (int i = 0; i < rawBuffer.size(); ++i) {
                float normalizedValue = (rawBuffer[i] + 1.0f) / 2.0f; // Normalize to 0-1
                int yy = y + h - static_cast<int>(normalizedValue * h);
                waveformPoints.push_back({ relativePosition.x + i, yy });
            }
            draw.lines(waveformPoints, { rawColor });
        }

        if (!avgBuffer.empty()) {
            std::vector<Point> waveformPoints;
            for (int i = 0; i < avgBuffer.size(); ++i) {
                float normalizedValue = (avgBuffer[i] + 1.0f) / 2.0f; // Normalize to 0-1
                int yy = y + h - static_cast<int>(normalizedValue * h);
                waveformPoints.push_back({ relativePosition.x + i, yy });
            }
            draw.lines(waveformPoints, { avgColor });
        }
    }

    void renderOverlay(int y, int h)
    {
        if (beatStart > currentBeat) {
            float count = beatStart - currentBeat;
            int w = count > beatLength ? size.w : count / beatLength * size.w;
            draw.filledRect({ relativePosition.x, y }, { w, h }, { overlayColor });
        }

        if (beatEnd < currentBeat + beatLength && beatEnd < totalBeat) {
            float count = currentBeat - beatEnd + beatLength;
            int w = count > beatLength ? size.w : (count / beatLength * size.w);
            draw.filledRect({ relativePosition.x + size.w - w, y }, { w, h }, { overlayColor });
        }
    }

    void render()
    {
        // if (lastWatchState != *watcherPtr) {
        //     lastWatchState = *watcherPtr;
        //     // loadAudioFile();
        // }
        draw.filledRect(relativePosition, size, { background });

        Color bg2 = lighten(background, 0.2);
        int w = size.w / 4;
        draw.filledRect({ relativePosition.x, relativePosition.y }, { w, size.h }, { bg2 });
        draw.filledRect({ relativePosition.x + w * 2, relativePosition.y }, { w, size.h }, { bg2 });

        int space = w / 4;
        for (int x = space; x < size.w; x += space) {
            draw.line({ x, relativePosition.y }, { x, relativePosition.y + size.h }, { beatColor });
        }

        // renderWavFile(relativePosition.y, size.h);
        // renderOverlay(relativePosition.y, size.h);

        // draw.text({ relativePosition.x + 4, relativePosition.y + 2 }, std::to_string(currentBeat) + "/" + std::to_string(totalBeat), 8, { valueColor });
        // if (valTrack != NULL) {
        //     draw.textRight({ relativePosition.x + size.w - 4, relativePosition.y + 2 }, "Track " + std::to_string((int)valTrack->get()), 8, { valueColor });
        // }

        // int bottomTextY = relativePosition.y + size.h - 10;
        // draw.text({ relativePosition.x + 4, bottomTextY }, "Start: " + fToString(beatStart, 2), 8, { valueColor });
        // draw.textRight({ relativePosition.x + size.w - 4, bottomTextY }, "End: " + fToString(beatEnd, 2), 8, { valueColor });
    }

    uint8_t beatEncoderId = 0;
    uint8_t trackEncoderId = 2;
    uint8_t startEncoderId = 1;
    uint8_t endEncoderId = 3;

    // void onEncoder(int id, int8_t direction)
    // {
    //     if (id == beatEncoderId) {
    //         incCurrentBeat(direction);
    //     } else if (id == trackEncoderId) {
    //         if (valTrack != NULL) {
    //             valTrack->set(valTrack->get() + direction);
    //             renderNext();
    //         }
    //     } else if (id == startEncoderId) {
    //         beatStart += direction * 0.25f;
    //         beatStart = CLAMP(beatStart, 0.0f, beatEnd);
    //         syncData();
    //         renderNext();
    //     } else if (id == endEncoderId) {
    //         beatEnd += direction * 0.25f;
    //         beatEnd = CLAMP(beatEnd, beatStart, totalBeat);
    //         syncData();
    //         renderNext();
    //     }
    // }

    void incCurrentBeat(int8_t direction)
    {
        currentBeat += direction;
        currentBeat = CLAMP(currentBeat, 0, totalBeat - 1);
        // loadAudioFile();
        renderNext();
    }

    void onKey(uint16_t id, int key, int8_t state, unsigned long now)
    {
        keypadLayout.onKey(id, key, state, now);
    }

    /*md **Config**: */
    bool config(char* key, char* value)
    {
        if (keypadLayout.config(key, value)) {
            return true;
        }

        /*md - `BACKGROUND_COLOR: background` is the color of the background of the component. */
        if (strcmp(key, "BACKGROUND_COLOR") == 0) {
            background = draw.getColor(value);
            return true;
        }

        /*md - `TAPE_FOLDER` set samples folder path. */
        if (strcmp(key, "TAPE_FOLDER") == 0) {
            folder = value;
            return true;
        }

        /*md - `FILENAME: filename` to set filename. By default it is `track`.*/
        if (strcmp(key, "FILENAME") == 0) {
            filename = value;
            // loadAudioFile();
            return true;
        }

        /*md - `BEAT_ENCODER: encoderId` to set the encoder id to define the beat where to start to display the waveform (default: 0).*/
        if (strcmp(key, "BEAT_ENCODER") == 0) {
            beatEncoderId = atoi(value);
            return true;
        }

        /*md - `TRACK_ENCODER: encoderId` to set the encoder id to define the track number to record (default: 2).*/
        if (strcmp(key, "TRACK_ENCODER") == 0) {
            trackEncoderId = atoi(value);
            return true;
        }

        /*md - `BPM_VALUE: plugin key` to get the bpm value.*/
        if (strcmp(key, "BPM_VALUE") == 0) {
            char* pluginName = strtok(value, " ");
            char* keyValue = strtok(NULL, " ");
            valBpm = watch(getPlugin(pluginName, track).getValue(keyValue));

            return true;
        }

        // /*md - `TAPE_PLUGIN: plugin [playStopDataId] [syncDataId]` to set the play/stop data id of the tape plugin.*/
        // if (strcmp(key, "TAPE_PLUGIN") == 0) {
        //     char* pluginName = strtok(value, " ");
        //     tapePlugin = &getPlugin(pluginName, track);
        //     char* playStopDataIdStr = strtok(NULL, " ");
        //     playStopDataId = tapePlugin->getDataId(playStopDataIdStr ? playStopDataIdStr : "PLAY_STOP");
        //     char* syncDataIdStr = strtok(NULL, " ");
        //     syncDataId = tapePlugin->getDataId(syncDataIdStr ? syncDataIdStr : "SYNC");
        //     watcherPtr = (int*)tapePlugin->data(syncDataId);

        //     valTrack = watch(tapePlugin->getValue("TRACK"));
        //     return true;
        // }

        return false;
    }
};

#endif
