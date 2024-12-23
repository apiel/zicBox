local ui = require "config/pixel/libs/ui"

--- Value component is used to display an audio plugin value.
local function component(params, position, options)
    ui.component("Value", {}, params, position, options)
end

return component
