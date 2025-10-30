#pragma once

// Simple feedback delay reverb — about 12000 samples ≈ 250ms at 48kHz
#define TINY_REVERB_BUFFER float tinyReverbBuffer[12000] = { 0 };
float tinyReverb(float input, float& reverb, int& pos, float* buffer)
{

    // Read delayed sample
    float out = buffer[pos];

    // Feedback: feed the input plus some of the old signal
    // delay[pos] = input + out * reverbFeedback;
    buffer[pos] = input + out * (0.5f + reverb * 0.4f);

    // Increment and wrap buffer index
    pos = (pos + 1) % 12000;

    // Mix dry/wet with adjustable reverb parameter (0.0f – 1.0f)
    float dry = input * (1.0f - reverb);
    float wet = out * reverb;

    return dry + wet;
}

// float tinyStringReverb(float input)
// {
//     // Simple feedback delay reverb — about 12000 samples ≈ 250ms at 48kHz
//     static float delay[12000] = { 0 };
//     static int pos = 0;

//     // Read delayed sample
//     float out = delay[pos];

//     // Feedback: feed the input plus some of the old signal
//     // delay[pos] = input + out * reverbFeedback;
//     delay[pos] = input + out * (0.5f + reverb * 0.4f);

//     // Increment and wrap buffer index
//     pos = (pos + 1) % 12000;

//     // Mix dry/wet with adjustable reverb parameter (0.0f – 1.0f)
//     float dry = input * (1.0f - reverb);
//     float wet = out * reverb;

//     return dry + wet;
// }