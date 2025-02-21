export function audioPlugin(plugin: string, config: {
    // name alias of the plugin that will be used in the UI to interact with the plugin
    alias?: string;
    midiChannel?: number;
    midiCmd?: { parameter: string; cmd: string }[];
    // additional values to apply to the plugin
    [key: string]: unknown;
} = {}) {
    config.plugin = plugin;
    if (!config.alias) {
        if (plugin.endsWith('.so')) {
            throw new Error(
                'Missing alias name. When providing a plugin path, an alias name must be provided.'
            );
        }
        config.alias = plugin;
    }
    return config;
}

export interface Track {
    id: number;
    plugins: ReturnType<typeof audioPlugin>[];
}