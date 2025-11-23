/** Description:
This header file defines a specialized function for generating a simple digital audio effect known as a "reverb." This effect simulates the spacious, echoing characteristics of sound bouncing off walls.

### Core Mechanism: Feedback Delay

The system operates using a core concept called a **feedback delay line**.

1.  **Memory Buffer:** A dedicated memory area, or buffer, is defined to store previous audio samples. This buffer is sized for 12,000 samples, which provides a maximum delay time of roughly 250 milliseconds (a quarter of a second).
2.  **Processing (The `tinyReverb` function):** The function handles audio processing one tiny moment (sample) at a time.
    *   It reads an old audio sample from the buffer. This is the source of the delay.
    *   It calculates a new value for that spot in the buffer by mixing the fresh incoming sound with a percentage of the sound just read back. This mixture is the "feedback," which causes the sound to repeat and gradually decay, creating the reverb tail.
    *   It continuously advances its position within the buffer, ensuring the system loops indefinitely to maintain the effect.

### Sound Control and Output

The function allows external control over the resulting sound quality:

*   **Input:** The immediate, fresh incoming audio sample.
*   **Reverb Intensity:** A parameter (ranging from 0% to 100%) that determines how much of the delayed signal is audible.
*   **Output Mix:** The final sound returned is a blend of the original, untouched signal ("dry") and the processed, delayed signal ("wet"). This blend is carefully weighted by the user-defined reverb intensity.

sha: e86f36e395ebfeab23d96154d2604de3c89fea7923e91fb336d629ad0529dde9 
*/
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