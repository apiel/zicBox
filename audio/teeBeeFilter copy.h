#pragma once

// standard-library includes:
#include <stdlib.h> // for the NULL macro

namespace rosic {

class OnePoleFilter {

public:
    /** This is an enumeration of the available filter modes. */
    enum modes {
        BYPASS = 0,
        LOWPASS,
        HIGHPASS,
        LOWSHELV,
        HIGHSHELV,
        ALLPASS
    };
    // \todo (maybe): let the user choose between LP/HP versions obtained via bilinear trafo and
    // impulse invariant trafo

    //---------------------------------------------------------------------------------------------
    // construction/destruction:

    OnePoleFilter::OnePoleFilter()
    {
        shelvingGain = 1.0;
        setSampleRate(44100.0); // sampleRate = 44100 Hz by default
        setMode(0); // bypass by default
        setCutoff(20000.0); // cutoff = 20000 Hz by default
        reset(); // reset memorized samples to zero
    }

    //-------------------------------------------------------------------------------------------------
    // parameter settings:

    void OnePoleFilter::setSampleRate(double newSampleRate)
    {
        if (newSampleRate > 0.0)
            sampleRate = newSampleRate;
        sampleRateRec = 1.0 / sampleRate;

        calcCoeffs();
        return;
    }

    void OnePoleFilter::setMode(int newMode)
    {
        mode = newMode; // 0:bypass, 1:Low Pass, 2:High Pass
        calcCoeffs();
    }

    void OnePoleFilter::setCutoff(double newCutoff)
    {
        if ((newCutoff > 0.0) && (newCutoff <= 20000.0))
            cutoff = newCutoff;
        else
            cutoff = 20000.0;

        calcCoeffs();
        return;
    }

    void OnePoleFilter::setShelvingGain(double newGain)
    {
        if (newGain > 0.0) {
            shelvingGain = newGain;
            calcCoeffs();
        } else
            DEBUG_BREAK; // this is a linear gain factor and must be >= 0.0
    }

    void OnePoleFilter::setShelvingGainInDecibels(double newGain)
    {
        setShelvingGain(dB2amp(newGain));
    }

    void OnePoleFilter::setCoefficients(double newB0, double newB1, double newA1)
    {
        b0 = newB0;
        b1 = newB1;
        a1 = newA1;
    }

    void OnePoleFilter::setInternalState(double newX1, double newY1)
    {
        x1 = newX1;
        y1 = newY1;
    }

    //-------------------------------------------------------------------------------------------------
    //others:

    void OnePoleFilter::calcCoeffs()
    {
        switch (mode) {
        case LOWPASS: {
            // formula from dspguide:
            double x = exp(-2.0 * PI * cutoff * sampleRateRec);
            b0 = 1 - x;
            b1 = 0.0;
            a1 = x;
        } break;
        case HIGHPASS: {
            // formula from dspguide:
            double x = exp(-2.0 * PI * cutoff * sampleRateRec);
            b0 = 0.5 * (1 + x);
            b1 = -0.5 * (1 + x);
            a1 = x;
        } break;
        case LOWSHELV: {
            // formula from DAFX:
            double c = 0.5 * (shelvingGain - 1.0);
            double t = tan(PI * cutoff * sampleRateRec);
            double a;
            if (shelvingGain >= 1.0)
                a = (t - 1.0) / (t + 1.0);
            else
                a = (t - shelvingGain) / (t + shelvingGain);

            b0 = 1.0 + c + c * a;
            b1 = c + c * a + a;
            a1 = -a;
        } break;
        case HIGHSHELV: {
            // formula from DAFX:
            double c = 0.5 * (shelvingGain - 1.0);
            double t = tan(PI * cutoff * sampleRateRec);
            double a;
            if (shelvingGain >= 1.0)
                a = (t - 1.0) / (t + 1.0);
            else
                a = (shelvingGain * t - 1.0) / (shelvingGain * t + 1.0);

            b0 = 1.0 + c - c * a;
            b1 = a + c * a - c;
            a1 = -a;
        } break;

        case ALLPASS: {
            // formula from DAFX:
            double t = tan(PI * cutoff * sampleRateRec);
            double x = (t - 1.0) / (t + 1.0);

            b0 = x;
            b1 = 1.0;
            a1 = -x;
        } break;

        default: // bypass
        {
            b0 = 1.0;
            b1 = 0.0;
            a1 = 0.0;
        } break;
        }
    }

