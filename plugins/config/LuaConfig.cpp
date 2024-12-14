#include "./LuaConfig.h"

extern "C" {
/*md
## LuaConfig

To set config from a lua script, you need to call `zic(key, value)` in the lua script:

```lua
zic("print", "Call ZicBox print from Lua.")
zic("PLUGIN_COMPONENT", "Encoder2 ./plugins/components/build/libzic_Encoder2Component.so")
zic("LOAD_HOST", "config.cfg")
zic("VIEW", "Main")

-- Let's create an encoder function
function setEncoder (x, y, width, height, encoder_id, value)
  zic("COMPONENT", "Encoder2" .. x .. " " .. y .. " " .. width .. " " .. height)
  zic("ENCODER_ID", encoder_id)
  zic("VALUE", value)
end

setEncoder(10, 10, 100, 100, 1, "FM DECAY_1")

-- ...
```

It might be necessary to install liblua5.4-dev:
```sh
sudo apt-get install liblua5.4-dev
```
*/
void config(std::string filename, void (*callback)(char* command, char* params, const char* filename), std::vector<Var> variables)
{
    luaConfig(filename, callback, variables);
}
}