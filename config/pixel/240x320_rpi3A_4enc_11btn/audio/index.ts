import { audioPlugin } from '@/libs/audio';
import {
    BassTrack,
    Drum23Track,
    PercTrack,
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
        audioPlugin('SynthDrum23', { alias: 'Drum23' }),
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

const bassTrack = {
    id: BassTrack,
    plugins: [
        audioPlugin('SynthBass', { alias: 'Bass' }),
        audioPlugin('Sequencer'),
        audioPlugin('EffectGainVolume', { alias: 'TrackFx' }),
        audioPlugin('SerializeTrack', { filename: 'bass', maxVariation, workspaceFolder }),
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

const masterTrack = {
    id: 0,
    plugins: [
        audioPlugin('Mixer6', { alias: 'Mixer' }),
        audioPlugin('EffectGainVolume', { alias: 'Volume' }),
        audioPlugin('AudioOutputPulse', { alias: 'AudioOutput' }),
        audioPlugin('SerializeTrack', { filename: 'master', maxVariation, workspaceFolder }),
        audioPlugin('Tempo'),
    ],
};

export const audio = {
    tracks: [drum23Track, synth1Track, synth2Track, synth3Track, bassTrack, percTrack, masterTrack],
    autoSave: 500,
};
