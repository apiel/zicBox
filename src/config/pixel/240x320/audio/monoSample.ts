import { plugin } from "@/libs/audio";
import { MAX_VARIATION } from "./constants";

export function monoSample(track: number) {
    plugin("MonoSample", [{ track }]);
    plugin("Sequencer", [{ track }]);
    plugin("SerializeTrack", [{ track, filename: `track_${track}`, MAX_VARIATION }]);
    plugin("SampleRateReducer EffectSampleRateReducer", [{ track }]);
    plugin("Volume EffectVolumeClipping", [{ track }]);
}
