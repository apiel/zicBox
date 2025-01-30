import { autoSave, plugin, pluginAlias } from '@/libs/audio';
import { STRING_CUTOFF_FORMAT, WORKSPACE_FOLDER } from './constants';
import { drum23 } from './drum23';
import { monoSample } from './monoSample';

pluginAlias('EffectDistortion', 'libzic_EffectDistortion.so');
pluginAlias('EffectDistortion2', 'libzic_EffectDistortion2.so');
pluginAlias('EffectFilterMultiMode', 'libzic_EffectFilterMultiMode.so');
pluginAlias('EffectSampleRateReducer', 'libzic_EffectSampleRateReducer.so');
pluginAlias('EffectGainVolume', 'libzic_EffectGainVolume.so');
pluginAlias('EffectVolumeClipping', 'libzic_EffectVolumeClipping.so');
pluginAlias('EffectDelay', 'libzic_EffectDelay.so');
pluginAlias('Mixer2', 'libzic_Mixer2.so');
pluginAlias('Mixer4', 'libzic_Mixer4.so');
pluginAlias('Mixer8', 'libzic_Mixer8.so');
pluginAlias('SerializeTrack', 'libzic_SerializeTrack.so');
pluginAlias('Tempo', 'libzic_Tempo.so');
pluginAlias('SynthFM', 'libzic_SynthFM2.so');
pluginAlias('Drum23', 'libzic_SynthDrum23.so');
pluginAlias('MonoSample', 'libzic_SynthMonoSample.so');
pluginAlias('Sequencer', 'libzic_Sequencer.so');
pluginAlias('AudioOutput', 'libzic_AudioOutputPulse.so');
pluginAlias('Spectrogram', 'libzic_AudioSpectrogram.so');
pluginAlias('Tape', 'libzic_TapeRecording.so');
pluginAlias('RamTape', 'libzic_RamTapeRecording.so');

// Drum tracks
drum23(1);
drum23(2);
monoSample(3);
monoSample(4);

plugin('MixerDrum Mixer4', [{ track: 9 }]);
plugin('SerializeTrack', [{ track: 9, filename: 'mixer_drum', WORKSPACE_FOLDER }]);

// Synth tracks
monoSample(5);
monoSample(6);
monoSample(7);
monoSample(8);

plugin('MixerSynth Mixer4', [{ track_start: 5, track: 10 }]);
plugin('SerializeTrack', [{ track: 10, filename: 'mixer_synth', WORKSPACE_FOLDER }]);

// Master

plugin('MixerMaster Mixer2', [{ track_a: 9, track_b: 10, value: 50, track: 0 }]);

plugin('MasterFilter EffectFilterMultiMode', [{ STRING_CUTOFF_FORMAT }]);
plugin('MasterVolume EffectGainVolume');
plugin('Spectrogram');
plugin('Tape', [{ filename: `rec`, max_track: 8 }]);
plugin('AudioOutput');
plugin('SerializeTrack', [{ filename: 'master', WORKSPACE_FOLDER }]);

plugin('Tempo');

autoSave(500);
