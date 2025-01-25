import { autoSave, plugin, pluginAlias } from '@/libs/audio';
import { Drum23Track, SampleTrack } from '../constants';

pluginAlias('EffectDistortion2', 'libzic_EffectDistortion2.so');
pluginAlias('EffectFilterMultiMode', 'libzic_EffectFilterMultiMode.so');
pluginAlias('EffectGainVolume', 'libzic_EffectGainVolume.so');
pluginAlias('SerializeTrack', 'libzic_SerializeTrack.so');
pluginAlias('Tempo', 'libzic_Tempo.so');
pluginAlias('Drum23', 'libzic_SynthDrum23.so');
pluginAlias('Sample', 'libzic_SynthMonoSample.so');
pluginAlias('Sequencer', 'libzic_Sequencer.so');
pluginAlias('AudioOutput', 'libzic_AudioOutputPulse.so');
pluginAlias('Mixer', 'libzic_Mixer2.so');
pluginAlias('Tape', 'libzic_TapeRecording.so');
pluginAlias('SampleSequencer', 'libzic_SampleSequencer.so');

export const STRING_CUTOFF_FORMAT = '%d%% %d%%';
export const MAX_VARIATION = 16;
const WORKSPACE_FOLDER = 'workspaces_kick';

let track = Drum23Track;
plugin('Drum23', [{ track }]);
plugin('Sequencer', [{ track }]);
plugin('Distortion EffectDistortion2', [{ track }]);
plugin('MMFilter EffectFilterMultiMode', [{ STRING_CUTOFF_FORMAT, track }]);
plugin('TrackFx EffectGainVolume', [{ track }]);
plugin('SerializeTrack', [{ track, filename: 'drum23', MAX_VARIATION, WORKSPACE_FOLDER }]);

// track = SampleTrack;
// plugin('Sample', [{ track }]);
// plugin('Sequencer', [{ track }]);
// plugin('TrackFx EffectGainVolume', [{ track }]);
// plugin('SerializeTrack', [{ track, filename: 'sample', MAX_VARIATION, WORKSPACE_FOLDER }]);

plugin('SampleSequencer', [{ track: SampleTrack }]);
plugin('SerializeTrack', [{ track: SampleTrack, filename: 'sampleSeq', MAX_VARIATION, WORKSPACE_FOLDER }]);

plugin('Mixer');
plugin('Volume EffectGainVolume');
plugin('Tape', [{ filename: 'kick', max_track: 2 }]);
plugin('AudioOutput');
plugin('SerializeTrack', [{ filename: 'master', MAX_VARIATION, WORKSPACE_FOLDER }]);

plugin('Tempo');

autoSave(500);
