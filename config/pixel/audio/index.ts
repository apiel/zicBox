import { audioPlugin } from '@/libs/audio';
import {
    Drum1Track,
    Drum2Track,
    Drum3Track,
    Drum4Track,
    Sample1Track,
    Sample2Track,
    Sample3Track,
    Sample4Track
} from '../constants';

export const maxVariation = 16;
export const workspaceFolder = 'data/workspaces/rpi0';

const drum1Track = {
    id: Drum1Track,
    plugins: [
        audioPlugin('SynthMultiDrum', { alias: 'Drum1' }),
        audioPlugin('Sequencer', { stepCount: 64 }),
        // audioPlugin('EffectScatter', { alias: `Scatter${Drum2Track}` }),
        audioPlugin('EffectGainVolume', { alias: 'TrackFx' }),
        audioPlugin('SerializeTrack', { filename: 'drum1', maxVariation, workspaceFolder }),
    ],
};

const drum2Track = {
    id: Drum2Track,
    plugins: [
        audioPlugin('SynthMultiDrum', { alias: 'Drum2' }),
        audioPlugin('Sequencer', { stepCount: 64 }),
        // audioPlugin('EffectScatter', { alias: `Scatter${Drum2Track}` }),
        audioPlugin('EffectGainVolume', { alias: 'TrackFx' }),
        audioPlugin('SerializeTrack', { filename: 'drum2', maxVariation, workspaceFolder }),
    ],
};

const drum3Track = {
    id: Drum3Track,
    plugins: [
        audioPlugin('SynthMultiDrum', { alias: 'Drum3' }),
        audioPlugin('Sequencer', { stepCount: 64 }),
        // audioPlugin('EffectScatter', { alias: `Scatter${Drum3Track}` }),
        audioPlugin('EffectGainVolume', { alias: 'TrackFx' }),
        audioPlugin('SerializeTrack', { filename: 'drum3', maxVariation, workspaceFolder }),
    ],
};

const drum4Track = {
    id: Drum4Track,
    plugins: [
        audioPlugin('SynthMultiDrum', { alias: 'Drum4' }),
        audioPlugin('Sequencer', { stepCount: 64 }),
        // audioPlugin('EffectScatter', { alias: `Scatter${Drum4Track}` }),
        audioPlugin('EffectGainVolume', { alias: 'TrackFx' }),
        audioPlugin('SerializeTrack', { filename: 'drum4', maxVariation, workspaceFolder }),
    ],
};

const sample1Track = {
    id: Sample1Track,
    plugins: [
        audioPlugin('SynthMonoSample', { alias: 'Sample1' }),
        audioPlugin('Sequencer', { stepCount: 64 }),
        // audioPlugin('EffectScatter', { alias: `Scatter${Sample1Track}` }),
        audioPlugin('EffectFilterMultiMode', { alias: 'Filter' }),
        audioPlugin('EffectVolumeMultiFx', { alias: 'TrackFx' }),
        audioPlugin('EffectVolumeMultiFx', { alias: 'TrackFx2' }),
        audioPlugin('SerializeTrack', { filename: 'sample1', maxVariation, workspaceFolder }),
    ],
};

const sample2Track = {
    id: Sample2Track,
    plugins: [
        audioPlugin('SynthMonoSample', { alias: 'Sample2' }),
        audioPlugin('Sequencer', { stepCount: 64 }),
        // audioPlugin('EffectScatter', { alias: `Scatter${Sample2Track}` }),
        audioPlugin('EffectFilterMultiMode', { alias: 'Filter' }),
        audioPlugin('EffectVolumeMultiFx', { alias: 'TrackFx' }),
        audioPlugin('EffectVolumeMultiFx', { alias: 'TrackFx2' }),
        audioPlugin('SerializeTrack', { filename: 'sample2', maxVariation, workspaceFolder }),
    ],
};

const sample3Track = {
    id: Sample3Track,
    plugins: [
        audioPlugin('SynthMonoSample', { alias: 'Sample3' }),
        audioPlugin('Sequencer', { stepCount: 64 }),
        // audioPlugin('EffectScatter', { alias: `Scatter${Sample3Track}` }),
        audioPlugin('EffectFilterMultiMode', { alias: 'Filter' }),
        audioPlugin('EffectVolumeMultiFx', { alias: 'TrackFx' }),
        audioPlugin('EffectVolumeMultiFx', { alias: 'TrackFx2' }),
        audioPlugin('SerializeTrack', { filename: 'sample3', maxVariation, workspaceFolder }),
    ],
}

const sample4Track = {
    id: Sample4Track,
    plugins: [
        audioPlugin('SynthMonoSample', { alias: 'Sample4' }),
        audioPlugin('Sequencer', { stepCount: 64 }),
        // audioPlugin('EffectScatter', { alias: `Scatter${Sample3Track}` }),
        audioPlugin('EffectFilterMultiMode', { alias: 'Filter' }),
        audioPlugin('EffectVolumeMultiFx', { alias: 'TrackFx' }),
        audioPlugin('EffectVolumeMultiFx', { alias: 'TrackFx2' }),
        audioPlugin('SerializeTrack', { filename: 'sample4', maxVariation, workspaceFolder }),
    ],
}

const masterTrack = {
    id: 0,
    plugins: [
        audioPlugin('Mixer8', { alias: 'Mixer' }),
        audioPlugin('EffectGainVolume', { alias: 'TrackFx' }),
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
        drum2Track,
        drum3Track,
        drum4Track,
        sample1Track,
        sample2Track,
        sample3Track,
        sample4Track,
        masterTrack,
    ],
    autoSave: 500,
};
