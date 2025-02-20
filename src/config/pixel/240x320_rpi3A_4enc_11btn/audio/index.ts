import { audioPlugin, autoSave } from '@/libs/audio';
import { BassTrack, Drum23Track, Fm1Track, PercTrack, SampleTrack, SynthTrack } from '../constants';

export const cutoffStringFormat = '%d%% %d%%';
export const maxVariation = 16;
const workspaceFolder = 'workspaces/rpi3A_4enc_11btn';

let track = Drum23Track;
audioPlugin({ plugin: 'SynthDrum23', aliasName: 'Drum23', track });
audioPlugin({ plugin: 'Sequencer', track });
audioPlugin({ plugin: 'EffectDistortion2', aliasName: 'Distortion', track });
audioPlugin({ plugin: 'EffectFilterMultiMode', aliasName: 'MMFilter', cutoffStringFormat, track });
audioPlugin({ plugin: 'EffectGainVolume', aliasName: 'TrackFx', track });
audioPlugin({
    plugin: 'SerializeTrack',
    track,
    filename: 'drum23',
    maxVariation,
    workspaceFolder,
});

track = BassTrack;
audioPlugin({ plugin: 'SynthBass', aliasName: 'Bass', track });
audioPlugin({ plugin: 'Sequencer', track });
audioPlugin({ plugin: 'EffectGainVolume', aliasName: 'TrackFx', track });
audioPlugin({
    plugin: 'SerializeTrack',
    track,
    filename: 'bass',
    maxVariation,
    workspaceFolder,
});

track = Fm1Track;
audioPlugin({ plugin: 'SynthFmDrum', aliasName: 'FmDrum', track });
audioPlugin({ plugin: 'Sequencer', track });
audioPlugin({ plugin: 'EffectGainVolume', aliasName: 'TrackFx', track });
audioPlugin({
    plugin: 'SerializeTrack',
    track,
    filename: 'fmDrum',
    maxVariation,
    workspaceFolder,
});

track = SynthTrack;
audioPlugin({ plugin: 'SynthHybrid', aliasName: 'Synth', track });
audioPlugin({ plugin: 'Sequencer', track });
audioPlugin({ plugin: 'EffectGainVolume', aliasName: 'TrackFx', track });
audioPlugin({
    plugin: 'SerializeTrack',
    track,
    filename: 'synth',
    maxVariation,
    workspaceFolder,
});

audioPlugin({ plugin: 'SampleSequencer', track: SampleTrack });
audioPlugin({
    plugin: 'SerializeTrack',
    track: SampleTrack,
    filename: 'sampleSeq',
    maxVariation,
    workspaceFolder,
});

track = PercTrack;
audioPlugin({ plugin: 'SynthPerc', aliasName: 'Perc', track });
audioPlugin({ plugin: 'Sequencer', track });
audioPlugin({ plugin: 'EffectGainVolume', aliasName: 'TrackFx', track });
audioPlugin({
    plugin: 'SerializeTrack',
    track,
    filename: 'perc',
    maxVariation,
    workspaceFolder,
});

audioPlugin({ plugin: 'Mixer6', aliasName: 'Mixer' });
audioPlugin({ plugin: 'EffectGainVolume', aliasName: 'Volume' });
audioPlugin({ plugin: 'AudioOutputPulse', aliasName: 'AudioOutput' });
audioPlugin({
    plugin: 'SerializeTrack',
    filename: 'master',
    maxVariation,
    workspaceFolder,
});

audioPlugin({ plugin: 'Tempo' });

autoSave(500);
