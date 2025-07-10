import { audioPlugin } from '@/libs/audio';
import {
    Drum1Track,
    Sample1Track,
    Sample2Track,
    Sample3Track,
    Sample4Track,
} from '../constants';

export const maxVariation = 16;
export const workspaceFolder = 'data/workspaces/plus_rpi0';

const drum1Track = {
    id: Drum1Track,
    plugins: [
        audioPlugin('SynthKick23', { alias: 'Drum1' }),
        audioPlugin('Sequencer'),
        audioPlugin('EffectFilterMultiMode', { alias: 'MMFilter' }),
        audioPlugin('EffectGainVolume', { alias: 'TrackFx' }),
        audioPlugin('SerializeTrack', { filename: 'drum1', maxVariation, workspaceFolder }),
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

const sample3Track = {
    id: Sample3Track,
    plugins: [
        audioPlugin('SynthMonoSample', { alias: 'Sample3' }),
        audioPlugin('Sequencer'),
        audioPlugin('EffectFilterMultiMode', { alias: 'Filter' }),
        audioPlugin('EffectVolumeMultiFx', { alias: 'TrackFx' }),
        audioPlugin('SerializeTrack', { filename: 'sample3', maxVariation, workspaceFolder }),
    ],
};

const sample4Track = {
    id: Sample4Track,
    plugins: [
        audioPlugin('SynthMonoSample', { alias: 'Sample4' }),
        audioPlugin('Sequencer'),
        audioPlugin('EffectFilterMultiMode', { alias: 'Filter' }),
        audioPlugin('EffectVolumeMultiFx', { alias: 'TrackFx' }),
        audioPlugin('SerializeTrack', { filename: 'sample4', maxVariation, workspaceFolder }),
    ],
};

const masterTrack = {
    id: 0,
    plugins: [
        audioPlugin('Mixer8', { alias: 'Mixer' }),
        audioPlugin('EffectGainVolume', { alias: 'Volume' }),
        audioPlugin('AudioOutputPulse', { alias: 'AudioOutput' }),
        // audioPlugin('AudioOutputAlsa', { alias: 'AudioOutput' }),
        audioPlugin('SerializeTrack', { filename: 'master', maxVariation, workspaceFolder }),
        audioPlugin('Tempo'),
    ],
};

export const audio = {
    // midiInput: 'Arduino Leonardo',
    midiOnActiveMidiTrack: 1,
    autoLoadFirstMidiDevice: true,
    tracks: [
        drum1Track,
        sample1Track,
        sample2Track,
        sample3Track,
        sample4Track,
        masterTrack,
    ],
    autoSave: 500,
};