    void OnePoleFilter::reset()
    {
        x1 = 0.0;
        y1 = 0.0;
    }

    /** Sets the sample-rate. */
    void setSampleRate(double newSampleRate);

    /** Chooses the filter mode. See the enumeration for available modes. */
    void setMode(int newMode);

    /** Sets the cutoff-frequency for this filter. */
    void setCutoff(double newCutoff);

    /** This will set the time constant 'tau' for the case, when lowpass mode is chosen. This is
    the time, it takes for the impulse response to die away to 1/e = 0.368... or equivalently, the
    time it takes for the step response to raise to 1-1/e = 0.632... */
    void setLowpassTimeConstant(double newTimeConstant) { setCutoff(1.0 / (2 * PI * newTimeConstant)); }

    /** Sets the gain factor for the shelving modes (this is not in decibels). */
    void setShelvingGain(double newGain);

    /** Sets the gain for the shelving modes in decibels. */
    void setShelvingGainInDecibels(double newGain);

    /** Sets the filter coefficients manually. */
    void setCoefficients(double newB0, double newB1, double newA1);

    /** Sets up the internal state variables for both channels. */
    void setInternalState(double newX1, double newY1);

    //---------------------------------------------------------------------------------------------
    // inquiry

    /** Returns the cutoff-frequency. */
    double getCutoff() const { return cutoff; }

    //---------------------------------------------------------------------------------------------
    // audio processing:

    /** Calculates a single filtered output-sample. */
    INLINE double getSample(double in);

    //---------------------------------------------------------------------------------------------
    // others:

    /** Resets the internal buffers (for the \f$ x[n-1], y[n-1] \f$-samples) to zero. */
    void reset();

    //=============================================================================================

protected:
    // buffering:
    double x1, y1;

    // filter coefficients:
    double b0; // feedforward coeffs
    double b1;
    double a1; // feedback coeff

    // filter parameters:
    double cutoff;
    double shelvingGain;
    int mode;

    double sampleRate;
    double sampleRateRec; // reciprocal of the sampleRate

    // internal functions:
    void calcCoeffs(); // calculates filter coefficients from filter parameters
};

//-----------------------------------------------------------------------------------------------
// inlined functions:

INLINE double OnePoleFilter::getSample(double in)
{
    // calculate the output sample:
    y1 = b0 * in + b1 * x1 + a1 * y1 + TINY;

    // update the buffer variables:
    x1 = in;

    return y1;
}

class TeeBeeFilter {

public:
    /** Enumeration of the available filter modes. */
    enum modes {
        FLAT = 0,
        LP_6,
        LP_12,
        LP_18,
        LP_24,
        HP_6,
        HP_12,
        HP_18,
        HP_24,
        BP_12_12,
        BP_6_18,
        BP_18_6,
        BP_6_12,
        BP_12_6,
        BP_6_6,
        TB_303, // ala mystran & kunn (page 40 in the kvr-thread)

        NUM_MODES
    };

    TeeBeeFilter()
    {
        cutoff = 1000.0;
        drive = 0.0;
        driveFactor = 1.0;
        resonanceRaw = 0.0;
        resonanceSkewed = 0.0;
        g = 1.0;
        sampleRate = 44100.0;
        twoPiOverSampleRate = 2.0 * PI / sampleRate;

        feedbackHighpass.setMode(OnePoleFilter::HIGHPASS);
        feedbackHighpass.setCutoff(150.0);

        //setMode(LP_18);
        setMode(TB_303);
        calculateCoefficientsExact();
        reset();
    }

    void setSampleRate(double newSampleRate)
    {
        if (newSampleRate > 0.0)
            sampleRate = newSampleRate;
        twoPiOverSampleRate = 2.0 * PI / sampleRate;
        feedbackHighpass.setSampleRate(newSampleRate);
        calculateCoefficientsExact();
    }

