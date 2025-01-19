import { autoSave, plugin, pluginAlias } from '@/libs/audio';

pluginAlias('EffectDistortion2', 'libzic_EffectDistortion2.so');
pluginAlias('EffectFilterMultiMode', 'libzic_EffectFilterMultiMode.so');
// pluginAlias('EffectSampleRateReducer', 'libzic_EffectSampleRateReducer.so');
pluginAlias('EffectGainVolume', 'libzic_EffectGainVolume.so');
pluginAlias('SerializeTrack', 'libzic_SerializeTrack.so');
pluginAlias('Tempo', 'libzic_Tempo.so');
pluginAlias('Drum23', 'libzic_SynthDrum23.so');
pluginAlias('Snare', 'libzic_SynthSnare.so');
pluginAlias('HiHat', 'libzic_SynthHiHat.so');
pluginAlias('Clap', 'libzic_SynthFmDrum.so');
pluginAlias('Sequencer', 'libzic_Sequencer.so');
pluginAlias('AudioOutput', 'libzic_AudioOutputPulse.so');
pluginAlias('Mixer4', 'libzic_Mixer4.so');
pluginAlias('Tape', 'libzic_TapeRecording.so');

export const STRING_CUTOFF_FORMAT = '%d%% %d%%';
export const MAX_VARIATION = 16;
const WORKSPACE_FOLDER = 'workspaces_kick';

let track = 1;
plugin('Drum23', [{ track }]);
plugin('Sequencer', [{ track }]);
plugin('Distortion EffectDistortion2', [{ track }]);
plugin('MMFilter EffectFilterMultiMode', [{ STRING_CUTOFF_FORMAT, track }]);
plugin('Volume EffectGainVolume', [{ track }]);
plugin('SerializeTrack', [{ track, filename: 'drum23', MAX_VARIATION, WORKSPACE_FOLDER }]);

track = 2;
plugin('Snare', [{ track }]);
plugin('Sequencer', [{ track }]);
plugin('Volume EffectGainVolume', [{ track }]);
plugin('SerializeTrack', [{ track, filename: 'snare', MAX_VARIATION, WORKSPACE_FOLDER }]);

track = 3;
plugin('HiHat', [{ track }]);
plugin('Sequencer', [{ track }]);
plugin('Volume EffectGainVolume', [{ track }]);
plugin('SerializeTrack', [{ track, filename: 'hihat', MAX_VARIATION, WORKSPACE_FOLDER }]);

track = 4;
plugin('Clap', [{ track }]);
plugin('Sequencer', [{ track }]);
plugin('Volume EffectGainVolume', [{ track }]);
plugin('SerializeTrack', [{ track, filename: 'clap', MAX_VARIATION, WORKSPACE_FOLDER }]);

plugin('MixerDrum Mixer4');
plugin('SerializeTrack', [{ filename: 'mixer', MAX_VARIATION, WORKSPACE_FOLDER }]);

// plugin('Tape', [{ filename: 'kick', max_track: 0 }]);
plugin('AudioOutput');
plugin('SerializeTrack', [{ filename: 'master', MAX_VARIATION, WORKSPACE_FOLDER }]);

plugin('Tempo');

autoSave(500);
