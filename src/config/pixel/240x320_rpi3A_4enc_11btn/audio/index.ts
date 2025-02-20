import { audioPlugin, autoSave } from '@/libs/audio';
import { BassTrack, Drum23Track, Fm1Track, PercTrack, SampleTrack, SynthTrack } from '../constants';

export const cutoffStringFormat = '%d%% %d%%';
export const maxVariation = 16;
const workspaceFolder = 'workspaces/rpi3A_4enc_11btn';

let track = Drum23Track;
audioPlugin('SynthDrum23', { aliasName: 'Drum23', track });
audioPlugin('Sequencer', { track });
audioPlugin('EffectDistortion2', { aliasName: 'Distortion', track });
audioPlugin('EffectFilterMultiMode', { aliasName: 'MMFilter', cutoffStringFormat, track });
audioPlugin('EffectGainVolume', { aliasName: 'TrackFx', track });
audioPlugin('SerializeTrack', {
    track,
    filename: 'drum23',
    maxVariation,
    workspaceFolder,
});

track = BassTrack;
audioPlugin('SynthBass', { aliasName: 'Bass', track });
audioPlugin('Sequencer', { track });
audioPlugin('EffectGainVolume', { aliasName: 'TrackFx', track });
audioPlugin('SerializeTrack', {
    track,
    filename: 'bass',
    maxVariation,
    workspaceFolder,
});

track = Fm1Track;
audioPlugin('SynthFmDrum', { aliasName: 'FmDrum', track });
audioPlugin('Sequencer', { track });
audioPlugin('EffectGainVolume', { aliasName: 'TrackFx', track });
audioPlugin('SerializeTrack', {
    track,
    filename: 'fmDrum',
    maxVariation,
    workspaceFolder,
});

track = SynthTrack;
audioPlugin('SynthHybrid', { aliasName: 'Synth', track });
audioPlugin('Sequencer', { track });
audioPlugin('EffectGainVolume', { aliasName: 'TrackFx', track });
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
audioPlugin('SynthPerc', { aliasName: 'Perc', track });
audioPlugin('Sequencer', { track });
audioPlugin('EffectGainVolume', { aliasName: 'TrackFx', track });
audioPlugin('SerializeTrack', {
    track,
    filename: 'perc',
    maxVariation,
    workspaceFolder,
});

audioPlugin('Mixer6', { aliasName: 'Mixer' });
audioPlugin('EffectGainVolume', { aliasName: 'Volume' });
audioPlugin('AudioOutputPulse', { aliasName: 'AudioOutput' });
audioPlugin('SerializeTrack', {
    filename: 'master',
    maxVariation,
    workspaceFolder,
});

audioPlugin('Tempo');

autoSave(500);
