import { plugin } from '@/libs/audio';
import { MAX_VARIATION, STRING_CUTOFF_FORMAT } from './constants';

export function drum23(track: number) {
    plugin('Drum23', [{ track }]);
    plugin('Sequencer', [{ track }]);
    plugin('Distortion EffectDistortion2', [{ track }]);
    plugin('MMFilter EffectFilterMultiMode', [{ track, STRING_CUTOFF_FORMAT }]);
    plugin('SerializeTrack', [{ track, filename: `track_${track}`, MAX_VARIATION }]);
    plugin('Volume EffectGainVolume', [{ track }]);
    // unfortunately, recording every traks doesn't work well on rpi
    // plugin('Tape', [{ track, filename: `track_${track}` }]);
    plugin('RamTape', [{ track, filename: `track_${track}` }]);
}
