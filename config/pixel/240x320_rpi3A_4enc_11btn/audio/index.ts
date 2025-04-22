import { audioPlugin } from '@/libs/audio';
import {
    Drum1Track,
    Drum2Track,
    Drum3Track,
    KickTrack,
    Sample1Track,
    Sample2Track,
    Synth1Track,
    Synth2Track,
} from '../constants';

export const maxVariation = 16;
export const workspaceFolder = 'data/workspaces/rpi3A_4enc_11btn';

const kickTrack = {
    id: KickTrack,
    plugins: [
        audioPlugin('SynthDrum23', { alias: 'Kick' }),
        audioPlugin('Sequencer'),
        audioPlugin('EffectDistortion2', { alias: 'Distortion' }),
        audioPlugin('EffectFilterMultiMode', { alias: 'MMFilter' }),
        // audioPlugin('EffectFilterMultiModeMix', { alias: 'MMFilter', cutoffStringFormat: '%d%% %d%%' }),
        audioPlugin('EffectGainVolume', { alias: 'TrackFx' }),
        audioPlugin('SerializeTrack', { filename: 'kick', maxVariation, workspaceFolder }),
    ],
};

const drum1Track = {
    id: Drum1Track,
    plugins: [
        audioPlugin('SynthDrum23', { alias: 'Drum1' }),
        audioPlugin('Sequencer'),
        audioPlugin('EffectFilterMultiMode', { alias: 'Filter' }),
        audioPlugin('EffectVolumeMultiFx', { alias: 'TrackFx' }),
        audioPlugin('SerializeTrack', { filename: 'drum1', maxVariation, workspaceFolder }),
    ],
};

const drum2Track = {
    id: Drum2Track,
    plugins: [
        audioPlugin('SynthDrum23', { alias: 'Drum2' }),
        audioPlugin('Sequencer'),
        audioPlugin('EffectFilterMultiMode', { alias: 'Filter' }),
        audioPlugin('EffectVolumeMultiFx', { alias: 'TrackFx' }),
        audioPlugin('SerializeTrack', { filename: 'drum2', maxVariation, workspaceFolder }),
    ],
};

const drum3Track = {
    id: Drum3Track,
    plugins: [
        audioPlugin('SynthDrum23', { alias: 'Drum3' }),
        audioPlugin('Sequencer'),
        audioPlugin('EffectFilterMultiMode', { alias: 'Filter' }),
        audioPlugin('EffectVolumeMultiFx', { alias: 'TrackFx' }),
        audioPlugin('SerializeTrack', { filename: 'drum3', maxVariation, workspaceFolder }),
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

const sample1Track = {
    id: Sample1Track,
    plugins: [
        audioPlugin('SynthMonoSample', { alias: 'Sample1' }),
        audioPlugin('Sequencer'),
        audioPlugin('EffectFilterMultiMode', { alias: 'Filter' }),
        audioPlugin('EffectVolumeMultiFx', { alias: 'TrackFx' }),
        audioPlugin('SerializeTrack', { filename: 'sample1', maxVariation, workspaceFolder }),
    ],
};

const sample2Track = {
    id: Sample2Track,
    plugins: [
        audioPlugin('SynthMonoSample', { alias: 'Sample2' }),
        audioPlugin('Sequencer'),
        audioPlugin('EffectFilterMultiMode', { alias: 'Filter' }),
        audioPlugin('EffectVolumeMultiFx', { alias: 'TrackFx' }),
        audioPlugin('SerializeTrack', { filename: 'sample2', maxVariation, workspaceFolder }),
    ],
};

const masterTrack = {
    id: 0,
    plugins: [
        audioPlugin('Mixer8', { alias: 'Mixer' }),
        audioPlugin('EffectGainVolume', { alias: 'Volume' }),
        // audioPlugin('AudioOutputPulse', { alias: 'AudioOutput' }),
        audioPlugin('AudioOutputAlsa', { alias: 'AudioOutput' }),
        audioPlugin('SerializeTrack', { filename: 'master', maxVariation, workspaceFolder }),
        audioPlugin('Tempo'),
    ],
};

export const audio = {
    // midiInput: 'Arduino Leonardo',
    midiOnActiveMidiTrack: 1,
    autoLoadFirstMidiDevice: true,
    tracks: [
        kickTrack,
        drum1Track,
        drum2Track,
        drum3Track,
        synth1Track,
        synth2Track,
        sample1Track,
        sample2Track,
        masterTrack,
    ],
    autoSave: 500,
};
