--[[ Generated with https://github.com/TypeScriptToLua/TypeScriptToLua ]]
local ____exports = {}
local React = require("config.libs.react")
local ____Keymap = require("config.libs.components.Keymap")
local Keymap = ____Keymap.Keymap
local ____Keymaps = require("config.libs.components.Keymaps")
local Keymaps = ____Keymaps.Keymaps
local ____TextGrid = require("config.libs.components.TextGrid")
local TextGrid = ____TextGrid.TextGrid
local ____View = require("config.libs.components.View")
local View = ____View.View
local ____VisibilityContext = require("config.libs.components.VisibilityContext")
local VisibilityContext = ____VisibilityContext.VisibilityContext
local ____Workspaces = require("config.libs.components.Workspaces")
local Workspaces = ____Workspaces.Workspaces
local ____constants = require("config.pixel.240x320_rpi3A_4enc_11btn.ui.constants")
local btn2 = ____constants.btn2
local btn5 = ____constants.btn5
local btn6 = ____constants.btn6
local btn7 = ____constants.btn7
local KeyInfoPosition = ____constants.KeyInfoPosition
local ScreenWidth = ____constants.ScreenWidth
function ____exports.WorkspacesView(____bindingPattern0)
    local name
    name = ____bindingPattern0.name
    return React.createElement(
        View,
        {name = name},
        React.createElement(
            Workspaces,
            {plugin = "SerializeTrack", bounds = {0, 0, ScreenWidth, 280}},
            React.createElement(Keymaps, {keys = {{key = btn5, action = ".data:LOAD_WORKSPACE", context = "254:0"}, {key = btn2, action = ".up", context = "254:0"}, {key = btn6, action = ".down", context = "254:0"}, {key = btn7, action = ".delete", context = "254:1"}}})
        ),
        React.createElement(
            TextGrid,
            {bounds = KeyInfoPosition, rows = {"&empty &icon::arrowUp::filled ...", "Use &icon::arrowDown::filled Exit"}},
            React.createElement(VisibilityContext, {index = 254, condition = "SHOW_WHEN", value = 0}),
            React.createElement(Keymap, {key = "d", action = "setView:Clips"}),
            React.createElement(Keymap, {key = "e", action = "contextToggle:254:1:0"})
        ),
        React.createElement(
            TextGrid,
            {bounds = KeyInfoPosition, rows = {"New &empty ^...", "&empty &empty &icon::trash"}},
            React.createElement(VisibilityContext, {index = 254, condition = "SHOW_WHEN", value = 1}),
            React.createElement(Keymap, {key = "q", action = "setView:CreateWorkspace", action2 = "contextToggle:254:1:0"}),
            React.createElement(Keymap, {key = "e", action = "contextToggle:254:1:0"})
        )
    )
end
return ____exports
