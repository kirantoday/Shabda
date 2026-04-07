#pragma once

#include "waveguide/DelayLine.h"
#include "common/Filters.h"
#include <array>

// Bank of sympathetic drone strings modeled as feedback comb filters.
// This file must NOT include any JUCE headers — pure C++ only.
//
// HOW SYMPATHETIC RESONANCE WORKS:
//
// The veena has 3 thalam (side drone) strings tuned to Sa, Pa, sa.
// When the main melody string plays a note whose harmonics align with
// a drone string's natural frequencies, the drone vibrates sympathetically.
// This creates a subtle shimmering "halo" around sustained notes.
//
// MODELING APPROACH: FEEDBACK COMB FILTERS
//
// Each drone string is modeled as a comb filter with feedback:
//   y[n] = x[n] + feedback * lowpass(delayLine[n - D])
//
// where D = sampleRate / droneFrequency.
//
// A comb filter has resonance peaks at f, 2f, 3f, 4f... — exactly the
// harmonic series of an open string. When the main string's output contains
// energy at any of these harmonics, the comb filter amplifies and sustains
// that energy. When no harmonics align, the input passes through with
// minimal reinforcement.
//
// This naturally implements energy transfer — no explicit frequency
// detection is needed! The physics does the work.
//
// The lowpass filter in the feedback loop makes higher harmonics of the
// drone decay faster, which is physically accurate (strings lose high
// frequencies more quickly than the fundamental).
//
// EXTENSION: future instruments (sitar, sarangi) may have 13-36 sympathetic
// strings. This class supports up to MAX_SYMPATHETIC_BANK_SIZE.

namespace engine {

constexpr int MAX_SYMPATHETIC_BANK_SIZE = 4;

class SympatheticBank
{
public:
    void prepare(float sampleRate, int maxBlockSize);
    void reset();

    // Configure the drone string tunings from an array of MIDI note numbers.
    void setTunings(const int* midiNotes, int count);

    // Overall volume of sympathetic resonance (0..1).
    // This is additive — the drones ADD to the main signal.
    void setGain(float gain);

    // Feedback controls how long the drones ring after excitation (0..0.999).
    void setFeedback(float feedback);

    // Damping controls high-frequency rolloff in the drone feedback loop (0..0.95).
    void setDamping(float damping);

    // Process a buffer in-place. Reads the main string signal,
    // adds the sympathetic contribution back into the same buffer.
    void processBlock(float* buffer, int numSamples);

private:
    // A single sympathetic drone string — feedback comb filter.
    struct SympatheticString
    {
        DelayLine delayLine;
        OnePoleFilter loopFilter;
        DcBlocker dcBlocker;
        float delayLengthSamples = 100.0f;
        float feedback = 0.98f;
        bool active = false;

        void prepare(float sampleRate, int maxDelaySamples);
        void reset();
        void setTuning(int midiNote, float sampleRate);

        float processSample(float input);
    };

    std::array<SympatheticString, MAX_SYMPATHETIC_BANK_SIZE> strings;
    int numActiveStrings = 0;
    float gain = 0.15f;           // TODO(TUNE): subtle by default
    float feedbackValue = 0.98f;  // TODO(TUNE): moderate ring time
    float dampingValue = 0.3f;    // TODO(TUNE): moderate high-freq rolloff
    float sampleRate = 48000.0f;
};

} // namespace engine
