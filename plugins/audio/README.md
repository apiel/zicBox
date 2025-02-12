# 01 Audio plugins

ZicHost take care to load and handle audio plugins. Each plugins have access to the 32 audio tracks buffer to manipulate them. Those plugins are called in loop in the same order as they have been instanciated. A plugin can be loaded multiple times under a different name, see `AUDIO_PLUGIN` configuration in previous section.
