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

export const maxVariation = 1000;

const workspaceFolder = 'data/workspaces/pixel';

const preset = true;

const track1Track = {
    id: Track1,
    plugins: [
        audioPlugin('SynthMultiEngine', { alias: 'Track1', preset }),
        audioPlugin('Sequencer', { stepCount: 64 }),
        audioPlugin('SerializeTrack', { filename: 'track1', maxVariation, workspaceFolder }),
    ],
};

const track2Track = {
    id: Track2,
    plugins: [
        audioPlugin('SynthMultiEngine', { alias: 'Track2', preset }),
        audioPlugin('Sequencer', { stepCount: 64 }),
        audioPlugin('SerializeTrack', { filename: 'track2', maxVariation, workspaceFolder }),
    ],
};

const track3Track = {
    id: Track3,
    plugins: [
        audioPlugin('SynthMultiEngine', { alias: 'Track3', preset }),
        audioPlugin('Sequencer', { stepCount: 64 }),
        audioPlugin('SerializeTrack', { filename: 'track3', maxVariation, workspaceFolder }),
    ],
};

const track4Track = {
    id: Track4,
    plugins: [
        audioPlugin('SynthMultiEngine', { alias: 'Track4', preset }),
        audioPlugin('Sequencer', { stepCount: 64 }),
        audioPlugin('SerializeTrack', { filename: 'track4', maxVariation, workspaceFolder }),
    ],
};

const track5Track = {
    id: Track5,
    plugins: [
        audioPlugin('SynthMultiEngine', { alias: 'Track5', preset }),
        audioPlugin('Sequencer', { stepCount: 64 }),
        audioPlugin('SerializeTrack', { filename: 'track5', maxVariation, workspaceFolder }),
    ],
};

const track6Track = {
    id: Track6,
    plugins: [
        audioPlugin('SynthMultiEngine', { alias: 'Track6', preset }),
        audioPlugin('Sequencer', { stepCount: 64 }),
        audioPlugin('SerializeTrack', { filename: 'track6', maxVariation, workspaceFolder }),
    ],
};

const sample1Track = {
    id: Sample1Track,
    plugins: [
        audioPlugin('SynthMultiSample', { alias: 'Sample1', preset }),
        audioPlugin('Sequencer', { stepCount: 64 }),
        audioPlugin('SerializeTrack', { filename: 'sample1', maxVariation, workspaceFolder }),
    ],
};

const sample2Track = {
    id: Sample2Track,
    plugins: [
        audioPlugin('SynthMultiSample', { alias: 'Sample2', preset }),
        audioPlugin('Sequencer', { stepCount: 64 }),
        audioPlugin('SerializeTrack', { filename: 'sample2', maxVariation, workspaceFolder }),
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
        audioPlugin('SerializeTrack', { filename: 'master', maxVariation, workspaceFolder }),
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
