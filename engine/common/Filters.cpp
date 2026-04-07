#include "common/Filters.h"
#include "common/DSPConstants.h"
#include <cmath>

namespace engine {

// --- OnePoleFilter ---

void OnePoleFilter::prepare(float newSampleRate)
{
    sampleRate = newSampleRate;
    reset();
}

void OnePoleFilter::reset()
{
    prevOutput = 0.0f;
}

void OnePoleFilter::setCoefficient(float g)
{
    coefficient = g;
}

void OnePoleFilter::setCutoffFrequency(float freqHz)
{
    // Convert cutoff frequency to one-pole coefficient.
    // This is derived from matching the -3dB point of the analog
    // RC lowpass filter to the digital one-pole filter.
    coefficient = std::exp(-TWO_PI * freqHz / sampleRate);
}

float OnePoleFilter::processSample(float input)
{
    prevOutput = (1.0f - coefficient) * input + coefficient * prevOutput;
    return prevOutput;
}

// --- DcBlocker ---

void DcBlocker::reset()
{
    prevInput = 0.0f;
    prevOutput = 0.0f;
}

float DcBlocker::processSample(float input)
{
    // Highpass: y[n] = x[n] - x[n-1] + R * y[n-1]
    // R close to 1.0 means very low cutoff frequency (~7 Hz at 48kHz).
    float output = input - prevInput + R * prevOutput;
    prevInput = input;
    prevOutput = output;
    return output;
}

// --- BiquadFilter ---

void BiquadFilter::prepare(float newSampleRate)
{
    sampleRate = newSampleRate;
    reset();
}

void BiquadFilter::reset()
{
    z1 = 0.0f;
    z2 = 0.0f;
}

void BiquadFilter::setPeakingEQ(float freqHz, float Q, float gainDb)
{
    // Robert Bristow-Johnson's Audio EQ Cookbook — peaking EQ.
    //
    // A = 10^(dBgain/40)  — note: /40 not /20 because peaking EQ
    //                        boosts by A and cuts by 1/A (symmetric in dB)
    // w0 = 2*PI*freq/sampleRate — normalized angular frequency
    // alpha = sin(w0)/(2*Q) — bandwidth parameter
    float A = std::pow(10.0f, gainDb / 40.0f);
    float w0 = TWO_PI * freqHz / sampleRate;
    float sinW0 = std::sin(w0);
    float cosW0 = std::cos(w0);
    float alpha = sinW0 / (2.0f * Q);

    float a0 = 1.0f + alpha / A;

    // Compute and normalize coefficients by a0.
    b0 = (1.0f + alpha * A) / a0;
    b1 = (-2.0f * cosW0) / a0;
    b2 = (1.0f - alpha * A) / a0;
    a1 = (-2.0f * cosW0) / a0;   // same as b1/a0 for peaking EQ
    a2 = (1.0f - alpha / A) / a0;
}

float BiquadFilter::processSample(float input)
{
    // Direct Form II Transposed — 2 state variables, good numerical stability.
    float output = b0 * input + z1;
    z1 = b1 * input - a1 * output + z2;
    z2 = b2 * input - a2 * output;
    return output;
}

} // namespace engine