    void setDrive(double newDrive)
    {
        drive = newDrive;
        driveFactor = dB2amp(drive);
    }

    void setMode(int newMode)
    {
        if (newMode >= 0 && newMode < NUM_MODES) {
            mode = newMode;
            switch (mode) {
            case FLAT:
                c0 = 1.0;
                c1 = 0.0;
                c2 = 0.0;
                c3 = 0.0;
                c4 = 0.0;
                break;
            case LP_6:
                c0 = 0.0;
                c1 = 1.0;
                c2 = 0.0;
                c3 = 0.0;
                c4 = 0.0;
                break;
            case LP_12:
                c0 = 0.0;
                c1 = 0.0;
                c2 = 1.0;
                c3 = 0.0;
                c4 = 0.0;
                break;
            case LP_18:
                c0 = 0.0;
                c1 = 0.0;
                c2 = 0.0;
                c3 = 1.0;
                c4 = 0.0;
                break;
            case LP_24:
                c0 = 0.0;
                c1 = 0.0;
                c2 = 0.0;
                c3 = 0.0;
                c4 = 1.0;
                break;
            case HP_6:
                c0 = 1.0;
                c1 = -1.0;
                c2 = 0.0;
                c3 = 0.0;
                c4 = 0.0;
                break;
            case HP_12:
                c0 = 1.0;
                c1 = -2.0;
                c2 = 1.0;
                c3 = 0.0;
                c4 = 0.0;
                break;
            case HP_18:
                c0 = 1.0;
                c1 = -3.0;
                c2 = 3.0;
                c3 = -1.0;
                c4 = 0.0;
                break;
            case HP_24:
                c0 = 1.0;
                c1 = -4.0;
                c2 = 6.0;
                c3 = -4.0;
                c4 = 1.0;
                break;
            case BP_12_12:
                c0 = 0.0;
                c1 = 0.0;
                c2 = 1.0;
                c3 = -2.0;
                c4 = 1.0;
                break;
            case BP_6_18:
                c0 = 0.0;
                c1 = 0.0;
                c2 = 0.0;
                c3 = 1.0;
                c4 = -1.0;
                break;
            case BP_18_6:
                c0 = 0.0;
                c1 = 1.0;
                c2 = -3.0;
                c3 = 3.0;
                c4 = -1.0;
                break;
            case BP_6_12:
                c0 = 0.0;
                c1 = 0.0;
                c2 = 1.0;
                c3 = -1.0;
                c4 = 0.0;
                break;
            case BP_12_6:
                c0 = 0.0;
                c1 = 1.0;
                c2 = -2.0;
                c3 = 1.0;
                c4 = 0.0;
                break;
            case BP_6_6:
                c0 = 0.0;
                c1 = 1.0;
                c2 = -1.0;
                c3 = 0.0;
                c4 = 0.0;
                break;
            default:
                c0 = 1.0;
                c1 = 0.0;
                c2 = 0.0;
                c3 = 0.0;
                c4 = 0.0; // flat
            }
        }
        calculateCoefficientsApprox4();
    }

    //-------------------------------------------------------------------------------------------------
    // others:

    void reset()
    {
        feedbackHighpass.reset();
        y1 = 0.0;
        y2 = 0.0;
        y3 = 0.0;
        y4 = 0.0;
    }

    /** Sets the cutoff frequency for this filter - the actual coefficient calculation may be
    supressed by passing 'false' as second parameter, in this case, it should be triggered
    manually later by calling calculateCoefficients. */
    INLINE void setCutoff(double newCutoff, bool updateCoefficients = true);

    /** Sets the resonance in percent where 100% is self oscillation. */
    INLINE void setResonance(double newResonance, bool updateCoefficients = true);

    /** Sets the cutoff frequency for the highpass filter in the feedback path. */
    void setFeedbackHighpassCutoff(double newCutoff) { feedbackHighpass.setCutoff(newCutoff); }

    //---------------------------------------------------------------------------------------------
    // inquiry:

    /** Returns the cutoff frequency of this filter. */
    double getCutoff() const { return cutoff; }

