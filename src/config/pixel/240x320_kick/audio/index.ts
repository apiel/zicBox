import { autoSave, plugin, pluginAlias } from '@/libs/audio';
import { MAX_VARIATION, STRING_CUTOFF_FORMAT } from './constants';

pluginAlias('EffectDistortion2', 'libzic_EffectDistortion2.so');
pluginAlias('EffectFilterMultiMode', 'libzic_EffectFilterMultiMode.so');
// pluginAlias('EffectSampleRateReducer', 'libzic_EffectSampleRateReducer.so');
pluginAlias('EffectGainVolume', 'libzic_EffectGainVolume.so');
pluginAlias('SerializeTrack', 'libzic_SerializeTrack.so');
pluginAlias('Tempo', 'libzic_Tempo.so');
pluginAlias('Drum23', 'libzic_SynthDrum23.so');
pluginAlias('Sequencer', 'libzic_Sequencer.so');
pluginAlias('AudioOutput', 'libzic_AudioOutputPulse.so');
pluginAlias('Spectrogram', 'libzic_AudioSpectrogram.so');
pluginAlias('Tape', 'libzic_TapeRecording.so');

plugin('Drum23');
plugin('Sequencer');
plugin('Distortion EffectDistortion2');
plugin('MMFilter EffectFilterMultiMode', [{ STRING_CUTOFF_FORMAT }]);
plugin('Volume EffectGainVolume');
// plugin('Tape', [{ filename: 'kick', max_track: 0 }]);
plugin('AudioOutput');
plugin('SerializeTrack', [{ filename: 'master', MAX_VARIATION, WORKSPACE_FOLDER: 'workspaces_kick' }]);

plugin('Tempo');

autoSave(500);
