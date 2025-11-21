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

    // Steps loaded from the active clip
    std::vector<Step> steps;
    uint16_t stepCount = 64;

    // Viewport
    int32_t viewStart = 0; // first visible step
    int32_t viewWidth = 128; // steps visible on screen (auto-filled on resize)
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

    std::string engine = "";
    std::string engineType = "";

    // Encoder config
    int8_t scrollEncoder = -1;

public:
    TimelineComponent(ComponentInterface::Props props)
        : Component(props)
    {
        nlohmann::json& config = props.config;

        timeline.config(config);
        clip.config(config);

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

        sequencerPlugin = config.value("sequencerPlugin", sequencerPlugin);
        enginePlugin = config.value("enginePlugin", enginePlugin);

        fontLane = draw.getFont(config.value("fontLane", "PoppinsLight_8").c_str()); //eg: "PoppinsLight_8"
        int fontSize = draw.getDefaultFontSize(fontLane);
        if (fontSize > 0) {
            fontLaneSize = fontSize;
        }

        resize();
    }

    void resize() override
    {
        viewWidth = size.w / stepPixel;
    }

    void loadClip(int clipId)
    {
        // logDebug("Loading steps from clip %d", clipId);
        steps.clear();

        auto json = clip.hydrate(clip.getFilename(clipId));

        // logDebug("json: %s", json.dump(4).c_str());
        if (!json.contains(sequencerPlugin)) return;
        auto& seqJson = json[sequencerPlugin];

        if (!seqJson.contains("STEPS")) return;

        for (auto& s : seqJson["STEPS"]) {
            Step step;
            step.hydrateJson(s);
            if (step.enabled && step.len > 0)
                steps.push_back(step);
        }

        uint16_t stepCount = seqJson.value("STEP_COUNT", 64);

        // logDebug("Loaded %d steps from clip %d with stepCount %d", steps.size(), clipId, stepCount);

        if (json.contains(enginePlugin)) {
            auto& engineJson = json[enginePlugin];
            engine = engineJson.value("engine", "");
            engineType = engineJson.value("engineType", "");
        }
    }

    void render()
    {
        draw.filledRect(relativePosition, size, { background });

        // GRID lines
        for (int i = 0; i <= viewWidth; i++) {
            int step = viewStart + i;
            int x = relativePosition.x + i * stepPixel;
            Color col = (step % 16 == 0)
                ? barColor
                : (step % 4 == 0) ? gridColor
                                  : darken(gridColor, 0.5);

            draw.line({ x, relativePosition.y },
                { x, relativePosition.y + size.h },
                { col });
        }

        // logDebug("-----------------> Events: %d", timeline.events.size());
        // RENDER EVENTS
        for (auto& ev : timeline.events) {
            if (ev.step < viewStart || ev.step > viewStart + viewWidth)
                continue;

            int x = relativePosition.x + (ev.step - viewStart) * stepPixel;

            if (ev.type == Timeline::EventType::LOAD_CLIP) {
                // Draw preview
                renderClipPreview(x, relativePosition.y + laneHeight + 2, ev.value, ev.step);
            } else if (ev.type == Timeline::EventType::LOOP_BACK) {
                // logDebug("LOOP_BACK: %d", ev.value);
                // LOOP marker
                draw.filledCircle({ x + 2, relativePosition.y + laneHeight / 2 }, 4, { loopColor });
                draw.text({ x + 8, relativePosition.y + (laneHeight - fontLaneSize) / 2 }, "<- " + std::to_string(ev.value), fontLaneSize, { textColor, .font = fontLane });
            }
        }

        // Bottom lane labels
        draw.text({ relativePosition.x + 2, relativePosition.y + size.h - 14 }, "View: " + std::to_string(viewStart) + " - " + std::to_string(viewStart + viewWidth), 12, { textColor });
    }

    // void renderClipPreview(int xStart, int y, int clipId)
    // {
    //     loadClip(clipId);

    //     int maxLen = 1;
    //     for (auto& st : steps)
    //         maxLen = std::max(maxLen, st.position + st.len);

    //     float scale = (float)stepPixel / (float)1; // clip steps drawn in same scale

    //     for (auto& st : steps) {
    //         int x = xStart + (int)(st.position * scale);
    //         int w = std::max(2, (int)(st.len * scale));

    //         draw.filledRect({ x, y }, { w, clipPreviewHeight }, { clipColor });
    //     }
    // }

    void renderClipPreview(int xStart, int y, int clipId, int clipStart)
    {
        loadClip(clipId);
        if (steps.empty())
            return;

        // ---- find clip boundaries ----
        int clipEnd = clipStart + stepCount;

        int minPitch = 128, maxPitch = -1;

        for (auto& st : steps) {
            clipEnd = std::max<int>(clipEnd, st.position + st.len);
            if (st.note >= 0) {
                minPitch = std::min<int>(minPitch, st.note);
                maxPitch = std::max<int>(maxPitch, st.note);
            }
        }

        if (minPitch > maxPitch) {
            minPitch = 60;
            maxPitch = 60;
        }

        int pitchRange = std::max(1, maxPitch - minPitch);
        float pitchScale = (float)clipPreviewHeight / (float)pitchRange;

        // ---- viewport cropping ----
        int viewEnd = viewStart + viewWidth;

        int visibleStart = std::max(clipStart, viewStart);
        int visibleEnd = std::min(clipEnd, viewEnd);

        if (visibleEnd <= visibleStart)
            return;

        // int xA = relativePosition.x + (visibleStart - viewStart) * stepPixel;
        int xA = xStart;
        int xB = relativePosition.x + (visibleEnd - viewStart) * stepPixel;

        int boxWidth = xB - xA;

        // ---- draw clip lane
        draw.text({ xStart, relativePosition.y + (laneHeight - fontLaneSize) / 2 }, "Clip: " + std::to_string(clipId) + " - " + engineType + " " + engine, fontLaneSize, { textColor, .font = fontLane });

        // ---- draw clip bounding box ----
        draw.filledRect({ xA, y }, { boxWidth, clipPreviewHeight }, 6, { darken(clipColor, 0.5) });

        // ---- draw steps (piano roll) ----
        for (auto& st : steps) {
            int stStart = st.position;
            int stEnd = st.position + st.len;

            if (stEnd <= visibleStart || stStart >= visibleEnd)
                continue;

            int clippedStart = std::max(stStart, visibleStart);
            int clippedEnd = std::min(stEnd, visibleEnd);

            int px = relativePosition.x + (clippedStart - viewStart) * stepPixel;
            int pw = std::max(1, (clippedEnd - clippedStart) * stepPixel);

            float pitchY = (float)(maxPitch - st.note) * pitchScale;
            int py = y + (int)pitchY;

            if (py < y) py = y;
            if (py > y + clipPreviewHeight - 3) py = y + clipPreviewHeight - 3;

            draw.filledRect(
                { px, py },
                { pw, 3 },
                { clipColor });
        }
    }

    void onEncoder(int8_t id, int8_t direction) override
    {
        if (id == scrollEncoder) {
            direction = direction > 0 ? 1 : -1;
            viewStart = std::max(0, viewStart + direction * 8);
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
