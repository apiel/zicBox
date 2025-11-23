/** Description:
This configuration file serves as the blueprint for initializing an audio processing application, such as a real-time digital audio engine or a simplified music workstation. It defines the structure, file paths, and default settings needed for operation.

### Basic Idea of How the Code Works

The code establishes the essential audio signal flow, known as the "Master Track." This track is not the sound itself, but the series of virtual modules (or "plugins") through which all sound must pass. It uses dedicated functions to define this audio chain.

### Key Configuration Areas

1.  **The Master Audio Pipeline:** The core setup is the Master Track. It defines a sequential processing chain:
    *   **Sound Generation:** A plugin like `SynthLoop` generates the initial audio signal.
    *   **Processing and Control:** Modules are included for volume control, tempo synchronization, and managing signal levels.
    *   **Output:** A dedicated output plugin (`AudioOutput`) takes the processed signal and routes it to the computer’s sound card or speakers.
    *   **Saving:** A serialization plugin ensures the state of the track can be saved to a defined file path (`workspaceFolder`).

2.  **File Paths and Limits:** Several constants dictate file management:
    *   `workspaceFolder`: Specifies the directory where project data and active sessions are stored.
    *   `repositoriesFolder`: Defines the location for stored sound libraries, presets, and reusable configurations.
    *   `maxClip`: Sets a critical limit (1000), likely controlling the maximum permissible signal amplitude or volume level to prevent clipping or distortion.

3.  **System Initialization Settings:** The main `audio` object wraps up all settings. It configures how the system handles external hardware, like MIDI controllers, determining which devices should be automatically loaded upon startup.

sha: d6d3f0b3c19ee5f4758264db7a7c65ea80db64823f3dfc799b1d4da4d59138d9 
*/
import { audioPlugin } from '@/libs/audio';

export const maxClip = 1000;

const workspaceFolder = 'data/workspaces/grain';

const masterTrack = {
    id: 0,
    plugins: [
        audioPlugin('SynthLoop'),
        audioPlugin('EffectGainVolume', { alias: 'TrackFx' }),
        audioPlugin('AudioOutputAlsa_int16', { alias: 'AudioOutput' }),
        audioPlugin('SerializeTrack', { filename: 'master', maxClip, workspaceFolder }),
        audioPlugin('Tempo'),
    ],
};

export const repositoriesFolder = 'data/repositories';
export const defaultRepository = 'default_grain';

export const audio = {
    // midiInput: 'MPK mini',
    initActiveMidiTrack: 1,
    autoLoadFirstMidiDevice: true,
    // debugMidi: true, // Will print midi messages that are not used...
    tracks: [
        masterTrack,
    ],
    // autoSave: 500,
    repository: {
        folder: repositoriesFolder,
        default: defaultRepository,
    }
};
