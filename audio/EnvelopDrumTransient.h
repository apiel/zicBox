/** Description:
This C++ header file serves as a specialized library for defining and managing the amplitude curves, or "envelopes," used to model the sharp initial impact of percussive sounds, like drums (known as the transient).

The file starts by defining a collection of mathematical "recipes" within a dedicated group. Each recipe is a function that takes a time value (from 0 to 1) and returns a corresponding intensity or volume level. These include simple shapes like a straight line (`linear`) and more dynamic, complex curves such as a rapid spike (`impulse`), a smooth oscillation (`sineBlip`), or patterns simulating electronic clicks (`pulseTrain`). These shapes are crucial for giving synthesized instruments their specific character.

All these distinct shapes are bundled into a single organized list (`transientShapes`).

The core component is the `EnvelopDrumTransient` class. This object inherits basic functionality from a general envelope system and specializes it for drum sounds. Its primary job is to provide access to the bundled transient shapes.

The class allows the user to set the maximum allowed output volume (`setMaxAmp`). When the envelope is used, the mathematical value produced by the chosen shape (which runs from 0 to 1) is immediately multiplied by this maximum amplitude, ensuring the final output is scaled correctly for the audio system. This design offers a flexible and ready-to-use structure for creating realistic and varied drum transients in sound synthesis applications.

sha: 274b0e0c53069abdd362c53b0755c2e17396383dac9fea69e0e706c01ae8272e 
*/
#pragma once

#include "EnvelopMorph.h"
#include "helpers/math.h"

#include <cmath>

namespace EnvelopeShapes {

inline float linear(float t) { return t; }
inline float impulse(float t) { return (t < 0.05f) ? 1.0f - (t * 20.0f) : 0.0f; }
inline float punchy(float t) { return t * t * (1.0f - t); }
inline float triangle(float t) { return 1.0f - std::abs(2.0f * t - 1.0f); }
inline float overshoot(float t) { return fastSin(t * 3.1415f) * (1.0f - t); }
inline float snareTail(float t) { return fastExpNeg(10.0f * t); }
inline float sineBlip(float t) { return fastSin(t * 3.1415f); }

inline float pulseTrain(float t)
{
    float decay = fastExpNeg(10.0f * t);
    float pulse = (std::fmod(t * 20.0f, 1.0f) < 0.5f) ? 1.0f : 0.0f; // 10 pulses
    return decay * pulse;
}
inline float pulseTrainDecay(float t)
{
    return pulseTrain(1.0f - t);
}

constexpr std::array<EnvelopeShapeFunc, 11> transientShapes = {
    exp6, exp3, linear, impulse, punchy, triangle, overshoot, snareTail, sineBlip, pulseTrainDecay, pulseTrain
};

} // namespace EnvelopeShapes

class EnvelopDrumTransient : public EnvelopMorph {
public:
    EnvelopDrumTransient()
        : maxAmp(0.7f)
    {
        shapeFuncs = EnvelopeShapes::transientShapes.data();
        shapeCount = static_cast<int>(EnvelopeShapes::transientShapes.size());
    }

    void setMaxAmp(float amp)
    {
        maxAmp = std::clamp(amp, 0.0f, 2.0f);
    }

protected:
    float maxAmp;

    float transformTime(float t) const override
    {
        return t; // Transient uses natural time (0 → 1)
    }

    float scaleOutput(float value) const override
    {
        return maxAmp * value;
    }
};
