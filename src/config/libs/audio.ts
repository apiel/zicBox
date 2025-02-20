import { applyZic, jsonStringify } from './core';

export function audioPlugin(plugin: string, config: {
    // name alias of the plugin that will be used in the UI to interact with the plugin
    aliasName?: string;
    // additional values to apply to the plugin
    [key: string]: unknown;
} = {}) {
    config.plugin = plugin;
    if (!config.aliasName) {
        if (plugin.endsWith('.so')) {
            throw new Error(
                'Missing alias name. When providing a plugin path, an alias name must be provided.'
            );
        }
        config.aliasName = plugin;
    }
    applyZic([{ AUDIO_PLUGIN: jsonStringify(config) }]);
}

/**
 * Set the auto save mode.
 *
 * @param time number - The time between each auto save in ms.
 * This will be apply only to plugins that has been already loaded.
 */
export function autoSave(time: number) {
    applyZic([{ AUTO_SAVE: time }]);
}
