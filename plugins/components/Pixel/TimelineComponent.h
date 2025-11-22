#pragma once

#include "helpers/midiNote.h"
#include "plugins/components/component.h"
#include "plugins/components/utils/color.h"

#include "plugins/audio/stepInterface.h"
#include "plugins/audio/utils/Clip.h"
#include "plugins/audio/utils/Timeline.h"

#include <string>
#include <vector>

class TimelineComponent : public Component {
protected:
    Timeline timeline;
    Clip clip;

    // Viewport
    int32_t viewStepStart = 0; // first visible step
    int32_t viewStepCount = 128; // steps visible on screen (auto-filled on resize)
    int stepPixel = 8; // size of each timeline step
    int laneHeight = 12;
    int clipPreviewHeight = 20;

    // Colors
    Color background;
    Color gridColor;
    Color barColor;
    Color clipColor;
    Color loopColor;
    Color textColor;
    Color selectedColor;

    void* fontLane = NULL;
    int fontLaneSize = 8;

    std::string sequencerPlugin = "Sequencer";
    std::string enginePlugin = "Track";

    struct ClipData {
        std::vector<Step> steps;
        uint16_t stepCount = 64;
        std::string engine = "";
        std::string engineType = "";
    };

    // Encoder config
    int8_t scrollEncoder
        = -1;

    // ---- Fixed MIDI range C0 (12) to B9 (119) = 108 semitones ----
    const int MIDI_MIN = 12;
    const int MIDI_MAX = 119;
    const float MIDI_RANGE = (float)(MIDI_MAX - MIDI_MIN);

    uint8_t trackContextId = 0;
    uint8_t stepContextId = 0;

    int16_t selectedTrack = 1;
    int32_t selectedStep = 0;

    void loadClips()
    {
        for (auto& event : timeline.events) {
            if (event.type == Timeline::EventType::LOAD_CLIP) {
                auto json = clip.hydrate(clip.getFilename(event.value));

                if (!json.contains(sequencerPlugin)) continue;
                auto& seqJson = json[sequencerPlugin];

                if (!seqJson.contains("STEPS")) continue;

                ClipData* data = new ClipData();
                for (auto& s : seqJson["STEPS"]) {
                    Step step;
                    step.hydrateJson(s);
                    if (step.enabled && step.len > 0)
                        data->steps.push_back(step);
                }

                data->stepCount = seqJson.value("STEP_COUNT", 64);

                if (json.contains(enginePlugin)) {
                    auto& engineJson = json[enginePlugin];
                    data->engine = engineJson.value("engine", "");
                    data->engineType = engineJson.value("engineType", "");
                }
                event.data = data;
            }
        }
    }

public:
    TimelineComponent(ComponentInterface::Props props)
        : Component(props)
    {
        nlohmann::json& config = props.config;

        timeline.config(config);
        clip.config(config);

        sequencerPlugin = config.value("sequencerPlugin", sequencerPlugin);
        enginePlugin = config.value("enginePlugin", enginePlugin);

        loadClips(); // <-------- // TODO how to deal with reload workspace event? // would have to delete clips first

        /// Encoder to scroll left/right
        scrollEncoder = config.value("scrollEncoderId", scrollEncoder);

        // Colors
        background = draw.getColor(config["background"], styles.colors.background);
        gridColor = draw.getColor(config["gridColor"], lighten(styles.colors.background, 0.4));
        barColor = draw.getColor(config["barColor"], lighten(styles.colors.background, 1.0));
        clipColor = draw.getColor(config["clipColor"], styles.colors.primary);
        loopColor = draw.getColor(config["loopColor"], styles.colors.secondary);
        textColor = draw.getColor(config["textColor"], styles.colors.white);
        selectedColor = draw.getColor(config["selectedColor"], styles.colors.white);

        fontLane = draw.getFont(config.value("fontLane", "PoppinsLight_8").c_str()); //eg: "PoppinsLight_8"
        int fontSize = draw.getDefaultFontSize(fontLane);
        if (fontSize > 0) {
            fontLaneSize = fontSize;
        }

        /// Set context id shared between components to show selected track, must be different than 0.
        trackContextId = config.value("trackContextId", trackContextId); //eg: 10

        /// Set context id shared between components to show selected step, must be different than 0.
        stepContextId = config.value("stepContextId", stepContextId); //eg: 11

        /// Default selected track
        selectedTrack = config.value("defaultSelectedTrack", selectedTrack);

        resize();
    }

    void resize() override
    {
        viewStepCount = size.w / stepPixel;

        clipPreviewHeight = size.h - laneHeight - 4;
    }

