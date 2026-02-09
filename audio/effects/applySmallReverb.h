#pragma once

struct SmallReverb {
    float* buf;
    int size;
    int ptr = 0;
    void init(float* b, int s)
    {
        buf = b;
        size = s;
        for (int i = 0; i < s; i++)
            buf[i] = 0;
    }
    inline float process(float in, float feedback)
    {
        float out = buf[ptr];
        buf[ptr] = in + (out * feedback);
        if (++ptr >= size) ptr = 0;
        return out;
    }
};

struct AllPass {
    float* buf;
    int size;
    int ptr = 0;
    void init(float* b, int s)
    {
        buf = b;
        size = s;
        for (int i = 0; i < s; i++)
            buf[i] = 0;
    }
    inline float process(float in, float feedback)
    {
        float bufOut = buf[ptr];
        float out = bufOut - in;
        buf[ptr] = in + (bufOut * feedback);
        if (++ptr >= size) ptr = 0;
        return out;
    }
};

inline float applySmallReverb(float output, float rv, SmallReverb& d1, SmallReverb& d2, SmallReverb& d3, AllPass& ap)
{
    if (rv > 0.01f) {
        float feedback = 0.7f + (rv * 0.28f);

        float wet = d1.process(output, feedback);
        wet += d2.process(output, feedback - 0.05f);
        wet += d3.process(output, feedback - 0.1f);

        float reverbSignal = wet * 0.2f;

        // Process through the instance-specific AllPass
        reverbSignal = ap.process(reverbSignal, 0.5f);

        output += reverbSignal * rv;
    }
    return output;
}

float applySmallReverb2(float output, float rv, SmallReverb& d1, SmallReverb& d2, SmallReverb& d3)
{
    if (rv > 0.01f) {
        float wet = d1.process(output, 0.7f * rv);
        wet += d2.process(output, 0.75f * rv);
        wet += d3.process(output, 0.8f * rv);
        output += wet * 0.3f;
    }
    return output;
}