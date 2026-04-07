#pragma once

// Interpolation utilities for fractional delay lines.
// This file must NOT include any JUCE headers — pure C++ only.
//
// WHY ALLPASS OVER LINEAR INTERPOLATION?
// Linear interpolation acts as a lowpass filter that attenuates high frequencies
// more as the fractional delay approaches 0.5 samples. This causes audible
// dulling on high notes where the delay line is short (few samples).
// Allpass interpolation passes all frequencies at unity gain — it only shifts
// phase. This preserves brightness across the entire pitch range.

namespace engine {
namespace Interpolation {

    // Calculate the allpass coefficient from a fractional delay value.
    //
    // The fractional delay `d` is the part after the decimal point of the
    // total delay (0 < d < 1). The coefficient controls how much phase shift
    // the allpass filter applies.
    //
    // Formula: alpha = (1 - d) / (1 + d)
    //
    // When d is near 0, alpha approaches 1 (minimal phase shift).
    // When d is near 1, alpha approaches 0 (maximum phase shift).
    // When d = 0.5, alpha = 1/3 (the "sweet spot").
    inline float allpassCoefficient(float fractionalDelay)
    {
        return (1.0f - fractionalDelay) / (1.0f + fractionalDelay);
    }

    // First-order allpass interpolation for fractional delay.
    //
    // This implements the difference equation:
    //   y[n] = alpha * x[n] + x[n-1] - alpha * y[n-1]
    //
    // where x[n] is the sample read at the integer delay position,
    // and the allpass filter provides the remaining fractional delay.
    //
    // prevInput and prevOutput are persistent state that must be maintained
    // between calls (stored in the DelayLine).
    inline float allpassInterpolate(float currentSample, float alpha,
                                     float& prevInput, float& prevOutput)
    {
        float output = alpha * currentSample + prevInput - alpha * prevOutput;
        prevInput = currentSample;
        prevOutput = output;
        return output;
    }

    // Linear interpolation — provided as a simpler fallback.
    // Blends between two adjacent samples based on `fraction` (0..1).
    //   fraction=0 → returns sample0
    //   fraction=1 → returns sample1
    inline float linearInterpolate(float sample0, float sample1, float fraction)
    {
        return sample0 + fraction * (sample1 - sample0);
    }

} // namespace Interpolation
} // namespace engine
