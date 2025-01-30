import { plugin } from "@/libs/audio";
import { MAX_VARIATION, STRING_CUTOFF_FORMAT, WORKSPACE_FOLDER } from "./constants";

export function fm(track: number) {
    plugin("SynthFM", [{ name: "FM", track }]);
    plugin("Sequencer", [{ track }]);
    plugin("MMFilter EffectFilterMultiMode", [{ track, STRING_CUTOFF_FORMAT }]);
    plugin("Bitcrusher EffectSampleRateReducer", [{ track }]);
    plugin("Delay EffectDelay", [{ track }]);
    plugin("SerializeTrack", [{ track, filename: `track_${track}`, MAX_VARIATION, WORKSPACE_FOLDER }]);
    plugin("Volume EffectGainVolume", [{ track }]);
}