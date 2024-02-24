# 06 Config plugins

By default ZicBox use [DustScript](https://github.com/apiel/dustscript) to set configuration. Configuration are set using a plugin, script language can be used, like Lua.

To load a different scripting language call zicBox like this:
```sh
./zicBox myconfig_file path/to/configLib.so
```

Lua extension will be recognized by default, so calling `./zicBox myconfig_file.lua` is enough, there is no need to specify the config plugin. If the extension is not recognized and no config plugin specified, it will by default use DustScript.

