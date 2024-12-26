local visibility = require("config/pixel/libs/containers/visibility")
local value = require("config/pixel/libs/component/value")
local encoder3 = require("config/pixel/libs/component/encoder3")
local graph = require("config/pixel/libs/component/graph")
local drumEnvelop = require("config/pixel/libs/component/drumEnvelop")

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
    options.SHOW_LABEL_OVER_VALUE = true
    -- options.LABEL_FONT_SIZE = 8
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

local function container(group, y, options)
    options.VISIBILITY_GROUP = group
    visibility({ x = 0, y = y, w = ScreenWidth, h = valueH * 2 + 2 }, options)
end


local function drum23Values(group, track, y, contextIndex)
    container(group, y, { VISIBILITY_CONTEXT = contextIndex .. " SHOW_WHEN 1" })

    value("Distortion WAVESHAPE", topLeft(), tertiary({}, group, track, 0))
    value("Distortion DRIVE", bottomLeft(), primary({}, group, track, 1))
    value("Distortion COMPRESS", topRight(), secondary({}, group, track, 2))
    value("Distortion BASS", bottomRight(), quaternary({}, group, track, 3))

    container(group, y, { VISIBILITY_CONTEXT = contextIndex .. " SHOW_WHEN 2" })
    -- graph(
    --     { PLUGIN = "Drum23", DATA_ID = "WAVEFORM" },
    --     { x = 0, y = 0, w = W3_4 - 2, h = 50 },
    --     { GROUP = group, TRACK = track, RENDER_TITLE_ON_TOP = false, ENCODERS = { "0 WAVEFORM_TYPE", "2 MACRO", "1 SHAPE" } }
    -- )
    -- encoder3(
    --     { ENCODER_ID = 3, VALUE = "Drum23 GAIN_CLIPPING" },
    --     { x = W3_4, y = 0, w = W1_4, h = 50 },
    --     { GROUP = group, COLOR = "quaternary", TRACK = track }
    -- )
    graph(
        { PLUGIN = "Drum23", DATA_ID = "WAVEFORM" },
        { x = 0, y = 0, w = ScreenWidth, h = 50 },
        { GROUP = group, TRACK = track, RENDER_TITLE_ON_TOP = false, ENCODERS = { "0 WAVEFORM_TYPE", "2 MACRO", "1 SHAPE" } }
    )

    container(group, y, { VISIBILITY_CONTEXT = contextIndex .. " SHOW_WHEN 3" })
    drumEnvelop(
        { PLUGIN = "Drum23", ENVELOP_DATA_ID = "0" },
        { x = 0, y = 0, w = W3_4 - 2, h = 50 },
        { GROUP = group, TRACK = track, RENDER_TITLE_ON_TOP = false, ENCODERS = { "0 TIME", "1 PHASE", "2 MODULATION", } }
    )
    encoder3(
        { ENCODER_ID = 3, VALUE = "Drum23 DURATION" },
        { x = W3_4, y = 0, w = W1_4, h = 50 },
        { GROUP = group, COLOR = "quaternary", TRACK = track }
    )

    container(group, y, { VISIBILITY_CONTEXT = contextIndex .. " SHOW_WHEN 4" })
    drumEnvelop(
        { PLUGIN = "Drum23", ENVELOP_DATA_ID = "4" },
        { x = 0, y = 0, w = W3_4 - 2, h = 50 },
        { GROUP = group, TRACK = track, RENDER_TITLE_ON_TOP = false, ENCODERS = { "0 TIME", "1 PHASE", "2 MODULATION" } }
    )
    encoder3(
        { ENCODER_ID = 3, VALUE = "Drum23 PITCH", },
        { x = W3_4, y = 0, w = W1_4, h = 50 },
        { GROUP = group, COLOR = "secondary", TRACK = track }
    )

    container(group, y, { VISIBILITY_CONTEXT = contextIndex .. " SHOW_WHEN 5" })
    value("Drum23 CLICK", topLeft(), tertiary({}, group, track, 0))
    value("Drum23 CLICK_DURATION", bottomLeft(), primary({}, group, track, 1))
    value("Drum23 CLICK_CUTOFF", topRight(), secondary({ USE_STRING_VALUE = true }, group, track, 2))
    value("Drum23 CLICK_RESONANCE", bottomRight(), quaternary({}, group, track, 3))
end

local function sampleValues(group, track, y, contextIndex)
    container(group, y, { VISIBILITY_CONTEXT = contextIndex .. " SHOW_WHEN 1" })

    value("DrumSample BROWSER", topLeft(), tertiary({}, group, track, 0))
    value("DrumSample START", bottomLeft(), primary({}, group, track, 1))
    -- value("DrumSample BROWSER", topRight(), secondary({}, group, track, 2))
    value("DrumSample END", bottomRight(), quaternary({}, group, track, 3))
end


local function seq(group, track, y)
    container(group, y, { VISIBILITY_CONTEXT = "10 SHOW_WHEN_OVER 0" })

    value("Sequencer STEP_NOTE", topLeft(), tertiary({}, group, track, 0))
    value("Sequencer STEP_CONDITION", bottomLeft(), primary({}, group, track, 1))
    value("Sequencer STEP_VELOCITY", topRight(), secondary({}, group, track, 2))
    value("Sequencer STEP_MOTION", bottomRight(), quaternary({}, group, track, 3))
end

function values.drumSample(group, track, y, contextIndex)
    container(group, y, { VISIBILITY_CONTEXT = { "10 SHOW_WHEN 0", contextIndex .. " SHOW_WHEN 0" } })

    value("Volume VOLUME", topLeft(), tertiary({}, group, track, 0))
    value("Volume GAIN_CLIPPING", bottomLeft(), primary({}, group, track, 1))
    value("Volume DRIVE", topRight(), secondary({}, group, track, 2))
    value("SampleRateReducer SAMPLE_STEP", bottomRight(), quaternary({}, group, track, 3))

    seq(group, track, y)

    sampleValues(group, track, y, contextIndex)
end

function values.drum23(group, track, y, contextIndex)
    container(group, y, { VISIBILITY_CONTEXT = { "10 SHOW_WHEN 0", contextIndex .. " SHOW_WHEN 0" } })

    value("Volume VOLUME", topLeft(), tertiary({}, group, track, 0))
    --> might want to put something else, no need of distortion level, could just be always at 100%
    -- value("Distortion LEVEL", bottomLeft(), primary({}, group, track, 1))
    value("Drum23 GAIN_CLIPPING", bottomLeft(), primary({}, group, track, 1))
    value("MMFilter CUTOFF", topRight(), quaternary({ USE_STRING_VALUE = true, LABEL = "Filter" }, group, track, 2))
    value("MMFilter RESONANCE", bottomRight(), quaternary({}, group, track, 3))

    seq(group, track, y)

    drum23Values(group, track, y, contextIndex)
end

function values.master(group, track, y)
    container(group, y, {})

    -- might not even want to put volume here can be in sub-page
    --> rather put step reducer and another effect
    value("MasterVolume VOLUME", topLeft(), tertiary({}, group, track, 0))
    -- value("VolumeDrive DRIVE", bottomLeft(), primary({}, group, track, 1))
    value("MasterFilter CUTOFF", topRight(), quaternary({ USE_STRING_VALUE = true, LABEL = "Filter" }, group, track, 2))
    value("MasterFilter RESONANCE", bottomRight(), quaternary({}, group, track, 3))
end

return values
