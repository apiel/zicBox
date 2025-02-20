import { applyZic, buildPlateform, jsonStringify, ZicValue } from './core';

/**
 * Pre-load an audio plugin to the zic configuration and assign it an alias.
 *
 * @param name string - The alias name of the plugin to load.
 * @param pluginPath string - The path of the plugin to load.
 */
export function pluginAlias(name: string, pluginPath: string) {
    applyZic([
        {
            AUDIO_PLUGIN_ALIAS:
                name + ' @/plugins/audio/build/' + buildPlateform() + '/' + pluginPath,
        },
    ]);
}

/**
 * Assign an audio plugin to the zic configuration using a specified alias and additional values.
 *
 * @param pluginAlias string - The alias name of the plugin to load: "aliasName" or "name aliasName". If "name" is provided, it will be used as the name of the plugin in the UI, instead of "aliasName".
 * @param values table - An array of additional Zic values to apply to the plugin.
 */
export function pluginOld(pluginAlias: string, values: ZicValue[] = []) {
    // applyZic([{ AUDIO_PLUGIN: pluginAlias }, values]);
}

// export function audioPlugin(config: {
//     // name or path of the plugin
//     plugin: string;
//     // name alias of the plugin that will be used in the UI to interact with the plugin
//     aliasName?: string;
//     // additional values to apply to the plugin
//     [key: string]: unknown;
// }) {
//     if (!config.aliasName) {
//         if (config.plugin.endsWith('.so')) {
//             throw new Error('Missing alias name. When providing a plugin path, an alias name must be provided.');
//         }
//         config.aliasName = config.plugin;
//     }
//     applyZic([{ AUDIO_PLUGIN: jsonStringify(config) }]);
// }

export function audioPlugin({aliasName, plugin, ...config}: {
    // name or path of the plugin
    plugin: string;
    // name alias of the plugin that will be used in the UI to interact with the plugin
    aliasName?: string;
    // additional values to apply to the plugin
    [key: string]: unknown;
}) {
    if (!aliasName) {
        if (plugin.endsWith('.so')) {
            throw new Error('Missing alias name. When providing a plugin path, an alias name must be provided.');
        }
        aliasName = plugin;
    }
    applyZic([{ AUDIO_PLUGIN: jsonStringify({aliasName, plugin, ...config}) }, [ config as any ]]);
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
