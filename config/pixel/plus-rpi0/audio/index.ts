import { audioPlugin } from '@/libs/audio';
import {
    Drum1Track,
    Rack1Track,
    Rack2Track,
    Rack3Track,
    Rack4Track,
} from '../constants';

export const maxVariation = 16;
export const workspaceFolder = 'data/workspaces/plus_rpi0';

const drum1Track = {
    id: Drum1Track,
    plugins: [
        audioPlugin('SynthDrum23', { alias: 'Drum1' }),
        audioPlugin('Sequencer'),
        audioPlugin('EffectDistortion2', { alias: 'Distortion' }),
        audioPlugin('EffectFilterMultiMode', { alias: 'MMFilter' }),
        audioPlugin('EffectGainVolume', { alias: 'TrackFx' }),
        audioPlugin('SerializeTrack', { filename: 'drum1', maxVariation, workspaceFolder }),
    ],
};

const rack1Track = {
    id: Rack1Track,
    plugins: [
        audioPlugin('SynthMonoSample', { alias: 'Rack1' }),
        audioPlugin('Sequencer'),
        audioPlugin('EffectFilterMultiMode', { alias: 'Filter' }),
        audioPlugin('EffectVolumeMultiFx', { alias: 'TrackFx' }),
        audioPlugin('SerializeTrack', { filename: 'rack1', maxVariation, workspaceFolder }),
    ],
};

const rack2Track = {
    id: Rack2Track,
    plugins: [
        audioPlugin('SynthMonoSample', { alias: 'Rack2' }),
        audioPlugin('Sequencer'),
        audioPlugin('EffectFilterMultiMode', { alias: 'Filter' }),
        audioPlugin('EffectVolumeMultiFx', { alias: 'TrackFx' }),
        audioPlugin('SerializeTrack', { filename: 'rack2', maxVariation, workspaceFolder }),
    ],
};

const rack3Track = {
    id: Rack3Track,
    plugins: [
        audioPlugin('SynthMonoSample', { alias: 'Rack3' }),
        audioPlugin('Sequencer'),
        audioPlugin('EffectFilterMultiMode', { alias: 'Filter' }),
        audioPlugin('EffectVolumeMultiFx', { alias: 'TrackFx' }),
        audioPlugin('SerializeTrack', { filename: 'rack3', maxVariation, workspaceFolder }),
    ],
};

const rack4Track = {
    id: Rack4Track,
    plugins: [
        audioPlugin('SynthMonoSample', { alias: 'Rack4' }),
        audioPlugin('Sequencer'),
        audioPlugin('EffectFilterMultiMode', { alias: 'Filter' }),
        audioPlugin('EffectVolumeMultiFx', { alias: 'TrackFx' }),
        audioPlugin('SerializeTrack', { filename: 'rack4', maxVariation, workspaceFolder }),
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
        rack1Track,
        rack2Track,
        rack3Track,
        rack4Track,
        masterTrack,
    ],
    autoSave: 500,
};
