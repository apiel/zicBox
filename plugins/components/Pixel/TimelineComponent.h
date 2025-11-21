#pragma once

#include "helpers/midiNote.h"
#include "plugins/components/component.h"
#include "plugins/components/utils/color.h"

#include "plugins/audio/utils/Clip.h"
#include "plugins/audio/utils/Timeline.h"
#include "plugins/audio/stepInterface.h"

#include <string>
#include <vector>

class TimelineComponent : public Component {
protected:
    Timeline timeline;
    Clip* clip = nullptr;

    // Steps loaded from the active clip
    std::vector<Step> steps;

    // Viewport
    int32_t viewStart = 0; // first visible step
    int32_t viewWidth = 128; // steps visible on screen (auto-filled on resize)
    int stepPixel = 8; // size of each timeline step
    int laneHeight = 24;
    int clipPreviewHeight = 20;

    // Colors
    Color background;
    Color gridColor;
    Color barColor;
    Color clipColor;
    Color loopColor;
    Color textColor;
    Color selectedColor;

    // Encoder config
    int8_t scrollEncoder = -1;

public:
    TimelineComponent(ComponentInterface::Props props)
        : Component(props)
    {
        nlohmann::json& config = props.config;

        timeline.config(config);

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

        resize();
    }

    void resize() override
    {
        viewWidth = size.w / stepPixel;
    }

    void loadClipSteps(int clipId)
    {
        // steps.clear();
        // if (!clip) return;

        // auto json = clip->hydrate(clip->getFilename(clipId));
        // if (!json.contains("STEPS")) return;

        // for (auto& s : json["STEPS"]) {
        //     Step st;
        //     st.hydrateJson(s);
        //     if (st.enabled && st.len > 0)
        //         steps.push_back(st);
        // }
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

        // RENDER EVENTS
        for (auto& ev : timeline.events) {
            if (ev.step < viewStart || ev.step > viewStart + viewWidth)
                continue;

            int x = relativePosition.x + (ev.step - viewStart) * stepPixel;

            if (ev.type == Timeline::EventType::LOAD_CLIP) {
                // Draw clip event marker
                draw.filledRect({ x, relativePosition.y },
                    { 3, laneHeight },
                    { clipColor });

                // Draw preview
                renderClipPreview(x, relativePosition.y + laneHeight + 2, ev.value);
            } else if (ev.type == Timeline::EventType::LOOP_BACK) {
                // LOOP marker
                draw.filledCircle({ x + 2, relativePosition.y + laneHeight / 2 }, 4, { loopColor });
                draw.text({ x + 8, relativePosition.y + laneHeight / 2 - 4 },
                    "<- " + std::to_string(ev.value), 8, { textColor });
            }
        }

        // Bottom lane labels
        draw.text({ relativePosition.x + 2, relativePosition.y + size.h - 10 },
            "View: " + std::to_string(viewStart)
                + " - " + std::to_string(viewStart + viewWidth),
            8, { textColor });
    }

    void renderClipPreview(int xStart, int y, int clipId)
    {
        loadClipSteps(clipId);

        int maxLen = 1;
        for (auto& st : steps)
            maxLen = std::max(maxLen, st.position + st.len);

        float scale = (float)stepPixel / (float)1; // clip steps drawn in same scale

        for (auto& st : steps) {
            int x = xStart + (int)(st.position * scale);
            int w = std::max(2, (int)(st.len * scale));

            draw.filledRect({ x, y }, { w, clipPreviewHeight }, { clipColor });
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
