#pragma once

#include "resonator/ModalFilter.h"
#include <array>

// Bank of parallel resonant filters modeling an instrument body.
// This file must NOT include any JUCE headers — pure C++ only.
//
// HOW BODY RESONANCE WORKS:
// A real instrument body (guitar, veena kudam, violin) has multiple
// resonant frequencies. When the string vibrates, the body amplifies
// energy near those frequencies, giving the instrument its characteristic
// tone color. Without body resonance, all plucked strings sound similar.
//
// SIGNAL FLOW:
//   Each input sample is fed to ALL modal filters in parallel.
//   Their outputs are summed to form the "wet" signal.
//   The final output is a mix of dry (original string) and wet (body-resonated):
//     output = (1 - mix) * dry + mix * wet
//
// WHY PARALLEL (not series):
//   In a real body, each resonant mode vibrates independently in response
//   to the string's energy. Parallel processing models this correctly.
//   Series would mean each mode's output feeds the next, creating artificial
//   interaction between modes.
//
// EXTENSION: future instruments swap the preset (different body = different
// filter frequencies/Q/gains). The BodyResonator class is instrument-agnostic.

namespace engine {

constexpr int MAX_BODY_MODES = 8;

class BodyResonator
{
public:
    void prepare(float sampleRate, int maxBlockSize);
    void reset();

    // Load a complete body preset — an array of ModalFilterParams.
    // count must be <= MAX_BODY_MODES.
    void setPreset(const ModalFilterParams* params, int count);

    // Set parameters for a single mode (0-indexed).
    void setModeParams(int modeIndex, const ModalFilterParams& params);

    // Dry/wet mix: 0.0 = fully dry (bypass), 1.0 = fully wet (body only).
    void setDryWetMix(float mix);

    float getDryWetMix() const { return dryWetMix; }
    int getNumModes() const { return numActiveModes; }

    // Process a buffer in-place.
    void processBlock(float* buffer, int numSamples);

    // Process a single sample.
    float processSample(float input);

private:
    std::array<ModalFilter, MAX_BODY_MODES> modes;
    int numActiveModes = 0;

    // TODO(TUNE): default body mix — 0.5 gives balanced body presence
    float dryWetMix = 0.5f;

    float sampleRate = 48000.0f;
};

} // namespace engine
