import { audioPlugin } from '@/libs/audio';
import {
    Drum1Track,
    Drum2Track,
    Drum3Track,
    Drum4Track,
    Sample1Track,
    Sample2Track,
    Synth1Track,
    Synth2Track
} from '../constants';

export const maxVariation = 1000;

const drum1Track = {
    id: Drum1Track,
    plugins: [
        audioPlugin('SynthMultiDrum', { alias: 'Drum1' }),
        audioPlugin('Sequencer', { stepCount: 64 }),
        audioPlugin('SerializeTrack', { filename: 'drum1', maxVariation }),
    ],
};

const drum2Track = {
    id: Drum2Track,
    plugins: [
        audioPlugin('SynthMultiDrum', { alias: 'Drum2' }),
        audioPlugin('Sequencer', { stepCount: 64 }),
        audioPlugin('SerializeTrack', { filename: 'drum2', maxVariation }),
    ],
};

const drum3Track = {
    id: Drum3Track,
    plugins: [
        audioPlugin('SynthMultiDrum', { alias: 'Drum3' }),
        audioPlugin('Sequencer', { stepCount: 64 }),
        audioPlugin('SerializeTrack', { filename: 'drum3', maxVariation }),
    ],
};

const drum4Track = {
    id: Drum4Track,
    plugins: [
        audioPlugin('SynthMultiDrum', { alias: 'Drum4' }),
        audioPlugin('Sequencer', { stepCount: 64 }),
        audioPlugin('SerializeTrack', { filename: 'drum4', maxVariation }),
    ],
};

const sample1Track = {
    id: Sample1Track,
    plugins: [
        audioPlugin('SynthMultiSample', { alias: 'Sample1' }),
        audioPlugin('Sequencer', { stepCount: 64 }),
        audioPlugin('SerializeTrack', { filename: 'sample1', maxVariation }),
    ],
};

const sample2Track = {
    id: Sample2Track,
    plugins: [
        audioPlugin('SynthMultiSample', { alias: 'Sample2' }),
        audioPlugin('Sequencer', { stepCount: 64 }),
        audioPlugin('SerializeTrack', { filename: 'sample2', maxVariation }),
    ],
};

const synth1Track = {
    id: Synth1Track,
    plugins: [
        audioPlugin('SynthMulti', { alias: 'Synth1' }),
        audioPlugin('Sequencer', { stepCount: 64 }),
        audioPlugin('SerializeTrack', { filename: 'synth1', maxVariation }),
    ],
};

const synth2Track = {
    id: Synth2Track,
    plugins: [
        audioPlugin('SynthMulti', { alias: 'Synth2' }),
        audioPlugin('Sequencer', { stepCount: 64 }),
        audioPlugin('SerializeTrack', { filename: 'synth2', maxVariation }),
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
        audioPlugin('SerializeTrack', { filename: 'master', maxVariation }),
        audioPlugin('Tempo'),
    ],
};

export const repositoriesFolder = 'data/repositories';

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
    dataRepository: `${repositoriesFolder}/default_pixel`,
};
