import { audioPlugin } from '@/libs/audio';
import {
    Sample1Track,
    Sample2Track,
    Track1,
    Track2,
    Track3,
    Track4,
    Track5,
    Track6
} from '../constants';

export const maxVariation = 1000;

const workspaceFolder = 'data/workspaces/pixel';

const drum1Track = {
    id: Track1,
    plugins: [
        audioPlugin('SynthMultiEngine', { alias: 'Drum1' }),
        audioPlugin('Sequencer', { stepCount: 64 }),
        audioPlugin('SerializeTrack', { filename: 'drum1', maxVariation, workspaceFolder }),
    ],
};

const drum2Track = {
    id: Track2,
    plugins: [
        audioPlugin('SynthMultiEngine', { alias: 'Drum2' }),
        audioPlugin('Sequencer', { stepCount: 64 }),
        audioPlugin('SerializeTrack', { filename: 'drum2', maxVariation, workspaceFolder }),
    ],
};

const drum3Track = {
    id: Track3,
    plugins: [
        audioPlugin('SynthMultiEngine', { alias: 'Drum3' }),
        audioPlugin('Sequencer', { stepCount: 64 }),
        audioPlugin('SerializeTrack', { filename: 'drum3', maxVariation, workspaceFolder }),
    ],
};

const drum4Track = {
    id: Track4,
    plugins: [
        audioPlugin('SynthMultiEngine', { alias: 'Drum4' }),
        audioPlugin('Sequencer', { stepCount: 64 }),
        audioPlugin('SerializeTrack', { filename: 'drum4', maxVariation, workspaceFolder }),
    ],
};

const sample1Track = {
    id: Sample1Track,
    plugins: [
        audioPlugin('SynthMultiSample', { alias: 'Sample1' }),
        audioPlugin('Sequencer', { stepCount: 64 }),
        audioPlugin('SerializeTrack', { filename: 'sample1', maxVariation, workspaceFolder }),
    ],
};

const sample2Track = {
    id: Sample2Track,
    plugins: [
        audioPlugin('SynthMultiSample', { alias: 'Sample2' }),
        audioPlugin('Sequencer', { stepCount: 64 }),
        audioPlugin('SerializeTrack', { filename: 'sample2', maxVariation, workspaceFolder }),
    ],
};

const synth1Track = {
    id: Track5,
    plugins: [
        audioPlugin('SynthMultiEngine', { alias: 'Synth1' }),
        audioPlugin('Sequencer', { stepCount: 64 }),
        audioPlugin('SerializeTrack', { filename: 'synth1', maxVariation, workspaceFolder }),
    ],
};

const synth2Track = {
    id: Track6,
    plugins: [
        audioPlugin('SynthMultiEngine', { alias: 'Synth2' }),
        audioPlugin('Sequencer', { stepCount: 64 }),
        audioPlugin('SerializeTrack', { filename: 'synth2', maxVariation, workspaceFolder }),
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
        drum1Track,
        drum2Track,
        drum3Track,
        drum4Track,
        sample1Track,
        sample2Track,
        synth1Track,
        synth2Track,
        masterTrack,
    ],
    // autoSave: 500,
    repository: {
        folder: repositoriesFolder,
        default: defaultRepository,
    }
};
