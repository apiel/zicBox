/** Description:
This configuration file acts as the comprehensive blueprint for setting up a digital audio environment, much like a detailed instruction manual for assembling a virtual recording studio.

The core of the system is built around defined "tracks," which function as separate channels on a virtual mixing board. This setup includes six standard synthesizer channels (Tracks 1 through 6), two specialized sampling channels (Sample 1 and Sample 2), and a crucial "Master Track" where all sounds are combined for final output.

Each track is loaded with a specific chain of "plugins," which are digital components that perform specific functions, similar to physical effects pedals or specialized instruments.

Key plugin roles include:

1.  **Sound Generation:** Plugins like "SynthMultiEngine" are responsible for creating new sounds, while "SynthMultiSample" plays back pre-recorded audio.
2.  **Timing and Arrangement:** The "Sequencer" controls rhythm and timing, and the "ClipSequencer" manages how segments of music (clips) are arranged over a timeline.
3.  **Data Management:** "SerializeTrack" handles saving sound clips and project data, ensuring everything is stored correctly in a specified "workspace folder."

The Master Track contains crucial components, including a "Mixer" to balance the levels of the other tracks and an "AudioOutput" plugin to route the final sound signal to the computer's speakers.

In summary, this file dictates the initial structure of the entire audio project: which instruments are active, how they are connected, how they generate sounds, and where all the associated performance data is saved.

sha: 8e6ed3eec686e195234ba7eb8eb7fc0cd1f50c6f2a016e2054dcfbfeff030926 
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

const track1Track = {
    id: Track1,
    plugins: [
        audioPlugin('SynthMultiEngine', { alias: getSynthAlias(Track1), preset }),
        audioPlugin('Sequencer', { defaultStepCount: 64 }),
        audioPlugin('SerializeTrack', { clipFolder: getClipFolder(Track1), ...serializeTrackParams }),
        audioPlugin('ClipSequencer', { workspaceFolder, timelineFilename: `${getClipFolder(Track1)}/timeline.json`, target: 'SerializeTrack' }),
    ],
};

const track2Track = {
    id: Track2,
    plugins: [
        audioPlugin('SynthMultiEngine', { alias: getSynthAlias(Track2), preset }),
        audioPlugin('Sequencer', { defaultStepCount: 64 }),
        audioPlugin('SerializeTrack', { clipFolder: getClipFolder(Track2), ...serializeTrackParams }),
        audioPlugin('ClipSequencer', { workspaceFolder, timelineFilename: `${getClipFolder(Track2)}/timeline.json`, target: 'SerializeTrack' }),
    ],
};

const track3Track = {
    id: Track3,
    plugins: [
        audioPlugin('SynthMultiEngine', { alias: getSynthAlias(Track3), preset }),
        audioPlugin('Sequencer', { defaultStepCount: 64 }),
        audioPlugin('SerializeTrack', { clipFolder: getClipFolder(Track3), ...serializeTrackParams }),
        audioPlugin('ClipSequencer', { workspaceFolder, timelineFilename: `${getClipFolder(Track3)}/timeline.json`, target: 'SerializeTrack' }),
    ],
};

const track4Track = {
    id: Track4,
    plugins: [
        audioPlugin('SynthMultiEngine', { alias: getSynthAlias(Track4), preset }),
        audioPlugin('Sequencer', { defaultStepCount: 64 }),
        audioPlugin('SerializeTrack', { clipFolder: getClipFolder(Track4), ...serializeTrackParams }),
        audioPlugin('ClipSequencer', { workspaceFolder, timelineFilename: `${getClipFolder(Track4)}/timeline.json`, target: 'SerializeTrack' }),
    ],
};

const track5Track = {
    id: Track5,
    plugins: [
        audioPlugin('SynthMultiEngine', { alias: getSynthAlias(Track5), preset }),
        audioPlugin('Sequencer', { defaultStepCount: 64 }),
        audioPlugin('SerializeTrack', { clipFolder: getClipFolder(Track5), ...serializeTrackParams }),
        audioPlugin('ClipSequencer', { workspaceFolder, timelineFilename: `${getClipFolder(Track5)}/timeline.json`, target: 'SerializeTrack' }),
    ],
};

const track6Track = {
    id: Track6,
    plugins: [
        audioPlugin('SynthMultiEngine', { alias: getSynthAlias(Track6), preset }),
        audioPlugin('Sequencer', { defaultStepCount: 64 }),
        audioPlugin('SerializeTrack', { clipFolder: getClipFolder(Track6), ...serializeTrackParams }),
        audioPlugin('ClipSequencer', { workspaceFolder, timelineFilename: `${getClipFolder(Track6)}/timeline.json`, target: 'SerializeTrack' }),
    ],
};

const sample1Track = {
    id: Sample1Track,
    plugins: [
        audioPlugin('SynthMultiSample', { alias: 'Sample1', preset }),
        audioPlugin('Sequencer', { defaultStepCount: 64 }),
        audioPlugin('SerializeTrack', { clipFolder: 'sample1', ...serializeTrackParams }),
    ],
};

const sample2Track = {
    id: Sample2Track,
    plugins: [
        audioPlugin('SynthMultiSample', { alias: 'Sample2', preset }),
        audioPlugin('Sequencer', { defaultStepCount: 64 }),
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
