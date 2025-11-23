/** Description:
This C++ header defines a foundational class called `BandEq`, which acts as a blueprint for a specialized digital audio filter. Its primary purpose is to implement a **Bandpass Filter**, allowing only a specific range of frequencies in an audio signal to pass through, while blocking lower and higher frequencies.

### How the System Works

The filter operates using mathematical processing on incoming audio samples, a standard technique in digital signal processing (DSP).

1.  **Core Filter Unit:** The system relies on a simple, standardized structure often called a "Biquad" filter. This unit performs mathematical calculations using the current audio input and a few previously processed samples to determine the new output, thereby applying the filtering effect.

2.  **Creating the Band:** To isolate a frequency band, the `BandEq` class chains two separate filter operations together:
    *   A **Highpass Filter** cuts off all frequencies below a defined minimum limit.
    *   A **Lowpass Filter** cuts off all frequencies above a defined maximum limit.
    The signal remaining after passing through both filters is the isolated band.

3.  **Key Settings:** The filter’s behavior is controlled by three main parameters:
    *   **Sample Rate:** The speed at which audio samples are processed (critical for digital accuracy).
    *   **Center Frequency:** The middle point of the desired frequency band.
    *   **Range (Hz):** The width of the band around the center frequency.

4.  **Dynamic Coefficients:** Whenever the center frequency or range is changed, the system instantly recalculates a set of complex mathematical values called "coefficients." These coefficients are crucial, as they define the precise mathematical recipe used by the internal filters to achieve the required highpass and lowpass cutoffs.

In summary, the class provides a streamlined way to take raw audio data, dynamically apply two cascaded digital filters based on user-defined frequency parameters, and output only the desired frequency range.

sha: fccfa14326820e24e65e6bb9795254fcec5701cce227f6e459dfd62a80f02972 
*/
#pragma once

#include <algorithm>
#include <math.h>
#include <cstdint>

class BandEq {
protected:
    uint64_t sampleRate;
    float centerFreq = 1000.0f;
    float rangeHz = 2000.0f;

    struct BQ {
        float b0 = 1, b1 = 0, b2 = 0, a1 = 0, a2 = 0;
        float x1 = 0, x2 = 0, y1 = 0, y2 = 0;
        float process(float x)
        {
            float y = b0 * x + b1 * x1 + b2 * x2 - a1 * y1 - a2 * y2;
            x2 = x1;
            x1 = x;
            y2 = y1;
            y1 = y;
            return y;
        }
    } lowpass, highpass;

    void updateCoeffs()
    {
        // Compute min/max freq from centerFreq and rangeHz
        float minFreq = std::max(20.0f, centerFreq - rangeHz * 0.5f);
        float maxFreq = std::min(sampleRate * 0.5f, centerFreq + rangeHz * 0.5f);

        setHighpass(highpass, minFreq, sampleRate);
        setLowpass(lowpass, maxFreq, sampleRate);
    }

    void setLowpass(BQ& bq, float freq, float fs)
    {
        float w0 = 2.0f * M_PI * freq / fs;
        float cosw0 = cosf(w0);
        float sinw0 = sinf(w0);
        float alpha = sinw0 / (2.0f * 0.707f); // Q=0.707

        float b0n = (1 - cosw0) / 2;
        float b1n = 1 - cosw0;
        float b2n = (1 - cosw0) / 2;
        float a0n = 1 + alpha;
        float a1n = -2 * cosw0;
        float a2n = 1 - alpha;

        bq.b0 = b0n / a0n;
        bq.b1 = b1n / a0n;
        bq.b2 = b2n / a0n;
        bq.a1 = a1n / a0n;
        bq.a2 = a2n / a0n;
    }

    void setHighpass(BQ& bq, float freq, float fs)
    {
        float w0 = 2.0f * M_PI * freq / fs;
        float cosw0 = cosf(w0);
        float sinw0 = sinf(w0);
        float alpha = sinw0 / (2.0f * 0.707f);

        float b0n = (1 + cosw0) / 2;
        float b1n = -(1 + cosw0);
        float b2n = (1 + cosw0) / 2;
        float a0n = 1 + alpha;
        float a1n = -2 * cosw0;
        float a2n = 1 - alpha;

        bq.b0 = b0n / a0n;
        bq.b1 = b1n / a0n;
        bq.b2 = b2n / a0n;
        bq.a1 = a1n / a0n;
        bq.a2 = a2n / a0n;
    }

public:
    BandEq(uint64_t sampleRate) : sampleRate(sampleRate) {
        updateCoeffs();
    }

    void setCenterFreq(float value) {
        centerFreq = value;
        updateCoeffs();
    }

    void setRangeHz(float value) {
        rangeHz = value;
        updateCoeffs();
    }

    float process(float input) {
        float band = highpass.process(input);
        band = lowpass.process(band);
        return band;
    }
};