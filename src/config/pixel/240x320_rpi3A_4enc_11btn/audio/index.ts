import { audioPlugin, autoSave } from '@/libs/audio';
import { BassTrack, Drum23Track, Fm1Track, PercTrack, SampleTrack, SynthTrack } from '../constants';

export const cutoffStringFormat = '%d%% %d%%';
export const maxVariation = 16;
const workspaceFolder = 'workspaces/rpi3A_4enc_11btn';

let track = Drum23Track;
audioPlugin('SynthDrum23', { alias: 'Drum23', track });
audioPlugin('Sequencer', { track });
audioPlugin('EffectDistortion2', { alias: 'Distortion', track });
audioPlugin('EffectFilterMultiMode', { alias: 'MMFilter', cutoffStringFormat, track });
audioPlugin('EffectGainVolume', { alias: 'TrackFx', track });
audioPlugin('SerializeTrack', {
    track,
    filename: 'drum23',
    maxVariation,
    workspaceFolder,
});

track = BassTrack;
audioPlugin('SynthBass', { alias: 'Bass', track });
audioPlugin('Sequencer', { track });
audioPlugin('EffectGainVolume', { alias: 'TrackFx', track });
audioPlugin('SerializeTrack', {
    track,
    filename: 'bass',
    maxVariation,
    workspaceFolder,
});

track = Fm1Track;
audioPlugin('SynthFmDrum', { alias: 'FmDrum', track });
audioPlugin('Sequencer', { track });
audioPlugin('EffectGainVolume', { alias: 'TrackFx', track });
audioPlugin('SerializeTrack', {
    track,
    filename: 'fmDrum',
    maxVariation,
    workspaceFolder,
});

track = SynthTrack;
audioPlugin('SynthHybrid', { alias: 'Synth', track });
audioPlugin('Sequencer', { track });
audioPlugin('EffectGainVolume', { alias: 'TrackFx', track });
audioPlugin('SerializeTrack', {
    track,
    filename: 'synth',
    maxVariation,
    workspaceFolder,
});

audioPlugin('SampleSequencer', { track: SampleTrack });
audioPlugin('SerializeTrack', {
    track: SampleTrack,
    filename: 'sampleSeq',
    maxVariation,
    workspaceFolder,
});

track = PercTrack;
audioPlugin('SynthPerc', { alias: 'Perc', track });
audioPlugin('Sequencer', { track });
audioPlugin('EffectGainVolume', { alias: 'TrackFx', track });
audioPlugin('SerializeTrack', {
    track,
    filename: 'perc',
    maxVariation,
    workspaceFolder,
});

audioPlugin('Mixer6', { alias: 'Mixer' });
audioPlugin('EffectGainVolume', { alias: 'Volume' });
audioPlugin('AudioOutputPulse', { alias: 'AudioOutput' });
audioPlugin('SerializeTrack', {
    filename: 'master',
    maxVariation,
    workspaceFolder,
});

audioPlugin('Tempo');

autoSave(500);
