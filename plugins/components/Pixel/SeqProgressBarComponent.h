#ifndef _UI_PIXEL_COMPONENT_SEQ_PROGRESS_BAR_H_
#define _UI_PIXEL_COMPONENT_SEQ_PROGRESS_BAR_H_

#include "helpers/midiNote.h"
#include "plugins/audio/stepInterface.h"
#include "plugins/components/base/KeypadLayout.h"
#include "plugins/components/component.h"
#include "plugins/components/utils/color.h"

/*md
## SeqProgressBarComponent

<img src="https://raw.githubusercontent.com/apiel/zicBox/main/plugins/components/Pixel/seqProgressBar.png" />

*/

class SeqProgressBarComponent : public Component {
protected:
    uint16_t stepCount = 32;
    int16_t lastStepCounter = -1;
    uint16_t* stepCounter = NULL;
    bool* seqPlayingPtr = NULL;
    std::vector<Step>* steps = NULL;

    KeypadLayout keypadLayout;

    Color background;
    Color selectionColor;
    Color foreground;
    Color activeColor;
    Color inactiveStepColor;

    ValueInterface* valVolume = NULL;
    bool showSteps = false;

public:
    SeqProgressBarComponent(ComponentInterface::Props props)
        : Component(props)
        , background(styles.colors.background)
        , selectionColor(styles.colors.white)
        , foreground(darken(styles.colors.primary, 0.5))
        , activeColor(styles.colors.primary)
        , inactiveStepColor({ 0x40, 0x40, 0x40 })
        , keypadLayout(this, [&](std::string action) {
            std::function<void(KeypadLayout::KeyMap&)> func = NULL;
            return func;
        })
    {
        jobRendering = [this](unsigned long now) {
            if (seqPlayingPtr != NULL) {
                if (*seqPlayingPtr) {
                    if (stepCounter != NULL && lastStepCounter != *stepCounter) {
                        lastStepCounter = *stepCounter;
                        renderNext();
                    }
                } else {
                    if (lastStepCounter != -1) {
                        renderNext();
                    }
                    lastStepCounter = -1;
                }
            }
        };

        /*md md_config:SeqProgressBar */
        nlohmann::json& config = props.config;

        /// The audio plugin sequencer.
        AudioPlugin* seqPlugin = getPluginPtr(config, "audioPlugin", track); //eg: "audio_plugin_name"
        stepCount = *(uint16_t*)seqPlugin->data(seqPlugin->getDataId("STEP_COUNT"));
        stepCounter = (uint16_t*)seqPlugin->data(seqPlugin->getDataId("STEP_COUNTER"));
        seqPlayingPtr = (bool*)seqPlugin->data(seqPlugin->getDataId("IS_PLAYING"));
        steps = (std::vector<Step>*)seqPlugin->data(seqPlugin->getDataId("STEPS"));

        if (config.contains("volumePlugin")) { //eg: { "plugin": "Volume", "param": "VOLUME" }
            valVolume = watch(getPlugin(config["volumePlugin"]["plugin"].get<std::string>(), track).getValue(config["volumePlugin"]["param"].get<std::string>().c_str()));
        }

        /// The background color of the text.
        background = draw.getColor(config["bgColor"], background); //eg: "#000000"

        /// The foreground color of the text.
        foreground = draw.getColor(config["fgColor"], foreground); //eg: "#ffffff"

        /// The color of the active step.
        activeColor = draw.getColor(config["activeColor"], activeColor); //eg: "#ffffff"

        /// The color of the inactive step.
        inactiveStepColor = draw.getColor(config["inactiveStepColor"], inactiveStepColor); //eg: "#ffffff"

        /// The color of the selection.
        selectionColor = draw.getColor(config["selectionColor"], selectionColor); //eg: "#ffffff"

        /// Show sequencer step value.
        showSteps = config.value("showSteps", showSteps); //eg: true

        /*md md_config_end */
    }

    void render() override
    {
        draw.filledRect(relativePosition, size, { background });

        int stepW = 4;
        int stepH = size.h;

        int textY = (size.h - 8) * 0.5 + relativePosition.y;

        int stepsW = stepCount * (stepW + 2 + 0.5); // 2 / 4 adding 2 pixel every 4 steps
        int nameW = size.w - stepsW - 5;
        int x = relativePosition.x + 1;

        if (valVolume != NULL) {
            draw.filledRect({ x, relativePosition.y }, { nameW, stepH }, { darken(activeColor, 0.5) });
            draw.filledRect({ x, relativePosition.y }, { (int)(nameW * valVolume->pct()), stepH }, { activeColor });
            draw.rect({ x, relativePosition.y }, { nameW, stepH - 1 }, { selectionColor });
        }

        x += nameW + 4;

        for (int i = 0; i < stepCount; i++) {
            Color color = lastStepCounter == i ? activeColor : (!showSteps || getStep(i) != NULL ? foreground : inactiveStepColor);
            draw.filledRect({ x, relativePosition.y }, { stepW, stepH }, { color });
            x += stepW + 2;
            if (i % 4 == 3) {
                x += 2;
            }
        }
    }

    Step* getStep(uint16_t position)
    {
        for (auto& step : *steps) {
            if (step.position == position && step.enabled && step.len > 0) {
                return &step;
            }
        }
        return NULL;
    }

    bool onKey(uint16_t id, int key, int8_t state, unsigned long now) override
    {
        return keypadLayout.onKey(id, key, state, now);
    }
};

#endif
