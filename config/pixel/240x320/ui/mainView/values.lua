local visibility = require("config/pixel/libs/containers/visibility")
local value = require("config/pixel/libs/component/value")

local values = {}

-- local valueH = 11
local valueH = 25
function values.main(group, track, y)
    visibility(
        "DrumSample" .. track,
        { x = 0, y = y, w = ScreenWidth, h = valueH * 2 + 2 },
        { VISIBILITY_GROUP = group, VISIBILITY_CONTEXT = "10 SHOW_WHEN 0" }
    )

    value("Volume VOLUME", { x = 0, y = 0, w = W2_4 - 2, h = valueH },
        { LABEL_COLOR = "tertiary", BAR_COLOR = TertiaryBar, ENCODER_ID = 0, TRACK = track, GROUP = group })
    value("DrumSample START", { x = 0, y = valueH + 1, w = W2_4 - 2, h = valueH },
        { LABEL_COLOR = "primary", BAR_COLOR = PrimaryBar, ENCODER_ID = 1, TRACK = track, GROUP = group })
    value("DrumSample BROWSER", { x = W2_4, y = 0, w = W2_4 - 2, h = valueH },
        { LABEL_COLOR = "secondary", BAR_COLOR = SecondaryBar, ENCODER_ID = 2, TRACK = track, GROUP = group, SHOW_LABEL = false, })
    value("DrumSample END", { x = W2_4, y = valueH + 1, w = W2_4 - 2, h = valueH },
        { LABEL_COLOR = "quaternary", BAR_COLOR = QuaternaryBar, ENCODER_ID = 3, TRACK = track, GROUP = group })

    visibility(
        "DrumSample" .. track,
        { x = 0, y = y, w = ScreenWidth, h = valueH * 2 + 2 },
        { VISIBILITY_GROUP = group, VISIBILITY_CONTEXT = "10 SHOW_WHEN_OVER 0" }
    )

    value("Sequencer STEP_VELOCITY", { x = 0, y = 0, w = W2_4 - 2, h = valueH },
        { LABEL_COLOR = "tertiary", BAR_COLOR = TertiaryBar, ENCODER_ID = 0, TRACK = track, GROUP = group })
    value("Sequencer STEP_CONDITION", { x = 0, y = valueH + 1, w = W2_4 - 2, h = valueH },
        { LABEL_COLOR = "primary", BAR_COLOR = PrimaryBar, ENCODER_ID = 1, TRACK = track, GROUP = group })
    value("Sequencer STEP_ENABLED", { x = W2_4, y = 0, w = W2_4 - 2, h = valueH },
        { LABEL_COLOR = "secondary", BAR_COLOR = SecondaryBar, ENCODER_ID = 2, TRACK = track, GROUP = group })
    -- value("Volume VOLUME", { x = W2_4, y = valueH + 1, w = W2_4 - 2, h = valueH },
    --     { LABEL_COLOR = "quaternary", BAR_COLOR = QuaternaryBar, ENCODER_ID = 3, TRACK = track, GROUP = group })
end

function values.master(group, track, y)
    visibility("DrumSampleMaster", { x = 0, y = y, w = ScreenWidth, h = valueH * 2 + 2 }, { VISIBILITY_GROUP = group })

    value("VolumeDrive VOLUME", { x = 0, y = 0, w = W2_4 - 2, h = valueH },
        { LABEL_COLOR = "tertiary", BAR_COLOR = TertiaryBar, ENCODER_ID = 0, TRACK = track, GROUP = group })
    value("VolumeDrive DRIVE", { x = 0, y = valueH + 1, w = W2_4 - 2, h = valueH },
        { LABEL_COLOR = "primary", BAR_COLOR = PrimaryBar, ENCODER_ID = 1, TRACK = track, GROUP = group })
    value("MMFilter CUTOFF", { x = W2_4, y = 0, w = W2_4 - 2, h = valueH },
        {
            LABEL_COLOR = "secondary",
            BAR_COLOR = SecondaryBar,
            USE_STRING_VALUE = true,
            SHOW_LABEL = false,
            ENCODER_ID = 2,
            TRACK = track,
            GROUP = group
        })
    value("MMFilter RESONANCE", { x = W2_4, y = valueH + 1, w = W2_4 - 2, h = valueH },
        { LABEL_COLOR = "quaternary", BAR_COLOR = QuaternaryBar, ENCODER_ID = 3, TRACK = track, GROUP = group })
end

return values
