import { applyZic, jsonStringify } from './core';

export function audioPlugin(config: {
    // name or path of the plugin
    plugin: string;
    // name alias of the plugin that will be used in the UI to interact with the plugin
    aliasName?: string;
    // additional values to apply to the plugin
    [key: string]: unknown;
}) {
    if (!config.aliasName) {
        if (config.plugin.endsWith('.so')) {
            throw new Error(
                'Missing alias name. When providing a plugin path, an alias name must be provided.'
            );
        }
        config.aliasName = config.plugin;
    }
    applyZic([{ AUDIO_PLUGIN: jsonStringify(config) }]);
}

// export function audioPlugin({aliasName, plugin, ...config}: {
//     // name or path of the plugin
//     plugin: string;
//     // name alias of the plugin that will be used in the UI to interact with the plugin
//     aliasName?: string;
//     // additional values to apply to the plugin
//     [key: string]: unknown;
// }) {
//     if (!aliasName) {
//         if (plugin.endsWith('.so')) {
//             throw new Error('Missing alias name. When providing a plugin path, an alias name must be provided.');
//         }
//         aliasName = plugin;
//     }
//     applyZic([{ AUDIO_PLUGIN: jsonStringify({aliasName, plugin, ...config}) }, [ config as any ]]);
// }

/**
 * Set the auto save mode.
 *
 * @param time number - The time between each auto save in ms.
 * This will be apply only to plugins that has been already loaded.
 */
export function autoSave(time: number) {
    applyZic([{ AUTO_SAVE: time }]);
}
