import { plugin } from "@/libs/audio";
import { MAX_VARIATION, STRING_CUTOFF_FORMAT } from "./constants";

export function fm(track: number) {
    plugin("SynthFM", [{ name: "FM", track }]);
    plugin("Sequencer", [{ track }]);
    plugin("EffectFilterMultiMode", [{ name: "MMFilter", track, STRING_CUTOFF_FORMAT }]);
    plugin("EffectSampleRateReducer", [{ name: "Bitcrusher", track }]);
    plugin("EffectDelay", [{ name: "Delay", track }]);
    plugin("SerializeTrack", [{ track, filename: `track_${track}`, MAX_VARIATION }]);
    plugin("EffectGainVolume", [{ name: "Volume", track }]);
}