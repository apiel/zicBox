/** Description:
This configuration file serves as the core blueprint for initializing a digital music studio environment. It precisely defines the structure, instruments, and effects chain that the audio application will load upon startup.

### How the System is Structured

The primary function of this setup is to organize the system into 11 distinct audio channels, or **Tracks**:

1.  **Drum Tracks (4):** Designed for rhythmic elements, using specialized sound generators like *SynthKick* (for kicks) and *SynthMultiDrum* (for other percussion).
2.  **Synthesizer Tracks (3):** Dedicated to melodic sounds, utilizing *SynthWavetable* generators.
3.  **Sample Tracks (3):** Intended for loading pre-recorded audio snippets using a *SynthMonoSample* generator, often including filters to shape the sound.
4.  **Master Track (1):** This controls the final, combined output of all other tracks, including global effects, mixing, and the actual sound output mechanism.

### The Role of Plugins

Each track is loaded with a sequential chain of **Plugins**. A plugin is essentially a virtual piece of equipment:

*   **Sound Sources:** These generate the sound (e.g., *SynthWavetable*).
*   **Sequencers:** These control the timing and rhythm, specifying a pattern length (here, typically 64 steps).
*   **Effects:** These modify the sound, such as applying volume adjustments (*EffectGainVolume*) or complex filtering (*EffectFilterMultiMode*).

### Configuration and Saving

The entire studio environment is configured to manage data efficiently. It specifies that up to 16 rhythm segments, known as clips, can be stored per track. Additionally, a *SerializeTrack* plugin on every channel ensures that the instrument settings, effects configuration, and performance data are automatically saved to a designated workspace folder, providing quick recall of the session.

sha: 6f3e3e14d56afac89a73b8f4cabd413bc84bf33fc8dff2422bb544a38a174d22 
*/
import { audioPlugin } from '@/libs/audio';
import {
    Drum1Track,
    Drum2Track,
    Drum3Track,
    Drum4Track,
    Sample1Track,
    Sample2Track,
    Sample3Track,
    Synth1Track,
    Synth2Track,
    Synth3Track,
} from '../constants';

export const maxClip = 16;

const workspaceFolder = 'data/workspaces/grid';

const drum1Track = {
    id: Drum1Track,
    plugins: [
        audioPlugin('SynthKick23', { alias: 'Drum1' }),
        audioPlugin('Sequencer', { defaultStepCount: 64 }),
        // audioPlugin('EffectScatter', { alias: `Scatter${Drum1Track}` }),
        audioPlugin('EffectGainVolume', { alias: 'TrackFx' }),
        audioPlugin('SerializeTrack', { filename: 'drum1', maxClip, workspaceFolder }),
    ],
};

// SynthMultiDrum

const drum2Track = {
    id: Drum2Track,
    plugins: [
        audioPlugin('SynthMultiDrum', { alias: 'Drum2' }),
        audioPlugin('Sequencer', { defaultStepCount: 64 }),
        // audioPlugin('EffectScatter', { alias: `Scatter${Drum2Track}` }),
        audioPlugin('EffectGainVolume', { alias: 'TrackFx' }),
        audioPlugin('SerializeTrack', { filename: 'drum2', maxClip, workspaceFolder }),
    ],
};

const drum3Track = {
    id: Drum3Track,
    plugins: [
        audioPlugin('SynthMultiDrum', { alias: 'Drum3' }),
        audioPlugin('Sequencer', { defaultStepCount: 64 }),
        // audioPlugin('EffectScatter', { alias: `Scatter${Drum3Track}` }),
        audioPlugin('EffectGainVolume', { alias: 'TrackFx' }),
        audioPlugin('SerializeTrack', { filename: 'drum3', maxClip, workspaceFolder }),
    ],
};

const drum4Track = {
    id: Drum4Track,
    plugins: [
        audioPlugin('SynthMultiDrum', { alias: 'Drum4' }),
        audioPlugin('Sequencer', { defaultStepCount: 64 }),
        // audioPlugin('EffectScatter', { alias: `Scatter${Drum4Track}` }),
        audioPlugin('EffectGainVolume', { alias: 'TrackFx' }),
        audioPlugin('SerializeTrack', { filename: 'drum4', maxClip, workspaceFolder }),
    ],
};

const synth1Track = {
    id: Synth1Track,
    plugins: [
        audioPlugin('SynthWavetable', { alias: 'Synth1' }),
        audioPlugin('Sequencer', { defaultStepCount: 64 }),
        // audioPlugin('EffectScatter', { alias: `Scatter${Synth1Track}` }),
        audioPlugin('EffectVolumeMultiFx', { alias: 'TrackFx' }),
        audioPlugin('EffectVolumeMultiFx', { alias: 'TrackFx2' }),
        audioPlugin('SerializeTrack', { filename: 'synth1', maxClip, workspaceFolder }),
    ],
};

