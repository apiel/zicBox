local ____lualib = require("lualib_bundle")
local __TS__ObjectRest = ____lualib.__TS__ObjectRest
local ____exports = {}
local ____core = require("config.pixel.ts.libs.core")
local applyZic = ____core.applyZic
local React = require("config.pixel.ts.libs.react")
local ____ui = require("config.pixel.ts.libs.ui")
local getView = ____ui.getView
local function View(self, ____bindingPattern0)
    local props
    local name
    name = ____bindingPattern0.name
    props = __TS__ObjectRest(____bindingPattern0, {name = true})
    return getView(nil, name, {props})
end
applyZic(React:createElement(View, {name = "test"}))
return ____exports
