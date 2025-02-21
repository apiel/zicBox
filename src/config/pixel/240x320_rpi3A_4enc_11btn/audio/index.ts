import { audioPlugin } from '@/libs/audio';
import { applyZic, jsonStringify } from '@/libs/core';
import { BassTrack, Drum23Track, Fm1Track, PercTrack, SampleTrack, SynthTrack } from '../constants';

export const cutoffStringFormat = '%d%% %d%%';
export const maxVariation = 16;
const workspaceFolder = 'workspaces/rpi3A_4enc_11btn';

const plugins = [];

let track = Drum23Track;
plugins.push(audioPlugin('SynthDrum23', { alias: 'Drum23', track }));
plugins.push(audioPlugin('Sequencer', { track }));
plugins.push(audioPlugin('EffectDistortion2', { alias: 'Distortion', track }));
plugins.push(
    audioPlugin('EffectFilterMultiMode', { alias: 'MMFilter', cutoffStringFormat, track })
);
plugins.push(audioPlugin('EffectGainVolume', { alias: 'TrackFx', track }));
plugins.push(
    audioPlugin('SerializeTrack', {
        track,
        filename: 'drum23',
        maxVariation,
        workspaceFolder,
    })
);

track = BassTrack;
plugins.push(audioPlugin('SynthBass', { alias: 'Bass', track }));
plugins.push(audioPlugin('Sequencer', { track }));
plugins.push(audioPlugin('EffectGainVolume', { alias: 'TrackFx', track }));
plugins.push(
    audioPlugin('SerializeTrack', {
        track,
        filename: 'bass',
        maxVariation,
        workspaceFolder,
    })
);

track = Fm1Track;
plugins.push(audioPlugin('SynthFmDrum', { alias: 'FmDrum', track }));
plugins.push(audioPlugin('Sequencer', { track }));
plugins.push(audioPlugin('EffectGainVolume', { alias: 'TrackFx', track }));
plugins.push(
    audioPlugin('SerializeTrack', {
        track,
        filename: 'fmDrum',
        maxVariation,
        workspaceFolder,
    })
);

track = SynthTrack;
plugins.push(audioPlugin('SynthHybrid', { alias: 'Synth', track }));
plugins.push(audioPlugin('Sequencer', { track }));
plugins.push(audioPlugin('EffectGainVolume', { alias: 'TrackFx', track }));
plugins.push(
    audioPlugin('SerializeTrack', {
        track,
        filename: 'synth',
        maxVariation,
        workspaceFolder,
    })
);

plugins.push(audioPlugin('SampleSequencer', { track: SampleTrack }));
plugins.push(
    audioPlugin('SerializeTrack', {
        track: SampleTrack,
        filename: 'sampleSeq',
        maxVariation,
        workspaceFolder,
    })
);

track = PercTrack;
plugins.push(audioPlugin('SynthPerc', { alias: 'Perc', track }));
plugins.push(audioPlugin('Sequencer', { track }));
plugins.push(audioPlugin('EffectGainVolume', { alias: 'TrackFx', track }));
plugins.push(
    audioPlugin('SerializeTrack', {
        track,
        filename: 'perc',
        maxVariation,
        workspaceFolder,
    })
);

plugins.push(audioPlugin('Mixer6', { alias: 'Mixer' }));
plugins.push(audioPlugin('EffectGainVolume', { alias: 'Volume' }));
plugins.push(audioPlugin('AudioOutputPulse', { alias: 'AudioOutput' }));
plugins.push(
    audioPlugin('SerializeTrack', {
        filename: 'master',
        maxVariation,
        workspaceFolder,
    })
);

plugins.push(audioPlugin('Tempo'));

// autoSave(500);

applyZic([{ AUDIO: jsonStringify({ plugins, autoSave: 500 }) }]);
