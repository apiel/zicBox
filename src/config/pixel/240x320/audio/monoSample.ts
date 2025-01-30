import { plugin } from "@/libs/audio";
import { MAX_VARIATION, STRING_CUTOFF_FORMAT, WORKSPACE_FOLDER } from "./constants";

export function monoSample(track: number) {
    plugin("MonoSample", [{ track }]);
    plugin("Sequencer", [{ track }]);
    plugin("SerializeTrack", [{ track, filename: `track_${track}`, MAX_VARIATION, WORKSPACE_FOLDER }]);
    plugin('MMFilter EffectFilterMultiMode', [{ track, STRING_CUTOFF_FORMAT }]);
    plugin("SampleRateReducer EffectSampleRateReducer", [{ track }]);
    plugin("Volume EffectVolumeClipping", [{ track }]);
}
