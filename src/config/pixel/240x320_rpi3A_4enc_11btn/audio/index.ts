import { audioPlugin, autoSave } from '@/libs/audio';
import { BassTrack, Drum23Track, Fm1Track, PercTrack, SampleTrack, SynthTrack } from '../constants';

// pluginAlias('EffectDistortion2', 'libzic_EffectDistortion2.so');
// pluginAlias('EffectFilterMultiMode', 'libzic_EffectFilterMultiMode.so');
// pluginAlias('EffectGainVolume', 'libzic_EffectGainVolume.so');
// pluginAlias('SerializeTrack', 'libzic_SerializeTrack.so');
// pluginAlias('Tempo', 'libzic_Tempo.so');
// pluginAlias('Drum23', 'libzic_SynthDrum23.so');
// pluginAlias('FmDrum', 'libzic_SynthFmDrum.so');
// pluginAlias('Perc', 'libzic_SynthPerc.so');
// pluginAlias('Bass', 'libzic_SynthBass.so');
// pluginAlias('Synth', 'libzic_SynthHybrid.so');
// pluginAlias('Sample', 'libzic_SynthMonoSample.so');
// pluginAlias('Sequencer', 'libzic_Sequencer.so');
// pluginAlias('AudioOutput', 'libzic_AudioOutputPulse.so');
// pluginAlias('Mixer', 'libzic_Mixer6.so');
// pluginAlias('Tape', 'libzic_TapeRecording.so');
// pluginAlias('SampleSequencer', 'libzic_SampleSequencer.so');

export const STRING_CUTOFF_FORMAT = '%d%% %d%%';
export const MAX_VARIATION = 16;
const WORKSPACE_FOLDER = 'workspaces/rpi3A_4enc_11btn';

let track = Drum23Track;
// plugin('Drum23', [{ track }]);
audioPlugin({ plugin: 'SynthDrum23', aliasName: 'Drum23', track });
// plugin('Sequencer', [{ track }]);
audioPlugin({ plugin: 'Sequencer', track });
// plugin('Distortion EffectDistortion2', [{ track }]);
audioPlugin({ plugin: 'EffectDistortion2', aliasName: 'Distortion', track });
// plugin('MMFilter EffectFilterMultiMode', [{ STRING_CUTOFF_FORMAT, track }]);
audioPlugin({ plugin: 'EffectFilterMultiMode', aliasName: 'MMFilter', STRING_CUTOFF_FORMAT, track });
// plugin('TrackFx EffectGainVolume', [{ track }]);
audioPlugin({ plugin: 'EffectGainVolume', aliasName: 'TrackFx', track });
// plugin('SerializeTrack', [{ track, filename: 'drum23', MAX_VARIATION, WORKSPACE_FOLDER }]);
audioPlugin({
    plugin: 'SerializeTrack',
    track,
    filename: 'drum23',
    MAX_VARIATION,
    WORKSPACE_FOLDER,
});

track = BassTrack;
// plugin('Bass', [{ track }]);
audioPlugin({ plugin: 'SynthBass', aliasName: 'Bass', track });
// plugin('Sequencer', [{ track }]);
audioPlugin({ plugin: 'Sequencer', track });
// plugin('TrackFx EffectGainVolume', [{ track }]);
audioPlugin({ plugin: 'EffectGainVolume', aliasName: 'TrackFx', track });
// plugin('SerializeTrack', [{ track, filename: 'bass', MAX_VARIATION, WORKSPACE_FOLDER }]);
audioPlugin({
    plugin: 'SerializeTrack',
    track,
    filename: 'bass',
    MAX_VARIATION,
    WORKSPACE_FOLDER,
});

track = Fm1Track;
// plugin('FmDrum', [{ track }]);
audioPlugin({ plugin: 'SynthFmDrum', aliasName: 'FmDrum', track });
// plugin('Sequencer', [{ track }]);
audioPlugin({ plugin: 'Sequencer', track });
// plugin('TrackFx EffectGainVolume', [{ track }]);
audioPlugin({ plugin: 'EffectGainVolume', aliasName: 'TrackFx', track });
// plugin('SerializeTrack', [{ track, filename: 'fmDrum', MAX_VARIATION, WORKSPACE_FOLDER }]);
audioPlugin({
    plugin: 'SerializeTrack',
    track,
    filename: 'fmDrum',
    MAX_VARIATION,
    WORKSPACE_FOLDER,
});

track = SynthTrack;
// plugin('Synth', [{ track }]);
audioPlugin({ plugin: 'SynthHybrid', aliasName: 'Synth', track });
// plugin('Sequencer', [{ track }]);
audioPlugin({ plugin: 'Sequencer', track });
// plugin('TrackFx EffectGainVolume', [{ track }]);
audioPlugin({ plugin: 'EffectGainVolume', aliasName: 'TrackFx', track });
// plugin('SerializeTrack', [{ track, filename: 'synth', MAX_VARIATION, WORKSPACE_FOLDER }]);
audioPlugin({
    plugin: 'SerializeTrack',
    track,
    filename: 'synth',
    MAX_VARIATION,
    WORKSPACE_FOLDER,
});

// plugin('SampleSequencer', [{ track: SampleTrack }]);
audioPlugin({ plugin: 'SampleSequencer', track: SampleTrack });
// plugin('SerializeTrack', [
//     { track: SampleTrack, filename: 'sampleSeq', MAX_VARIATION, WORKSPACE_FOLDER },
// ]);
audioPlugin({
    plugin: 'SerializeTrack',
    track: SampleTrack,
    filename: 'sampleSeq',
    MAX_VARIATION,
    WORKSPACE_FOLDER,
});

track = PercTrack;
// plugin('Perc', [{ track }]);
audioPlugin({ plugin: 'SynthPerc', aliasName: 'Perc', track });
// plugin('Sequencer', [{ track }]);
audioPlugin({ plugin: 'Sequencer', track });
// plugin('TrackFx EffectGainVolume', [{ track }]);
audioPlugin({ plugin: 'EffectGainVolume', aliasName: 'TrackFx', track });
// plugin('SerializeTrack', [{ track, filename: 'perc', MAX_VARIATION, WORKSPACE_FOLDER }]);
audioPlugin({
    plugin: 'SerializeTrack',
    track,
    filename: 'perc',
    MAX_VARIATION,
    WORKSPACE_FOLDER,
});

// plugin('Mixer');
audioPlugin({ plugin: 'Mixer6', aliasName: 'Mixer' });
// plugin('Volume EffectGainVolume');
audioPlugin({ plugin: 'EffectGainVolume', aliasName: 'Volume' });
// plugin('AudioOutput');
audioPlugin({ plugin: 'AudioOutputPulse', aliasName: 'AudioOutput' });
// plugin('SerializeTrack', [{ filename: 'master', MAX_VARIATION, WORKSPACE_FOLDER }]);
audioPlugin({
    plugin: 'SerializeTrack',
    filename: 'master',
    MAX_VARIATION,
    WORKSPACE_FOLDER,
});

// plugin('Tempo');
audioPlugin({ plugin: 'Tempo' });

autoSave(500);
