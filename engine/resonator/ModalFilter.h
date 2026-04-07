#pragma once

#include "common/Filters.h"

// A single resonant mode of an instrument body.
// This file must NOT include any JUCE headers — pure C++ only.
//
// WHAT IS A BODY MODE?
// When you tap a wooden instrument body, it vibrates at several specific
// frequencies determined by its shape, material, and size. Each frequency
// is a "mode" — a natural resonance. Together, these modes give the
// instrument its characteristic tone color (warm, bright, woody, etc.).
//
// This class wraps a BiquadFilter configured as a peaking EQ to simulate
// one mode. Multiple ModalFilters combined form a BodyResonator.
//
// EXTENSION: future versions may add mode coupling, amplitude-dependent Q,
// or nonlinear saturation per mode for more realistic behavior.

namespace engine {

// Parameters for a single body mode.
// This is a simple aggregate — no constructors, so constexpr arrays work.
struct ModalFilterParams
{
    float frequencyHz = 200.0f;   // Center frequency of this body mode
    float Q           = 5.0f;     // Quality factor (higher = narrower resonance)
    float gainDb      = 3.0f;     // Boost in dB at center frequency
};

class ModalFilter
{
public:
    void prepare(float sampleRate);
    void reset();

    // Set or update the mode parameters. Can be called at any time.
    void setParams(const ModalFilterParams& newParams);

    float processSample(float input);

    const ModalFilterParams& getParams() const { return params; }

private:
    BiquadFilter filter;
    ModalFilterParams params;
    float sampleRate = 48000.0f;
};

} // namespace engine
