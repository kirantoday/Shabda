#pragma once

#include "common/DSPConstants.h"
#include <cmath>

// Simple sine LFO for vibrato / kampita modulation.
// This file must NOT include any JUCE headers — pure C++ only.
//
// KAMPITA (Indian vibrato):
// Unlike Western vibrato which is typically symmetric and steady,
// kampita is an ornamental oscillation specific to Indian classical music.
// It can be asymmetric, variable in speed, and is used as a deliberate
// embellishment rather than a constant effect.
//
// For v1, we use a simple sine oscillator. The depth is controlled by
// CC1 (mod wheel) — 0 = no vibrato, 127 = maximum depth.
//
// TODO(TUNE): kampita is often asymmetric (more bend up than down).
// Future: add waveform shaping parameter for asymmetric kampita.

namespace engine {

class VibratoLFO
{
public:
    void prepare(float newSampleRate)
    {
        sampleRate = newSampleRate;
        phaseIncrement = rate / sampleRate;
    }

    void reset()
    {
        phase = 0.0f;
    }

    // Set vibrato rate in Hz. Typical kampita: 4-7 Hz.
    // TODO(TUNE): default rate
    void setRate(float hz)
    {
        rate = hz;
        phaseIncrement = rate / sampleRate;
    }

    // Set vibrato depth in semitones.
    // 0 = no vibrato, 0.5 = moderate, 1.0 = extreme.
    // Controlled by CC1 (mod wheel).
    void setDepth(float semitones)
    {
        depth = semitones;
    }

    // Get the next LFO sample — returns pitch offset in semitones.
    // Call once per audio sample in the processBlock loop.
    float getNextSample()
    {
        if (depth < 1.0e-6f)
            return 0.0f;

        float value = depth * std::sin(TWO_PI * phase);

        phase += phaseIncrement;
        if (phase >= 1.0f)
            phase -= 1.0f;

        return value;
    }

    float getDepth() const { return depth; }
    float getRate() const { return rate; }

private:
    float sampleRate = 48000.0f;
    float phase = 0.0f;
    float phaseIncrement = 0.0f;
    float rate = 5.5f;     // TODO(TUNE): default kampita rate in Hz
    float depth = 0.0f;    // Controlled by CC1, starts at 0 (off)
};

} // namespace engine
