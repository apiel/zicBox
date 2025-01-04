import { autoSave, plugin, pluginAlias } from '@/libs/audio';
import { STRING_CUTOFF_FORMAT } from './constants';
import { drum23 } from './drum23';
import { drumSample } from './drumSample';

pluginAlias('EffectDistortion', 'libzic_EffectDistortion.so');
pluginAlias('EffectDistortion2', 'libzic_EffectDistortion2.so');
pluginAlias('EffectFilterMultiMode', 'libzic_EffectFilterMultiMode.so');
pluginAlias('EffectSampleRateReducer', 'libzic_EffectSampleRateReducer.so');
pluginAlias('EffectGainVolume', 'libzic_EffectGainVolume.so');
pluginAlias('EffectVolumeDrive', 'libzic_EffectVolumeDrive.so');
pluginAlias('EffectVolumeClipping', 'libzic_EffectVolumeClipping.so');
pluginAlias('EffectDelay', 'libzic_EffectDelay.so');
pluginAlias('Mixer2', 'libzic_Mixer2.so');
pluginAlias('Mixer4', 'libzic_Mixer4.so');
pluginAlias('Mixer8', 'libzic_Mixer8.so');
pluginAlias('SerializeTrack', 'libzic_SerializeTrack.so');
pluginAlias('Tempo', 'libzic_Tempo.so');
pluginAlias('SynthFM', 'libzic_SynthFM2.so');
pluginAlias('Drum23', 'libzic_SynthDrum23.so');
pluginAlias('DrumSample', 'libzic_SynthDrumSample.so');
pluginAlias('Sequencer', 'libzic_Sequencer.so');
pluginAlias('AudioOutput', 'libzic_AudioOutputPulse.so');
pluginAlias('Spectrogram', 'libzic_AudioSpectrogram.so');

// Drum tracks
drum23(1);
drum23(2);
drumSample(3);
drumSample(4);

plugin('Mixer4', [{ name: 'MixerDrum', track: 9 }]);
plugin('SerializeTrack', [{ track: 9, filename: 'mixer_drum' }]);

// FM tracks
drumSample(5);
drumSample(6);
drumSample(7);
drumSample(8);

plugin('Mixer4', [{ name: 'MixerSynth', track_start: 5, track: 10 }]);
plugin('SerializeTrack', [{ track: 10, filename: 'mixer_synth' }]);

// Master

plugin('Mixer2', [{ name: 'MixerMaster', track_a: 9, track_b: 10, value: 50, track: 0 }]);

plugin('EffectFilterMultiMode', [{ name: 'MasterFilter', STRING_CUTOFF_FORMAT }]);
plugin('EffectGainVolume', [{ name: 'MasterVolume' }]);
plugin('Spectrogram');
plugin('AudioOutput');
plugin('SerializeTrack', [{ filename: 'master' }]);

plugin('Tempo');

autoSave(500);
