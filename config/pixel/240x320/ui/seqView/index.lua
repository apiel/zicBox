local ____lualib = require("lualib_bundle")
local __TS__ObjectAssign = ____lualib.__TS__ObjectAssign
local __TS__ObjectRest = ____lualib.__TS__ObjectRest
local ____exports = {}
local React = require("config.libs.react")
local ____Keymap = require("config.libs.components.Keymap")
local Keymap = ____Keymap.Keymap
local ____TextGrid = require("config.libs.components.TextGrid")
local TextGrid = ____TextGrid.TextGrid
local ____View = require("config.libs.components.View")
local View = ____View.View
local ____VisibilityContext = require("config.libs.components.VisibilityContext")
local VisibilityContext = ____VisibilityContext.VisibilityContext
local ____VisibilityGroup = require("config.libs.components.VisibilityGroup")
local VisibilityGroup = ____VisibilityGroup.VisibilityGroup
local ____constants = require("config.pixel.240x320.ui.constants")
local KeyInfoPosition = ____constants.KeyInfoPosition
function ____exports.SeqView(____bindingPattern0)
    local props
    local name
    name = ____bindingPattern0.name
    props = __TS__ObjectRest(____bindingPattern0, {name = true})
    return React.createElement(
        View,
        __TS__ObjectAssign({name = name}, props),
        React.createElement(
            TextGrid,
            {position = KeyInfoPosition, rows = {"&icon::play::filled &icon::arrowUp::filled ...", "&icon::arrowLeft::filled &icon::arrowDown::filled &icon::arrowRight::filled"}},
            React.createElement(VisibilityGroup, {condition = "SHOW_WHEN", group = 0}),
            React.createElement(VisibilityContext, {index = 254, condition = "SHOW_WHEN", value = 0}),
            React.createElement(Keymap, {key = "q", action = "playPause"}),
            React.createElement(Keymap, {key = "e", action = "contextToggle:254:1:0"})
        ),
        React.createElement(
            TextGrid,
            {position = KeyInfoPosition, rows = {"Menu &icon::play::filled ^...", "Clips ? Save"}},
            React.createElement(VisibilityGroup, {condition = "SHOW_WHEN", group = 0}),
            React.createElement(VisibilityContext, {index = 254, condition = "SHOW_WHEN", value = 1}),
            React.createElement(Keymap, {key = "q", action = "setView:Menu", action2 = "contextToggle:254:1:0"}),
            React.createElement(Keymap, {key = "w", action = "playPause"}),
            React.createElement(Keymap, {key = "a", action = "setView:Clips"}),
            React.createElement(Keymap, {key = "e", action = "contextToggle:254:1:0"})
        )
    )
end
return ____exports
