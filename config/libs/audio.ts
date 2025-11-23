/** Description:
This TypeScript code serves as a structured configuration system for managing and building audio processing components, typically referred to as "plugins," within a larger software project.

**Core Functionality:**

1.  **Plugin Registration:** A function is provided to register each audio plugin. When a plugin is registered, the system creates a detailed configuration for it, including specific settings like how it responds to MIDI signals and assigning it a simplified nickname (an "alias"). If the plugin is referenced by its full file path, the system ensures an alias is mandatory for easier interaction. All successful registrations are stored in a central, unique master list.

2.  **Configuration Output:** The system includes a function to generate a specific construction blueprint file called a `Makefile`. This blueprint uses the collective list of all registered plugins to create instructions for the build process. Essentially, it tells the computer’s build environment exactly which audio components must be compiled and included in the final software application.

3.  **Data Structure:** The code defines how these components are organized into an application structure. An "Audio Track" is defined as a container that holds a unique identifier and an ordered list of fully configured plugins. This structure represents the sequence of effects and instruments applied to an audio signal.

In short, this code manages the identification, configuration, and documentation necessary to ensure that only the correct and required audio plugins are successfully integrated into a custom software build.

sha: 578efa8e55d32acbd0157741ae2a02fe1e86289602458190772a19c12ea3f617 
*/
import { writeFileSync } from 'fs';

const audioPluginList = new Set();

export function generateAudioMakefile(dest: string) {
    const content = `customBuild:
	make ${[...audioPluginList].join(' ')}
`;

    writeFileSync(dest, content);
}

export function audioPlugin(
    plugin: string,
    config: {
        // name alias of the plugin that will be used in the UI to interact with the plugin
        alias?: string;
        midiChannel?: number;
        midiCmd?: { parameter: string; cmd: string }[];
        // additional values to apply to the plugin
        [key: string]: unknown;
    } = {}
) {
    audioPluginList.add(plugin);
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
