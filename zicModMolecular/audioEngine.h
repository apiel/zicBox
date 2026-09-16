#pragma once

#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <map>
#include <string>
#include <vector>
#include "types.h"

class AudioEngine {
public:
    static constexpr float SAMPLE_RATE = 44100.0f;

    struct OscState {
        float phase = 0.0f;
        float currentFreq = 440.0f;
    };

    struct FilterState {
        float low = 0.0f;
        float band = 0.0f;
        float high = 0.0f;
    };

    struct DelayBuffer {
        std::vector<float> buffer;
        size_t writePos = 0;
        DelayBuffer() { buffer.resize(static_cast<size_t>(SAMPLE_RATE * 1.0f), 0.0f); }
    };

    std::map<std::string, OscState> oscStates;
    std::map<std::string, FilterState> filterStates;
    std::map<std::string, DelayBuffer> delayBuffers;

    bool isMuted = false;
    float masterVolume = 0.45f;

    AudioEngine() {}

    void renderAudioBlock(float* outputBuffer, size_t numFrames, std::vector<SynthNode>& nodes, const std::vector<Connection>& connections)
    {
        std::fill(outputBuffer, outputBuffer + numFrames, 0.0f);
        if (isMuted) return;

        // Build node lookup map
        std::map<std::string, SynthNode*> nodeMap;
        for (auto& n : nodes) {
            nodeMap[n.id] = &n;
        }

        // Process audio frame by frame
        for (size_t f = 0; f < numFrames; ++f) {
            float masterSample = 0.0f;

            // 1. Calculate Parameter Modulations per Node
            std::map<std::string, float> freqMods;
            std::map<std::string, float> gainMods;
            std::map<std::string, float> cutoffMods;

            for (const auto& conn : connections) {
                if (conn.target == ModTarget::NONE) continue; // Audio-only forwarding (No parameter modulation)

                auto srcIt = nodeMap.find(conn.fromId);
                auto dstIt = nodeMap.find(conn.toId);
                if (srcIt != nodeMap.end() && dstIt != nodeMap.end()) {
                    float modSig = renderOscillatorSample(*srcIt->second, f);

                    switch (conn.target) {
                    case ModTarget::FREQUENCY:
                        freqMods[conn.toId] += modSig * conn.depth * 500.0f;
                        break;
                    case ModTarget::GAIN:
                        gainMods[conn.toId] += modSig * conn.depth;
                        break;
                    case ModTarget::CUTOFF:
                        cutoffMods[conn.toId] += modSig * conn.depth * 0.5f;
                        break;
                    default:
                        break;
                    }
                }
            }

            // 2. Render Node Outputs & Apply Catalyst Disturbances
            for (auto& node : nodes) {
                // Decay collision disturbance burst
                float disturbanceOffset = node.disturbance * 120.0f * std::sin(f * 0.25f);
                if (f == numFrames - 1 && node.disturbance > 0.0f) {
                    node.disturbance -= 0.04f;
                    if (node.disturbance < 0.0f) node.disturbance = 0.0f;
                }

                if (node.type == NodeType::OSC) {
                    float baseFreq = node.frequency + disturbanceOffset + freqMods[node.id];
                    float sample = renderOscillatorSampleWithFreq(node, baseFreq);

                    float gain = std::clamp(node.paramB + gainMods[node.id] + (node.disturbance * 0.4f), 0.0f, 1.0f);

                    if (node.isAudible) {
                        masterSample += sample * gain;
                    }
                } else if (node.type == NodeType::FX) {
                    float fxInput = 0.0f;
                    // Find nodes (OSC or upstream FX) feeding into this FX
                    for (const auto& conn : connections) {
                        if (conn.toId == node.id) {
                            auto srcIt = nodeMap.find(conn.fromId);
                            if (srcIt != nodeMap.end()) {
                                if (srcIt->second->type == NodeType::OSC) {
                                    float baseFreq = srcIt->second->frequency + freqMods[srcIt->second->id];
                                    fxInput += renderOscillatorSampleWithFreq(*srcIt->second, baseFreq);
                                }
                            }
                        }
                    }

                    float modCutoff = std::clamp(node.paramA + cutoffMods[node.id] + (node.disturbance * 0.3f), 0.01f, 0.99f);
                    float fxOutput = renderEffectSampleWithMod(node, fxInput, modCutoff);

                    if (node.isAudible) {
                        masterSample += fxOutput;
                    }
                }
            }

            // Soft clipping
            masterSample = std::tanh(masterSample * masterVolume);
            outputBuffer[f] = masterSample;
        }
    }

private:
    float renderOscillatorSample(const SynthNode& node, size_t frameIdx)
    {
        return renderOscillatorSampleWithFreq(node, node.frequency);
    }

