local visibility = require("config/pixel/libs/containers/visibility")
local value = require("config/pixel/libs/component/value")

-- local valueH = 11
local valueH = 25

local function topLeft() return { x = 0, y = 0, w = W2_4 - 2, h = valueH } end
local function bottomLeft() return { x = 0, y = valueH + 1, w = W2_4 - 2, h = valueH } end
local function topRight() return { x = W2_4, y = 0, w = W2_4 - 2, h = valueH } end
local function bottomRight() return { x = W2_4, y = valueH + 1, w = W2_4 - 2, h = valueH } end

local values = {}

local function base(options, group, track, encoderId)
    options.ENCODER_ID = encoderId
    options.TRACK = track
    options.GROUP = group
    return options
end

local function primary(options, group, track, encoderId)
    options.LABEL_COLOR = "primary"
    options.BAR_COLOR = PrimaryBar
    return base(options, group, track, encoderId)
end

local function secondary(options, group, track, encoderId)
    options.LABEL_COLOR = "secondary"
    options.BAR_COLOR = SecondaryBar
    return base(options, group, track, encoderId)
end

local function tertiary(options, group, track, encoderId)
    options.LABEL_COLOR = "tertiary"
    options.BAR_COLOR = TertiaryBar
    return base(options, group, track, encoderId)
end

local function quaternary(options, group, track, encoderId)
    options.LABEL_COLOR = "quaternary"
    options.BAR_COLOR = QuaternaryBar
    return base(options, group, track, encoderId)
end

function values.main(group, track, y)
    visibility(
        "DrumSample" .. track,
        { x = 0, y = y, w = ScreenWidth, h = valueH * 2 + 2 },
        { VISIBILITY_GROUP = group, VISIBILITY_CONTEXT = "10 SHOW_WHEN 0" }
    )

    value("Volume VOLUME", topLeft(), tertiary({}, group, track, 0))
    value("DrumSample START", bottomLeft(), primary({}, group, track, 1))
    value("DrumSample BROWSER", topRight(), secondary({ SHOW_LABEL = false, }, group, track, 2))
    value("DrumSample END", bottomRight(), quaternary({}, group, track, 3))

    visibility(
        "DrumSample" .. track,
        { x = 0, y = y, w = ScreenWidth, h = valueH * 2 + 2 },
        { VISIBILITY_GROUP = group, VISIBILITY_CONTEXT = "10 SHOW_WHEN_OVER 0" }
    )

    value("Sequencer STEP_VELOCITY", topLeft(), tertiary({}, group, track, 0))
    value("Sequencer STEP_CONDITION", bottomLeft(), primary({}, group, track, 1))
    value("Sequencer STEP_ENABLED", topRight(), secondary({}, group, track, 2))
    -- value("Volume VOLUME", bottomRight(), quaternary({}, group, track, 3))
end

function values.master(group, track, y)
    visibility("DrumSampleMaster", { x = 0, y = y, w = ScreenWidth, h = valueH * 2 + 2 }, { VISIBILITY_GROUP = group })

    value("VolumeDrive VOLUME", topLeft(), tertiary({}, group, track, 0))
    value("VolumeDrive DRIVE", bottomLeft(), primary({}, group, track, 1))
    value("MMFilter CUTOFF", topRight(), secondary({ USE_STRING_VALUE = true, SHOW_LABEL = false }, group, track, 2))
    value("MMFilter RESONANCE", bottomRight(), quaternary({}, group, track, 3))
end

return values
