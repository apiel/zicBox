import { audioPlugin } from '@/libs/audio';

export const maxVariation = 1000;

const workspaceFolder = 'data/workspaces/grain';

const masterTrack = {
    id: 0,
    plugins: [
        audioPlugin('SynthLoop'),
        audioPlugin('EffectGainVolume', { alias: 'TrackFx' }),
        audioPlugin('AudioOutputAlsa_int16', { alias: 'AudioOutput' }),
        audioPlugin('SerializeTrack', { filename: 'master', maxVariation, workspaceFolder }),
        audioPlugin('Tempo'),
    ],
};

export const repositoriesFolder = 'data/repositories';
export const defaultRepository = 'default_grain';

export const audio = {
    // midiInput: 'MPK mini',
    initActiveMidiTrack: 1,
    autoLoadFirstMidiDevice: true,
    // debugMidi: true, // Will print midi messages that are not used...
    tracks: [
        masterTrack,
    ],
    // autoSave: 500,
    repository: {
        folder: repositoriesFolder,
        default: defaultRepository,
    }
};
