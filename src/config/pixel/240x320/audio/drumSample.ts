import { plugin } from "@/libs/audio";
import { MAX_VARIATION } from "./constants";

export function drumSample(track: number) {
    plugin("DrumSample", [{ track }]);
    plugin("Sequencer", [{ track }]);
    plugin("SerializeTrack", [{ track, filename: `track_${track}`, MAX_VARIATION }]);
    plugin("EffectSampleRateReducer", [{ name: "SampleRateReducer", track }]);
    plugin("EffectVolumeClipping", [{ name: "Volume", track }]);
}
