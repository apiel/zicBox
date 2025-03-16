import { audioPlugin } from '@/libs/audio';
import {
    Drum23Track,
    PercTrack,
    Sample1Track,
    Sample2Track,
    Sample3Track,
    Synth1Track,
    Synth2Track,
    Synth3Track,
} from '../constants';

export const cutoffStringFormat = '%d%% %d%%';
export const maxVariation = 16;
const workspaceFolder = 'workspaces/rpi3A_4enc_11btn';

const drum23Track = {
    id: Drum23Track,
    plugins: [
        audioPlugin('SynthDrum23', { alias: 'Kick' }),
        audioPlugin('Sequencer'),
        audioPlugin('EffectDistortion2', { alias: 'Distortion' }),
        audioPlugin('EffectFilterMultiMode', { alias: 'MMFilter', cutoffStringFormat }),
        audioPlugin('EffectGainVolume', { alias: 'TrackFx' }),
        audioPlugin('SerializeTrack', { filename: 'drum23', maxVariation, workspaceFolder }),
    ],
};

const synth1Track = {
    id: Synth1Track,
    plugins: [
        audioPlugin('SynthWavetable', { alias: 'Synth1' }),
        audioPlugin('Sequencer'),
        audioPlugin('EffectVolumeMultiFx', { alias: 'TrackFx' }),
        audioPlugin('SerializeTrack', { filename: 'synth1', maxVariation, workspaceFolder }),
    ],
};

const synth2Track = {
    id: Synth2Track,
    plugins: [
        audioPlugin('SynthWavetable', { alias: 'Synth2' }),
        audioPlugin('Sequencer'),
        audioPlugin('EffectVolumeMultiFx', { alias: 'TrackFx' }),
        audioPlugin('SerializeTrack', { filename: 'synth2', maxVariation, workspaceFolder }),
    ],
};

const synth3Track = {
    id: Synth3Track,
    plugins: [
        audioPlugin('SynthWavetable', { alias: 'Synth3' }),
        audioPlugin('Sequencer'),
        audioPlugin('EffectVolumeMultiFx', { alias: 'TrackFx' }),
        audioPlugin('SerializeTrack', { filename: 'synth3', maxVariation, workspaceFolder }),
    ],
};

const percTrack = {
    id: PercTrack,
    plugins: [
        audioPlugin('SynthPerc', { alias: 'Perc' }),
        audioPlugin('Sequencer'),
        audioPlugin('EffectGainVolume', { alias: 'TrackFx' }),
        audioPlugin('SerializeTrack', { filename: 'perc', maxVariation, workspaceFolder }),
    ],
};

const sample1Track = {
    id: Sample1Track,
    plugins: [
        audioPlugin('SynthMonoSample', { alias: 'Sample1' }),
        audioPlugin('Sequencer'),
        audioPlugin('EffectFilter', { alias: 'Filter' }),
        audioPlugin('EffectVolumeMultiFx', { alias: 'TrackFx' }),
        audioPlugin('SerializeTrack', { filename: 'sample1', maxVariation, workspaceFolder }),
    ],
};

const sample2Track = {
    id: Sample2Track,
    plugins: [
        audioPlugin('SynthMonoSample', { alias: 'Sample2' }),
        audioPlugin('Sequencer'),
        audioPlugin('EffectFilter', { alias: 'Filter' }),
        audioPlugin('EffectVolumeMultiFx', { alias: 'TrackFx' }),
        audioPlugin('SerializeTrack', { filename: 'sample2', maxVariation, workspaceFolder }),
    ],
};

const sample3Track = {
    id: Sample3Track,
    plugins: [
        audioPlugin('SynthMonoSample', { alias: 'Sample3' }),
        audioPlugin('Sequencer'),
        audioPlugin('EffectFilter', { alias: 'Filter' }),
        audioPlugin('EffectVolumeMultiFx', { alias: 'TrackFx' }),
        audioPlugin('SerializeTrack', { filename: 'sample3', maxVariation, workspaceFolder }),
    ],
};

const masterTrack = {
    id: 0,
    plugins: [
        audioPlugin('Mixer8', { alias: 'Mixer' }),
        audioPlugin('EffectGainVolume', { alias: 'Volume' }),
        audioPlugin('AudioOutputPulse', { alias: 'AudioOutput' }),
        audioPlugin('SerializeTrack', { filename: 'master', maxVariation, workspaceFolder }),
        audioPlugin('Tempo'),
    ],
};

export const audio = {
    tracks: [
        drum23Track,
        synth1Track,
        synth2Track,
        synth3Track,
        percTrack,
        sample1Track,
        sample2Track,
        sample3Track,
        masterTrack,
    ],
    autoSave: 500,
};
