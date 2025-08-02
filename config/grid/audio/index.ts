import { audioPlugin } from '@/libs/audio';
import {
    Drum1Track,
    Drum2Track,
    Drum3Track,
    Drum4Track,
    Sample1Track,
    Sample2Track,
    Sample3Track,
    Synth1Track,
    Synth2Track,
    Synth3Track,
} from '../constants';

export const maxVariation = 16;
export const workspaceFolder = 'data/workspaces/plus_rpi0';

const drum1Track = {
    id: Drum1Track,
    plugins: [
        audioPlugin('SynthKick23', { alias: 'Drum1' }),
        audioPlugin('Sequencer', { stepCount: 64 }),
        audioPlugin('EffectGainVolume', { alias: 'TrackFx' }),
        audioPlugin('SerializeTrack', { filename: 'drum1', maxVariation, workspaceFolder }),
    ],
};

// SynthMultiDrum

const drum2Track = {
    id: Drum2Track,
    plugins: [
        audioPlugin('SynthMultiDrum', { alias: 'Drum2' }),
        audioPlugin('Sequencer', { stepCount: 64 }),
        audioPlugin('EffectGainVolume', { alias: 'TrackFx' }),
        audioPlugin('SerializeTrack', { filename: 'drum2', maxVariation, workspaceFolder }),
    ],
};

const drum3Track = {
    id: Drum3Track,
    plugins: [
        audioPlugin('SynthMultiDrum', { alias: 'Drum3' }),
        audioPlugin('Sequencer', { stepCount: 64 }),
        audioPlugin('EffectGainVolume', { alias: 'TrackFx' }),
        audioPlugin('SerializeTrack', { filename: 'drum3', maxVariation, workspaceFolder }),
    ],
};

const drum4Track = {
    id: Drum4Track,
    plugins: [
        audioPlugin('SynthMultiDrum', { alias: 'Drum4' }),
        audioPlugin('Sequencer', { stepCount: 64 }),
        audioPlugin('EffectGainVolume', { alias: 'TrackFx' }),
        audioPlugin('SerializeTrack', { filename: 'drum4', maxVariation, workspaceFolder }),
    ],
};

const synth1Track = {
    id: Synth1Track,
    plugins: [
        audioPlugin('SynthWavetable', { alias: 'Synth1' }),
        audioPlugin('Sequencer', { stepCount: 64 }),
        audioPlugin('EffectVolumeMultiFx', { alias: 'TrackFx' }),
        audioPlugin('EffectVolumeMultiFx', { alias: 'TrackFx2' }),
        audioPlugin('SerializeTrack', { filename: 'synth1', maxVariation, workspaceFolder }),
    ],
};

const synth2Track = {
    id: Synth2Track,
    plugins: [
        audioPlugin('SynthWavetable', { alias: 'Synth2' }),
        audioPlugin('Sequencer', { stepCount: 64 }),
        audioPlugin('EffectVolumeMultiFx', { alias: 'TrackFx' }),
        audioPlugin('EffectVolumeMultiFx', { alias: 'TrackFx2' }),
        audioPlugin('SerializeTrack', { filename: 'synth2', maxVariation, workspaceFolder }),
    ],
};

const synth3Track = {
    id: Synth3Track,
    plugins: [
        audioPlugin('SynthWavetable', { alias: 'Synth3' }),
        audioPlugin('Sequencer', { stepCount: 64 }),
        audioPlugin('EffectVolumeMultiFx', { alias: 'TrackFx' }),
        audioPlugin('EffectVolumeMultiFx', { alias: 'TrackFx2' }),
        audioPlugin('SerializeTrack', { filename: 'synth3', maxVariation, workspaceFolder }),
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
}

const masterTrack = {
    id: 0,
    plugins: [
        audioPlugin('Mixer10', { alias: 'Mixer' }),
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
        drum2Track,
        drum3Track,
        drum4Track,
        synth1Track,
        synth2Track,
        synth3Track,
        sample1Track,
        sample2Track,
        sample3Track,
        masterTrack,
    ],
    autoSave: 500,
};
