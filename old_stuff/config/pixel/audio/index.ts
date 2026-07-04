import { audioPlugin } from '@/libs/audio';
import { Track1, Track2, Track3, Track4, Track5, Track6, Track7, Track8 } from '../constants';

export const maxClip = 1000;

const workspaceFolder = 'data/workspaces/pixel';

const preset = true;

const track1Track = {
    id: Track1,
    plugins: [
        audioPlugin('SynthMultiEngine', { alias: 'Track1', preset }),
        audioPlugin('Sequencer', { defaultStepCount: 64 }),
        audioPlugin('SerializeTrack', { clipFolder: 'track1', maxClip, workspaceFolder }),
    ],
};

const track2Track = {
    id: Track2,
    plugins: [
        audioPlugin('SynthMultiEngine', { alias: 'Track2', preset }),
        audioPlugin('Sequencer', { defaultStepCount: 64 }),
        audioPlugin('SerializeTrack', { clipFolder: 'track2', maxClip, workspaceFolder }),
    ],
};

const track3Track = {
    id: Track3,
    plugins: [
        audioPlugin('SynthMultiEngine', { alias: 'Track3', preset }),
        audioPlugin('Sequencer', { defaultStepCount: 64 }),
        audioPlugin('SerializeTrack', { clipFolder: 'track3', maxClip, workspaceFolder }),
    ],
};

const track4Track = {
    id: Track4,
    plugins: [
        audioPlugin('SynthMultiEngine', { alias: 'Track4', preset }),
        audioPlugin('Sequencer', { defaultStepCount: 64 }),
        audioPlugin('SerializeTrack', { clipFolder: 'track4', maxClip, workspaceFolder }),
    ],
};

const track5Track = {
    id: Track5,
    plugins: [
        audioPlugin('SynthMultiEngine', { alias: 'Track5', preset }),
        audioPlugin('Sequencer', { defaultStepCount: 64 }),
        audioPlugin('SerializeTrack', { clipFolder: 'track5', maxClip, workspaceFolder }),
    ],
};

const track6Track = {
    id: Track6,
    plugins: [
        audioPlugin('SynthMultiEngine', { alias: 'Track6', preset }),
        audioPlugin('Sequencer', { defaultStepCount: 64 }),
        audioPlugin('SerializeTrack', { clipFolder: 'track6', maxClip, workspaceFolder }),
    ],
};

const track7Track = {
    id: Track7,
    plugins: [
        audioPlugin('SynthMultiEngine', { alias: 'Track7', preset }),
        audioPlugin('Sequencer', { defaultStepCount: 64 }),
        audioPlugin('SerializeTrack', { clipFolder: 'track7', maxClip, workspaceFolder }),
    ],
};

const track8Track = {
    id: Track8,
    plugins: [
        audioPlugin('SynthMultiEngine', { alias: 'Track8', preset }),
        audioPlugin('Sequencer', { defaultStepCount: 64 }),
        audioPlugin('SerializeTrack', { clipFolder: 'track8', maxClip, workspaceFolder }),
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
        audioPlugin('SerializeTrack', { clipFolder: 'master', maxClip, workspaceFolder }),
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
    tracks: [track1Track, track2Track, track3Track, track4Track, track5Track, track6Track, track7Track, track8Track, masterTrack],
    // autoSave: 500,
    repository: {
        folder: repositoriesFolder,
        default: defaultRepository,
    },
};
