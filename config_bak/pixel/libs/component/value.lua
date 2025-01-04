local ui = require "config/pixel/libs/ui"

--- Value component is used to display an audio plugin value.
local function component(value, position, options)
    ui.component("Value", {"VALUE"}, { VALUE = value }, position, options)
end

return component
