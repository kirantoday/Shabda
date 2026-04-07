#pragma once

#include "common/Filters.h"
#include <vector>
#include <cstdint>

// Pluck excitation generator for Karplus-Strong synthesis.
// This file must NOT include any JUCE headers — pure C++ only.
//
// HOW PLUCK EXCITATION WORKS:
// When you pluck a real string, you displace it into a shape and release.
// That initial shape contains energy at many harmonics. We model this as
// a short burst of filtered noise — the length of one period of the
// fundamental frequency.
//
// Three parameters shape the excitation:
//
// 1. BRIGHTNESS: A lowpass filter on the noise. Bright pluck (near 1.0)
//    has lots of high-frequency energy (sharp attack). Dull pluck (near 0.0)
//    sounds muffled (like plucking with the flesh of the finger).
//
// 2. PLUCK POSITION: Where along the string you pluck. Plucking at position
//    P suppresses every harmonic that has a node at that position. Modeled
//    as a comb filter: y[n] = x[n] - x[n - round(delay*position)].
//    - Position 0.1 (near bridge): bright, nasal, all harmonics present
//    - Position 0.5 (middle): mellow, odd harmonics only
//    - Veena: typically 0.1–0.2 (bright, characteristic attack)
//
// 3. PLUCK STRENGTH: Amplitude scaling, mapped from MIDI velocity.

namespace engine {

class PluckExciter
{
public:
    void prepare(float sampleRate, int maxBlockSize);
    void reset();

    // Generate excitation samples into the provided buffer.
    // Returns the number of samples written (= one period of the fundamental).
    //
    // delayLength:   delay line length in samples (determines period)
    // pluckPosition: 0..1 position along the string
    // pluckStrength: 0..1 amplitude (typically from MIDI velocity)
    // brightness:    0..1 controls lowpass filter on noise
    int generateExcitation(float* outputBuffer, int bufferSize,
                           float delayLength,
                           float pluckPosition,
                           float pluckStrength,
                           float brightness);

private:
    std::vector<float> workBuffer;  // preallocated workspace
    OnePoleFilter brightnessFilter;
    float sampleRate = 48000.0f;

    // Fast xorshift32 RNG — no stdlib dependency in audio code.
    // Quality doesn't matter much since we're filtering the output anyway.
    uint32_t rngState = 123456789u;
    float nextRandom();  // returns uniform random in [-1, +1]
};

} // namespace engine
