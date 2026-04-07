#pragma once

// Basic filter primitives for the physical modeling engine.
// This file must NOT include any JUCE headers — pure C++ only.

namespace engine {

// One-pole lowpass filter.
//
// Used as the loop filter inside the Karplus-Strong string model.
// Each time the signal travels around the delay line loop, it passes
// through this filter, losing high-frequency energy — mimicking how
// a real string's higher harmonics decay faster than the fundamental.
//
// Difference equation: y[n] = (1 - g) * x[n] + g * y[n-1]
//
// g = 0: no filtering (bypass), all harmonics sustain equally
// g = 1: complete filtering (silence), all energy absorbed instantly
// g = 0.3–0.5: typical range for plucked strings (bright but natural decay)
class OnePoleFilter
{
public:
    void prepare(float sampleRate);
    void reset();

    // Set the filter coefficient directly (0..1).
    // Higher values = more lowpass = darker tone = faster high-freq decay.
    void setCoefficient(float g);

    // Set from a cutoff frequency in Hz (convenience method).
    // Internally converts to coefficient: g = exp(-2*pi*freq/sampleRate).
    void setCutoffFrequency(float freqHz);

    float processSample(float input);

    float getCoefficient() const { return coefficient; }

private:
    float sampleRate = 48000.0f;
    float coefficient = 0.5f;
    float prevOutput = 0.0f;
};

// DC blocking filter.
//
// The Karplus-Strong feedback loop can accumulate a small DC offset over
// many iterations. Left unchecked, this causes speaker excursion without
// audible sound and wastes headroom.
//
// This is a simple highpass filter that removes frequencies below ~10 Hz:
//   y[n] = x[n] - x[n-1] + R * y[n-1]
//
// R = 0.995 gives a cutoff around 7 Hz at 48kHz — low enough to not
// affect any audible content, high enough to remove DC effectively.
class DcBlocker
{
public:
    void reset();

    float processSample(float input);

private:
    static constexpr float R = 0.995f;
    float prevInput = 0.0f;
    float prevOutput = 0.0f;
};

// Second-order IIR (biquad) filter — Direct Form II Transposed.
//
// A biquad filter is the building block for parametric EQ, body resonance
// simulation, and many other audio effects. "Biquad" = two poles and two
// zeros in the Z-domain transfer function.
//
// WHY DIRECT FORM II TRANSPOSED (DF2T):
// Standard Direct Form I uses 4 state variables and can have large
// intermediate values that cause float32 precision issues at low frequencies
// or high Q. DF2T uses only 2 state variables (z1, z2) and has better
// numerical behavior — critical when modeling narrow body resonances.
//
// The transfer function:
//   H(z) = (b0 + b1*z^-1 + b2*z^-2) / (1 + a1*z^-1 + a2*z^-2)
//
// DF2T difference equations:
//   y[n] = b0 * x[n] + z1
//   z1   = b1 * x[n] - a1 * y[n] + z2
//   z2   = b2 * x[n] - a2 * y[n]
//
// Coefficient formulas from Robert Bristow-Johnson's "Audio EQ Cookbook".
class BiquadFilter
{
public:
    void prepare(float sampleRate);
    void reset();

    // Configure as a peaking EQ (resonant boost/cut at a center frequency).
    //
    // freqHz:  center frequency of the resonance (Hz)
    // Q:       quality factor — higher = narrower resonance
    //          Q=1 is very broad, Q=10 is narrow, Q=30+ is ringing
    // gainDb:  boost (+) or cut (-) in decibels at the center frequency
    //
    // RBJ Cookbook peaking EQ formulas:
    //   A     = 10^(gainDb/40)
    //   w0    = 2*PI*freq/sampleRate
    //   alpha = sin(w0) / (2*Q)
    //   b0    = 1 + alpha*A     b1 = -2*cos(w0)    b2 = 1 - alpha*A
    //   a0    = 1 + alpha/A     a1 = -2*cos(w0)    a2 = 1 - alpha/A
    //   (all divided by a0 to normalize)
    void setPeakingEQ(float freqHz, float Q, float gainDb);

    float processSample(float input);

private:
    float sampleRate = 48000.0f;

    // Normalized coefficients (a0 is folded into the others)
    float b0 = 1.0f, b1 = 0.0f, b2 = 0.0f;
    float a1 = 0.0f, a2 = 0.0f;

    // DF2T state variables
    float z1 = 0.0f, z2 = 0.0f;
};

} // namespace engine