    /** Returns the resonance parameter of this filter. */
    double getResonance() const { return 100.0 * resonanceRaw; }

    /** Returns the drive parameter in decibels. */
    double getDrive() const { return drive; }

    /** Returns the slected filter mode. */
    int getMode() const { return mode; }

    /** Returns the cutoff frequency for the highpass filter in the feedback path. */
    double getFeedbackHighpassCutoff() const { return feedbackHighpass.getCutoff(); }

    //---------------------------------------------------------------------------------------------
    // audio processing:

    /** Calculates one output sample at a time. */
    INLINE double getSample(double in);

    //---------------------------------------------------------------------------------------------
    // others:

    /** Causes the filter to re-calculate the coeffiecients via the exact formulas. */
    INLINE void calculateCoefficientsExact();

    /** Causes the filter to re-calculate the coeffiecients using an approximation that is valid
    for normalized radian cutoff frequencies up to pi/4. */
    INLINE void calculateCoefficientsApprox4();

    /** Implements the waveshaping nonlinearity between the stages. */
    INLINE double shape(double x);

    //=============================================================================================

protected:
    double b0, a1; // coefficients for the first order sections
    double y1, y2, y3, y4; // output signals of the 4 filter stages
    double c0, c1, c2, c3, c4; // coefficients for combining various ouput stages
    double k; // feedback factor in the loop
    double g; // output gain
    double driveFactor; // filter drive as raw factor
    double cutoff; // cutoff frequency
    double drive; // filter drive in decibels
    double resonanceRaw; // resonance parameter (normalized to 0...1)
    double resonanceSkewed; // mapped resonance parameter to make it behave more musical
    double sampleRate; // the sample rate in Hz
    double twoPiOverSampleRate; // 2*PI/sampleRate
    int mode; // the selected filter-mode

