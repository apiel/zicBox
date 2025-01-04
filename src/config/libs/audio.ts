import { applyZic, buildPlateform, ZicValue } from './core';

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
export function plugin(pluginAlias: string, values: ZicValue[] = []) {
    applyZic([{ AUDIO_PLUGIN: pluginAlias }, values]);
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
