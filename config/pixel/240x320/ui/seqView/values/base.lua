local ____lualib = require("lualib_bundle")
local __TS__ObjectAssign = ____lualib.__TS__ObjectAssign
local ____exports = {}
local ____constants = require("config.pixel.240x320.ui.constants")
local PrimaryBar = ____constants.PrimaryBar
local QuaternaryBar = ____constants.QuaternaryBar
local SecondaryBar = ____constants.SecondaryBar
local TertiaryBar = ____constants.TertiaryBar
local function base(props)
    return {__TS__ObjectAssign({SHOW_LABEL_OVER_VALUE = 6, LABEL_FONT_SIZE = 8}, props)}
end
function ____exports.primary(props)
    return base(__TS__ObjectAssign({LABEL_COLOR = "primary", BAR_COLOR = PrimaryBar}, props))
end
function ____exports.secondary(props)
    return base(__TS__ObjectAssign({LABEL_COLOR = "secondary", BAR_COLOR = SecondaryBar}, props))
end
function ____exports.tertiary(props)
    return base(__TS__ObjectAssign({LABEL_COLOR = "tertiary", BAR_COLOR = TertiaryBar}, props))
end
function ____exports.quaternary(props)
    return base(__TS__ObjectAssign({LABEL_COLOR = "quaternary", BAR_COLOR = QuaternaryBar}, props))
end
return ____exports
