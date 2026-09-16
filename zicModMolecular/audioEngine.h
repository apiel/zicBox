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
    float masterVolume = 0.4f;

    AudioEngine() {}

    void renderAudioBlock(float* outputBuffer, size_t numFrames, const std::vector<SynthNode>& nodes, const std::vector<Connection>& connections)
    {
        std::fill(outputBuffer, outputBuffer + numFrames, 0.0f);
        if (isMuted) return;

        // Build node lookup map
        std::map<std::string, const SynthNode*> nodeMap;
        for (const auto& n : nodes) {
            nodeMap[n.id] = &n;
        }

        // Build incoming connections per node
        std::map<std::string, std::vector<std::string>> incomingConnections;
        for (const auto& conn : connections) {
            incomingConnections[conn.toId].push_back(conn.fromId);
        }

        // Process audio frame by frame
        for (size_t f = 0; f < numFrames; ++f) {
            float masterSample = 0.0f;

            for (const auto& node : nodes) {
                if (node.type == NodeType::OSC) {
                    float sample = renderOscillatorSample(node, f);

                    // If this OSC is connected to an FX node, pass through incoming connections
                    // If this OSC is marked as Audible (Master Feed), add to master mix
                    if (node.isAudible) {
                        masterSample += sample * node.paramB; // ParamB acts as gain
                    }
                } else if (node.type == NodeType::FX) {
                    // Collect audio input from connected OSC nodes
                    float fxInput = 0.0f;
                    auto it = incomingConnections.find(node.id);
                    if (it != incomingConnections.end()) {
                        for (const auto& srcId : it->second) {
                            auto srcIt = nodeMap.find(srcId);
                            if (srcIt != nodeMap.end() && srcIt->second->type == NodeType::OSC) {
                                fxInput += renderOscillatorSample(*srcIt->second, f);
                            }
                        }
                    }

                    float fxOutput = renderEffectSample(node, fxInput);

                    if (node.isAudible) {
                        masterSample += fxOutput;
                    }
                }
            }

            // Soft-clipping master output
            masterSample = std::tanh(masterSample * masterVolume);
            outputBuffer[f] = masterSample;
        }
    }

private:
    float renderOscillatorSample(const SynthNode& node, size_t frameIdx)
    {
        OscState& st = oscStates[node.id];
        float freq = std::clamp(node.frequency, 20.0f, 4000.0f);
        float phaseInc = freq / SAMPLE_RATE;

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

    float renderEffectSample(const SynthNode& node, float inputSample)
    {
        FxType type = static_cast<FxType>(node.subType);
        float paramA = std::clamp(node.paramA, 0.01f, 0.99f);
        float paramB = std::clamp(node.paramB, 0.01f, 0.99f);

        switch (type) {
        case FxType::FILTER_LP: {
            // Chamberlin State Variable Filter (Lowpass)
            FilterState& st = filterStates[node.id];
            float cutoffFreq = 100.0f + paramA * 8000.0f;
            float f = 2.0f * std::sin(M_PI * cutoffFreq / SAMPLE_RATE);
            float q = 1.0f - paramB * 0.9f;

            st.low += f * st.band;
            st.high = inputSample - st.low - q * st.band;
            st.band += f * st.high;

            return st.low;
        }
        case FxType::FILTER_HP: {
            FilterState& st = filterStates[node.id];
            float cutoffFreq = 50.0f + paramA * 5000.0f;
            float f = 2.0f * std::sin(M_PI * cutoffFreq / SAMPLE_RATE);
            float q = 1.0f - paramB * 0.9f;

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

            float feedback = paramB * 0.7f;
            db.buffer[db.writePos] = inputSample + delayed * feedback;
            db.writePos = (db.writePos + 1) % db.buffer.size();

            return inputSample + delayed * 0.5f;
        }
        case FxType::DISTORTION: {
            float drive = 1.0f + paramA * 20.0f;
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