    void render()
    {
        draw.filledRect(relativePosition, size, { background });

        // GRID lines
        for (int i = 0; i <= viewStepCount; i++) {
            int step = viewStepStart + i;
            int x = relativePosition.x + i * stepPixel;
            Color col = (step % 16 == 0)
                ? barColor
                : (step % 4 == 0) ? gridColor
                                  : darken(gridColor, 0.5);

            draw.line({ x, relativePosition.y }, { x, relativePosition.y + size.h }, { col });
        }

        for (int i = 0; i <= viewStepCount; i++) {
            int step = viewStepStart + i;
            int x = relativePosition.x + i * stepPixel;
            if (step % 16 == 0) {
                draw.text({ x + 2, relativePosition.y }, std::to_string(step), fontLaneSize, { barColor, .font = fontLane });
            }
        }

        // RENDER EVENTS
        for (auto& ev : timeline.events) {
            if (ev.step < viewStepStart && ev.step > viewStepStart + viewStepCount)
                continue;

            int x = relativePosition.x + (ev.step - viewStepStart) * stepPixel;

            if (ev.type == Timeline::EventType::LOAD_CLIP) {
                if (ev.data) {
                    ClipData* clipData = static_cast<ClipData*>(ev.data);

                    draw.filledRect({ x, relativePosition.y }, { 36, laneHeight }, { clipColor });
                    Point textPos = { x + 2, relativePosition.y + (laneHeight - fontLaneSize) / 2 };
                    draw.text(textPos, "Clip: " + std::to_string(ev.value), fontLaneSize, { textColor, .font = fontLane });
                    textPos.x += 38;
                    draw.text(textPos, clipData->engineType + " " + clipData->engine, fontLaneSize, { textColor, .font = fontLane });

                    renderClipPreview(x, relativePosition.y + laneHeight, ev.step, clipData);
                }
            } else if (ev.type == Timeline::EventType::LOOP_BACK) {
                // draw.filledCircle({ x + 2, relativePosition.y + laneHeight / 2 }, 4, { loopColor });
                draw.filledRect({ x, relativePosition.y }, { 36, laneHeight }, { barColor });
                draw.filledRect({ x, relativePosition.y }, { 2, laneHeight }, { loopColor });
                draw.text({ x + 5, relativePosition.y + (laneHeight - fontLaneSize) / 2 }, "<- " + std::to_string(ev.value), fontLaneSize, { textColor, .font = fontLane });
            }
        }
    }

    void renderClipPreview(int xStart, int y, int clipStepStart, ClipData* clipData)
    {
        // ---- viewport cropping ----
        int clipEnd = clipStepStart + clipData->stepCount;
        int viewEnd = viewStepStart + viewStepCount;

        int visibleStart = std::max(clipStepStart, viewStepStart);
        int visibleEnd = std::min(clipEnd, viewEnd);

        if (visibleEnd <= visibleStart)
            return;

        // ---- draw clip bounding box ----
        int xA = relativePosition.x + (visibleStart - viewStepStart) * stepPixel;
        int xB = relativePosition.x + (visibleEnd - viewStepStart) * stepPixel;
        int boxWidth = xB - xA;
        if (visibleEnd < clipEnd) boxWidth += 10; // just to ensure that we dont cut the clip too early making the feeling that there is nothing coming...

        bool isSelected = track == selectedTrack && selectedStep >= clipStepStart && selectedStep < clipEnd;

        draw.filledRect({ xA, y }, { boxWidth, clipPreviewHeight }, { darken(clipColor, isSelected ? 0.3f : 0.5f) });
        draw.rect({ xA, y }, { boxWidth - (isSelected ? 1 : 0), clipPreviewHeight }, { isSelected ? selectedColor : clipColor });

        // ---- draw each note in piano-roll style ----
        if (clipData->steps.size() == 0)
            return;

        for (auto& step : clipData->steps) {
            int stStart = step.position + clipStepStart;
            int stEnd = stStart + step.len;

            // Skip notes outside visible range
            if (stEnd <= visibleStart || stStart >= visibleEnd)
                continue;

            // Clip start/end
            int clippedStart = std::max(stStart, visibleStart);
            int clippedEnd = std::min(stEnd, visibleEnd);

            int px = relativePosition.x + (clippedStart - viewStepStart) * stepPixel;
            int pw = std::max(1, (clippedEnd - clippedStart) * stepPixel);

            // ---- vertical placement using fixed MIDI map ----
            float noteNorm = (float)(step.note - MIDI_MIN) / MIDI_RANGE;
            noteNorm = std::clamp(noteNorm, 0.0f, 1.0f);

            int py = y + (clipPreviewHeight - 1) - (int)(noteNorm * (clipPreviewHeight - 1));
            int ph = 3;

            draw.filledRect({ px, py }, { pw, ph }, { clipColor });
        }
    }

    void onEncoder(int8_t id, int8_t direction) override
    {
        if (id == scrollEncoder) {
            direction = direction > 0 ? 1 : -1;
            viewStepStart = std::max(0, viewStepStart + direction * 8);
            renderNext();
        }
    }

    const std::vector<EventInterface::EncoderPosition> getEncoderPositions() override
    {
        if (scrollEncoder < 0)
            return {};

        return {
            { scrollEncoder, size, relativePosition }
        };
    }
};