const synth2Track = {
    id: Synth2Track,
    plugins: [
        audioPlugin('SynthWavetable', { alias: 'Synth2' }),
        audioPlugin('Sequencer', { defaultStepCount: 64 }),
        // audioPlugin('EffectScatter', { alias: `Scatter${Synth2Track}` }),
        audioPlugin('EffectVolumeMultiFx', { alias: 'TrackFx' }),
        audioPlugin('EffectVolumeMultiFx', { alias: 'TrackFx2' }),
        audioPlugin('SerializeTrack', { filename: 'synth2', maxClip, workspaceFolder }),
    ],
};

const synth3Track = {
    id: Synth3Track,
    plugins: [
        audioPlugin('SynthWavetable', { alias: 'Synth3' }),
        audioPlugin('Sequencer', { defaultStepCount: 64 }),
        // audioPlugin('EffectScatter', { alias: `Scatter${Synth3Track}` }),
        audioPlugin('EffectVolumeMultiFx', { alias: 'TrackFx' }),
        audioPlugin('EffectVolumeMultiFx', { alias: 'TrackFx2' }),
        audioPlugin('SerializeTrack', { filename: 'synth3', maxClip, workspaceFolder }),
    ],
};

const sample1Track = {
    id: Sample1Track,
    plugins: [
        audioPlugin('SynthMonoSample', { alias: 'Sample1' }),
        audioPlugin('Sequencer', { defaultStepCount: 64 }),
        // audioPlugin('EffectScatter', { alias: `Scatter${Sample1Track}` }),
        audioPlugin('EffectFilterMultiMode', { alias: 'Filter' }),
        audioPlugin('EffectVolumeMultiFx', { alias: 'TrackFx' }),
        audioPlugin('EffectVolumeMultiFx', { alias: 'TrackFx2' }),
        audioPlugin('SerializeTrack', { filename: 'sample1', maxClip, workspaceFolder }),
    ],
};

const sample2Track = {
    id: Sample2Track,
    plugins: [
        audioPlugin('SynthMonoSample', { alias: 'Sample2' }),
        audioPlugin('Sequencer', { defaultStepCount: 64 }),
        // audioPlugin('EffectScatter', { alias: `Scatter${Sample2Track}` }),
        audioPlugin('EffectFilterMultiMode', { alias: 'Filter' }),
        audioPlugin('EffectVolumeMultiFx', { alias: 'TrackFx' }),
        audioPlugin('EffectVolumeMultiFx', { alias: 'TrackFx2' }),
        audioPlugin('SerializeTrack', { filename: 'sample2', maxClip, workspaceFolder }),
    ],
};

const sample3Track = {
    id: Sample3Track,
    plugins: [
        audioPlugin('SynthMonoSample', { alias: 'Sample3' }),
        audioPlugin('Sequencer', { defaultStepCount: 64 }),
        // audioPlugin('EffectScatter', { alias: `Scatter${Sample3Track}` }),
        audioPlugin('EffectFilterMultiMode', { alias: 'Filter' }),
        audioPlugin('EffectVolumeMultiFx', { alias: 'TrackFx' }),
        audioPlugin('EffectVolumeMultiFx', { alias: 'TrackFx2' }),
        audioPlugin('SerializeTrack', { filename: 'sample3', maxClip, workspaceFolder }),
    ],
}

const masterTrack = {
    id: 0,
    plugins: [
        audioPlugin('Mixer10', { alias: 'Mixer' }),
        audioPlugin('EffectScatter', { alias: 'Scatter0' }),
        audioPlugin('EffectFilteredMultiFx', { alias: 'FilteredFx' }),
        audioPlugin('EffectBandIsolatorFx', { alias: 'IsolatorFx' }),
        audioPlugin('EffectVolumeMultiFx', { alias: 'TrackFx' }),
        audioPlugin('AudioOutputPulse', { alias: 'AudioOutput' }),
        // audioPlugin('AudioOutputAlsa', { alias: 'AudioOutput' }),
        audioPlugin('SerializeTrack', { filename: 'master', maxClip, workspaceFolder }),
        audioPlugin('Tempo'),
    ],
};

export const audio = {
    // midiInput: 'Arduino Leonardo',
    initActiveMidiTrack: 1,
    autoLoadFirstMidiDevice: true,
    tracks: [
        drum1Track,
        drum2Track,
        drum3Track,
        drum4Track,
        synth1Track,
        synth2Track,
        synth3Track,
        sample1Track,
        sample2Track,
        sample3Track,
        masterTrack,
    ],
    autoSave: 500,
};