    float renderOscillatorSampleWithFreq(const SynthNode& node, float freq)
    {
        OscState& st = oscStates[node.id];
        float clampedFreq = std::clamp(freq, 20.0f, 4000.0f);
        float phaseInc = clampedFreq / SAMPLE_RATE;

        st.phase += phaseInc;
        if (st.phase >= 1.0f) st.phase -= 1.0f;

        float sample = 0.0f;
        OscType type = static_cast<OscType>(node.subType);

        switch (type) {
        case OscType::SINE:
            sample = std::sin(st.phase * 2.0f * M_PI);
            break;
        case OscType::SQUARE:
            sample = (st.phase < 0.5f) ? 1.0f : -1.0f;
            break;
        case OscType::SAW:
            sample = 2.0f * st.phase - 1.0f;
            break;
        case OscType::TRIANGLE:
            sample = 4.0f * std::abs(st.phase - 0.5f) - 1.0f;
            break;
        case OscType::NOISE:
            sample = (static_cast<float>(rand()) / RAND_MAX) * 2.0f - 1.0f;
            break;
        }

        return sample;
    }

    float renderEffectSampleWithMod(const SynthNode& node, float inputSample, float cutoffParam)
    {
        FxType type = static_cast<FxType>(node.subType);
        float paramA = cutoffParam;
        float paramB = std::clamp(node.paramB, 0.01f, 0.99f);

        switch (type) {
        case FxType::FILTER_LP: {
            FilterState& st = filterStates[node.id];
            float cutoffFreq = 80.0f + paramA * 9000.0f;
            float f = 2.0f * std::sin(M_PI * cutoffFreq / SAMPLE_RATE);
            float q = 1.0f - paramB * 0.92f;

            st.low += f * st.band;
            st.high = inputSample - st.low - q * st.band;
            st.band += f * st.high;

            return st.low;
        }
        case FxType::FILTER_HP: {
            FilterState& st = filterStates[node.id];
            float cutoffFreq = 40.0f + paramA * 6000.0f;
            float f = 2.0f * std::sin(M_PI * cutoffFreq / SAMPLE_RATE);
            float q = 1.0f - paramB * 0.92f;

            st.low += f * st.band;
            st.high = inputSample - st.low - q * st.band;
            st.band += f * st.high;

            return st.high;
        }
        case FxType::DELAY: {
            DelayBuffer& db = delayBuffers[node.id];
            size_t delaySamples = static_cast<size_t>((0.05f + paramA * 0.5f) * SAMPLE_RATE);
            if (delaySamples >= db.buffer.size()) delaySamples = db.buffer.size() - 1;

            size_t readPos = (db.writePos + db.buffer.size() - delaySamples) % db.buffer.size();
            float delayed = db.buffer[readPos];

            float feedback = paramB * 0.75f;
            db.buffer[db.writePos] = inputSample + delayed * feedback;
            db.writePos = (db.writePos + 1) % db.buffer.size();

            return inputSample + delayed * 0.5f;
        }
        case FxType::DISTORTION: {
            float drive = 1.0f + paramA * 25.0f;
            float raw = std::tanh(inputSample * drive);
            return raw * paramB;
        }
        case FxType::BITCRUSHER: {
            float bits = 2.0f + paramA * 14.0f;
            float steps = std::pow(2.0f, bits);
            float crushed = std::round(inputSample * steps) / steps;
            return crushed * paramB;
        }
        }
        return inputSample;
    }
};
