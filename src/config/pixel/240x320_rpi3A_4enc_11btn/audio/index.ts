import { audioPlugin } from '@/libs/audio';
import { BassTrack, Drum23Track, Fm1Track, PercTrack, SampleTrack, SynthTrack } from '../constants';

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

const bassTrack = {
    id: BassTrack,
    plugins: [
        audioPlugin('SynthBass', { alias: 'Bass' }),
        audioPlugin('Sequencer'),
        audioPlugin('EffectGainVolume', { alias: 'TrackFx' }),
        audioPlugin('SerializeTrack', { filename: 'bass', maxVariation, workspaceFolder }),
    ],
};

const fm1Track = {
    id: Fm1Track,
    plugins: [
        audioPlugin('SynthFmDrum', { alias: 'FmDrum' }),
        audioPlugin('Sequencer'),
        audioPlugin('EffectGainVolume', { alias: 'TrackFx' }),
        audioPlugin('SerializeTrack', { filename: 'fmDrum', maxVariation, workspaceFolder }),
    ],
};

const synthTrack = {
    id: SynthTrack,
    plugins: [
        audioPlugin('SynthHybrid', { alias: 'Synth' }),
        audioPlugin('Sequencer'),
        audioPlugin('EffectGainVolume', { alias: 'TrackFx' }),
        audioPlugin('SerializeTrack', { filename: 'synth', maxVariation, workspaceFolder }),
    ],
};

const sampleTrack = {
    id: SampleTrack,
    plugins: [
        audioPlugin('SampleSequencer'),
        audioPlugin('Sequencer'),
        audioPlugin('EffectGainVolume', { alias: 'TrackFx' }),
        audioPlugin('SerializeTrack', { filename: 'sampleSeq', maxVariation, workspaceFolder }),
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
    tracks: [drum23Track, bassTrack, fm1Track, synthTrack, sampleTrack, percTrack, masterTrack],
    autoSave: 500,
};
