/** Description:
This configuration file serves as the detailed blueprint for an application designed to create and manage music, similar to a digital mixing studio. It structures the entire audio environment by defining various channels, known as "tracks," and assigning specific functional modules, called "plugins," to each one.

### Core Functionality

The code establishes nine primary audio channels: six standard instrument tracks, two specialized sample tracks, and one master track.

1.  **Instrument Tracks (Track 1-6):** These tracks are set up to use a 'Synth Multi-Engine' plugin, which is responsible for generating complex musical sounds (synthesis). They also include a 'Sequencer' for managing rhythm and patterns, and specialized storage plugins that handle saving musical data (clips) to the workspace folder.
2.  **Sample Tracks:** These tracks substitute the standard synthesizer with a 'Multi-Sample' plugin, meaning they are designed specifically for playing back and manipulating pre-recorded audio snippets (samples).
3.  **Master Track:** This crucial track combines the output from all other channels. It uses a 'Mixer' plugin, applies overall volume adjustments and effects, and directs the final, mixed audio stream to your computer's audio output device.

### Data Management

The configuration also handles project data. It sets a specific path for all project files (the 'desktop workspace folder') and ensures that every track has the necessary tools to save its musical clips efficiently, defining parameters like the maximum number of clips allowed and how track data is serialized (prepared for storage).

In summary, this file defines the comprehensive structure of the music application, ensuring every track has the correct tools—from sound generation and sequencing to mixing, file saving, and final audio output.

Tags: Music Production Setup, Sound Generation, Music Arrangement, Mixing Console, Track Configuration
sha: 2273c10eb52546d218771836cd63458262694820590ebad19ae06dcdad3703b6 
*/
import { audioPlugin } from '@/libs/audio';
import {
    Sample1Track,
    Sample2Track,
    Track1,
    Track2,
    Track3,
    Track4,
    Track5,
    Track6,
} from '../constants';

export const workspaceFolder = 'data/workspaces/desktop';

const preset = true;

export const getClipFolder = (trackId: number) => `track${trackId}`;
export const getSynthAlias = (trackId: number) => `Track${trackId}`;

const serializeTrackParams = {
    workspaceFolder,
    maxClip: 1000,
    loadInMemory: true,
    saveBeforeChangingClip: true,
}

const timelineMode = true;

const track1Track = {
    id: Track1,
    plugins: [
        audioPlugin('SynthMultiEngine', { alias: getSynthAlias(Track1), preset }),
        audioPlugin('Sequencer', { defaultStepCount: 64, timelineMode }),
        audioPlugin('SerializeTrack', { clipFolder: getClipFolder(Track1), ...serializeTrackParams }),
        audioPlugin('TimelineSequencer', { workspaceFolder, timelineFilename: `${getClipFolder(Track1)}/timeline.json`, target: 'SerializeTrack' }),
    ],
};

const track2Track = {
    id: Track2,
    plugins: [
        audioPlugin('SynthMultiEngine', { alias: getSynthAlias(Track2), preset }),
        audioPlugin('Sequencer', { defaultStepCount: 64, timelineMode }),
        audioPlugin('SerializeTrack', { clipFolder: getClipFolder(Track2), ...serializeTrackParams }),
        audioPlugin('TimelineSequencer', { workspaceFolder, timelineFilename: `${getClipFolder(Track2)}/timeline.json`, target: 'SerializeTrack' }),
    ],
};

const track3Track = {
    id: Track3,
    plugins: [
        audioPlugin('SynthMultiEngine', { alias: getSynthAlias(Track3), preset }),
        audioPlugin('Sequencer', { defaultStepCount: 64, timelineMode }),
        audioPlugin('SerializeTrack', { clipFolder: getClipFolder(Track3), ...serializeTrackParams }),
        audioPlugin('TimelineSequencer', { workspaceFolder, timelineFilename: `${getClipFolder(Track3)}/timeline.json`, target: 'SerializeTrack' }),
    ],
};

const track4Track = {
    id: Track4,
    plugins: [
        audioPlugin('SynthMultiEngine', { alias: getSynthAlias(Track4), preset }),
        audioPlugin('Sequencer', { defaultStepCount: 64, timelineMode }),
        audioPlugin('SerializeTrack', { clipFolder: getClipFolder(Track4), ...serializeTrackParams }),
        audioPlugin('TimelineSequencer', { workspaceFolder, timelineFilename: `${getClipFolder(Track4)}/timeline.json`, target: 'SerializeTrack' }),
    ],
};

const track5Track = {
    id: Track5,
    plugins: [
        audioPlugin('SynthMultiEngine', { alias: getSynthAlias(Track5), preset }),
        audioPlugin('Sequencer', { defaultStepCount: 64, timelineMode }),
        audioPlugin('SerializeTrack', { clipFolder: getClipFolder(Track5), ...serializeTrackParams }),
        audioPlugin('TimelineSequencer', { workspaceFolder, timelineFilename: `${getClipFolder(Track5)}/timeline.json`, target: 'SerializeTrack' }),
    ],
};

const track6Track = {
    id: Track6,
    plugins: [
        audioPlugin('SynthMultiEngine', { alias: getSynthAlias(Track6), preset }),
        audioPlugin('Sequencer', { defaultStepCount: 64, timelineMode }),
        audioPlugin('SerializeTrack', { clipFolder: getClipFolder(Track6), ...serializeTrackParams }),
        audioPlugin('TimelineSequencer', { workspaceFolder, timelineFilename: `${getClipFolder(Track6)}/timeline.json`, target: 'SerializeTrack' }),
    ],
};

const sample1Track = {
    id: Sample1Track,
    plugins: [
        audioPlugin('SynthMultiSample', { alias: 'Sample1', preset }),
        audioPlugin('Sequencer', { defaultStepCount: 64, timelineMode }),
        audioPlugin('SerializeTrack', { clipFolder: 'sample1', ...serializeTrackParams }),
    ],
};

const sample2Track = {
    id: Sample2Track,
    plugins: [
        audioPlugin('SynthMultiSample', { alias: 'Sample2', preset }),
        audioPlugin('Sequencer', { defaultStepCount: 64, timelineMode }),
        audioPlugin('SerializeTrack', { clipFolder: 'sample2', ...serializeTrackParams }),
    ],
};

const masterTrack = {
    id: 0,
    plugins: [
        audioPlugin('Mixer8', { alias: 'Mixer' }),
        audioPlugin('EffectGainVolume', { alias: 'TrackFx' }),
        // audioPlugin('AudioOutputPulse', { alias: 'AudioOutput' }),
        // audioPlugin('AudioOutputAlsa', { alias: 'AudioOutput' }),
        audioPlugin('AudioOutputAlsa_int16', { alias: 'AudioOutput' }),
        audioPlugin('SerializeTrack', { clipFolder: 'master', maxClip: serializeTrackParams.maxClip, workspaceFolder }),
        audioPlugin('Tempo'),
    ],
};

export const repositoriesFolder = 'data/repositories';
export const defaultRepository = 'default_pixel';

export const audio = {
    // midiInput: 'MPK mini',
    initActiveMidiTrack: 1,
    autoLoadFirstMidiDevice: true,
    // debugMidi: true, // Will print midi messages that are not used...
    tracks: [
        track1Track,
        track2Track,
        track3Track,
        track4Track,
        track5Track,
        track6Track,
        sample1Track,
        sample2Track,
        masterTrack,
    ],
    // autoSave: 500,
    repository: {
        folder: repositoriesFolder,
        default: defaultRepository,
    },
};
