#pragma once

#include <cmath>

// One-pole exponential smoother for control-rate parameters.
// This file must NOT include any JUCE headers — pure C++ only.
//
// HOW IT WORKS:
// When you set a new target value, the smoother doesn't jump there instantly.
// Instead, it exponentially approaches the target over a configurable time.
// This prevents audible "zipper noise" (staircase artifacts) when parameters
// change rapidly — like when a pitch wheel sends many discrete MIDI messages.
//
// The math: y[n] = alpha * y[n-1] + (1 - alpha) * target
//
// alpha is derived from a time constant in milliseconds:
//   alpha = exp(-1 / (sampleRate * timeMs / 1000))
//
// With a 5ms time constant at 48kHz:
//   - After 5ms:  ~63% of the way to target
//   - After 15ms: ~95% of the way to target
//   - After 25ms: ~99% of the way to target (effectively arrived)
//
// WHY SEPARATE FROM OnePoleFilter:
// OnePoleFilter is for audio signal processing (setCutoffFrequency).
// ParameterSmoother is for control parameters (setTarget/getNext).
// Different mental models, different usage patterns.

namespace engine {

class ParameterSmoother
{
public:
    // Configure the smoother. Must be called before use.
    // smoothingTimeMs: time to reach ~63% of target.
    // TODO(TUNE): default smoothing time — 5ms is a starting point
    void prepare(float sampleRate, float smoothingTimeMs = 5.0f)
    {
        float timeInSamples = sampleRate * smoothingTimeMs / 1000.0f;
        if (timeInSamples < 1.0f)
            timeInSamples = 1.0f;
        alpha = std::exp(-1.0f / timeInSamples);
    }

    void reset()
    {
        // Snap current to target — no gliding after reset.
        currentValue = targetValue;
    }

    // Set the value we're smoothing toward.
    void setTarget(float newTarget)
    {
        targetValue = newTarget;
    }

    // Get the next smoothed value. Call once per sample.
    float getNext()
    {
        currentValue = alpha * currentValue + (1.0f - alpha) * targetValue;
        return currentValue;
    }

    // Jump immediately to a value — no smoothing.
    // Use on noteOn to prevent pitch glide from previous note's bend.
    void snapTo(float value)
    {
        currentValue = value;
        targetValue = value;
    }

    float getCurrentValue() const { return currentValue; }
    float getTargetValue() const { return targetValue; }

    // Returns true if still moving toward target.
    bool isSmoothing() const
    {
        float diff = currentValue - targetValue;
        return (diff * diff) > 1.0e-10f;
    }

private:
    float alpha = 0.99f;
    float currentValue = 0.0f;
    float targetValue = 0.0f;
};

} // namespace engine
