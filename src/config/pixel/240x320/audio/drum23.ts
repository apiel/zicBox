import { plugin } from '@/libs/audio';
import { MAX_VARIATION, STRING_CUTOFF_FORMAT, WORKSPACE_FOLDER } from './constants';

export function drum23(track: number) {
    plugin('Drum23', [{ track }]);
    plugin('Sequencer', [{ track }]);
    plugin('Distortion EffectDistortion2', [{ track }]);
    plugin('MMFilter EffectFilterMultiMode', [{ track, STRING_CUTOFF_FORMAT }]);
    plugin('SerializeTrack', [{ track, filename: `track_${track}`, MAX_VARIATION, WORKSPACE_FOLDER }]);
    plugin('Volume EffectGainVolume', [{ track }]);
}