    OnePoleFilter feedbackHighpass;
};

INLINE void TeeBeeFilter::setCutoff(double newCutoff, bool updateCoefficients)
{
    if (newCutoff != cutoff) {
        if (newCutoff < 200.0) // an absolute floor for the cutoff frequency - tweakable
            cutoff = 200.0;
        else if (newCutoff > 20000.0)
            cutoff = 20000.0;
        else
            cutoff = newCutoff;

        if (updateCoefficients == true)
            calculateCoefficientsApprox4();
    }
}

INLINE void TeeBeeFilter::setResonance(double newResonance, bool updateCoefficients)
{
    resonanceRaw = 0.01 * newResonance;
    resonanceSkewed = (1.0 - exp(-3.0 * resonanceRaw)) / (1.0 - exp(-3.0));
    if (updateCoefficients == true)
        calculateCoefficientsApprox4();
}

INLINE void TeeBeeFilter::calculateCoefficientsExact()
{
    // calculate intermediate variables:
    double wc = twoPiOverSampleRate * cutoff;
    double s, c;
    sinCos(wc, &s, &c); // c = cos(wc); s = sin(wc);
    double t = tan(0.25 * (wc - PI));
    double r = resonanceSkewed;

    // calculate filter a1-coefficient tuned such the resonance frequency is just right:
    double a1_fullRes = t / (s - c * t);

    // calculate filter a1-coefficient as if there were no resonance:
    double x = exp(-wc);
    double a1_noRes = -x;

    // use a weighted sum between the resonance-tuned and no-resonance coefficient:
    a1 = r * a1_fullRes + (1.0 - r) * a1_noRes;

    // calculate the b0-coefficient from the condition that each stage should be a leaky
    // integrator:
    b0 = 1.0 + a1;

    // calculate feedback factor by dividing the resonance parameter by the magnitude at the
    // resonant frequency:
    double gsq = b0 * b0 / (1.0 + a1 * a1 + 2.0 * a1 * c);
    k = r / (gsq * gsq);

    if (mode == TB_303)
        k *= (17.0 / 4.0);
}

INLINE void TeeBeeFilter::calculateCoefficientsApprox4()
{
    // calculate intermediate variables:
    double wc = twoPiOverSampleRate * cutoff;
    double wc2 = wc * wc;
    double r = resonanceSkewed;
    double tmp;

    // compute the filter coefficient via a 12th order polynomial approximation (polynomial
    // evaluation is done with a Horner-rule alike scheme with nested quadratic factors in the hope
    // for potentially better parallelization compared to Horner's rule as is):
    const double pa12 = -1.341281325101042e-02;
    const double pa11 = 8.168739417977708e-02;
    const double pa10 = -2.365036766021623e-01;
    const double pa09 = 4.439739664918068e-01;
    const double pa08 = -6.297350825423579e-01;
    const double pa07 = 7.529691648678890e-01;
    const double pa06 = -8.249882473764324e-01;
    const double pa05 = 8.736418933533319e-01;
    const double pa04 = -9.164580250284832e-01;
    const double pa03 = 9.583192455599817e-01;
    const double pa02 = -9.999994950291231e-01;
    const double pa01 = 9.999999927726119e-01;
    const double pa00 = -9.999999999857464e-01;
    tmp = wc2 * pa12 + pa11 * wc + pa10;
    tmp = wc2 * tmp + pa09 * wc + pa08;
    tmp = wc2 * tmp + pa07 * wc + pa06;
    tmp = wc2 * tmp + pa05 * wc + pa04;
    tmp = wc2 * tmp + pa03 * wc + pa02;
    a1 = wc2 * tmp + pa01 * wc + pa00;
    b0 = 1.0 + a1;

    // compute the scale factor for the resonance parameter (the factor to obtain k from r) via an
    // 8th order polynomial approximation:
    const double pr8 = -4.554677015609929e-05;
    const double pr7 = -2.022131730719448e-05;
    const double pr6 = 2.784706718370008e-03;
    const double pr5 = 2.079921151733780e-03;
    const double pr4 = -8.333236384240325e-02;
    const double pr3 = -1.666668203490468e-01;
    const double pr2 = 1.000000012124230e+00;
    const double pr1 = 3.999999999650040e+00;
    const double pr0 = 4.000000000000113e+00;
    tmp = wc2 * pr8 + pr7 * wc + pr6;
    tmp = wc2 * tmp + pr5 * wc + pr4;
    tmp = wc2 * tmp + pr3 * wc + pr2;
    tmp = wc2 * tmp + pr1 * wc + pr0; // this is now the scale factor
    k = r * tmp;
    g = 1.0;

    if (mode == TB_303) {
        double fx = wc * ONE_OVER_SQRT2 / (2 * PI);
        b0 = (0.00045522346 + 6.1922189 * fx) / (1.0 + 12.358354 * fx + 4.4156345 * (fx * fx));
        k = fx * (fx * (fx * (fx * (fx * (fx + 7198.6997) - 5837.7917) - 476.47308) + 614.95611) + 213.87126) + 16.998792;
        g = k * 0.058823529411764705882352941176471; // 17 reciprocal
        g = (g - 1.0) * r + 1.0; // r is 0 to 1.0
        g = (g * (1.0 + r));
        k = k * r; // k is ready now
    }
}

INLINE double TeeBeeFilter::shape(double x)
{
    const double r6 = 1.0 / 6.0;
    x = clip(x, -SQRT2, SQRT2);
    return x - r6 * x * x * x;
}

INLINE double TeeBeeFilter::getSample(double in)
{
    double y0;

    if (mode == TB_303) {
        y0 = in - feedbackHighpass.getSample(k * y4);
        y1 += 2 * b0 * (y0 - y1 + y2);
        y2 += b0 * (y1 - 2 * y2 + y3);
        y3 += b0 * (y2 - 2 * y3 + y4);
        y4 += b0 * (y3 - 2 * y4);
        return 2 * g * y4;
    }

    y0 = 0.125 * driveFactor * in - feedbackHighpass.getSample(k * y4);
    y1 = y0 + a1 * (y0 - y1);
    y2 = y1 + a1 * (y1 - y2);
    y3 = y2 + a1 * (y2 - y3);
    y4 = y3 + a1 * (y3 - y4);

    return 8.0 * (c0 * y0 + c1 * y1 + c2 * y2 + c3 * y3 + c4 * y4);
    ;
}

}
