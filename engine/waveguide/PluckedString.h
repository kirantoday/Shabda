#pragma once

#include "waveguide/DelayLine.h"
#include "common/Filters.h"
#include "exciter/PluckExciter.h"
#include <vector>

// Karplus-Strong extended plucked string model.
// This file must NOT include any JUCE headers — pure C++ only.
//
// HOW KARPLUS-STRONG WORKS:
//
// 1. Fill a delay line with a short burst of noise (the "pluck").
// 2. Each sample period:
//    a. Read the oldest sample from the delay line
//    b. Pass it through a lowpass "loop filter"
//    c. Write the filtered sample back into the delay line
//    d. Output the sample
//
// The delay line length determines the pitch (longer = lower).
// The loop filter controls the decay character:
//   - Each trip around the loop, the filter removes some high-frequency energy
//   - High harmonics die first (just like a real string!)
//   - The fundamental decays last (long sustain)
//
// The result is a remarkably realistic plucked string sound from a very
// simple algorithm. Kevin Karplus and Alex Strong published this in 1983.

namespace engine {

class PluckedString
{
public:
    void prepare(float sampleRate, int maxBlockSize);
    void reset();

    // Trigger a new note. Velocity is 0..1 (from MIDI velocity / 127).
    void noteOn(int midiNote, float velocity);

    // Release the note — increases damping for faster decay.
    void noteOff();

    // Render audio into the output buffer (mono).
    void processBlock(float* outputBuffer, int numSamples);

    // Process a single sample. Call setPitchOffset() before each sample
    // if pitch is changing continuously (meend/gamaka).
    float processSample();

    // --- Parameter setters ---
    // All parameters can be changed at any time (smoothing handled internally).

    void setPluckPosition(float position);   // 0.05..0.5
    void setDamping(float damping);          // 0..0.95
    void setBrightness(float brightness);    // 0..1
    void setPluckStrength(float strength);   // 0..1

    // Continuous pitch offset in semitones for meend/gamaka.
    // Can be called per-sample for smooth pitch bending.
    void setPitchOffset(float semitones);

    bool isActive() const { return active; }

private:
    DelayLine delayLine;
    OnePoleFilter loopFilter;
    DcBlocker dcBlocker;
    PluckExciter exciter;

    float sampleRate = 48000.0f;
    float currentDelayLength = 100.0f;  // in samples, determines current pitch
    float baseDelayLength = 100.0f;     // delay at base MIDI note (no pitch offset)
    int currentMidiNote = 60;
    float currentVelocity = 0.0f;

    // Parameters — defaults are for generic string, veena overrides in VeenaConfig
    float pluckPosition = 0.15f;      // TODO(TUNE): where along string to pluck
    float damping = 0.4f;             // TODO(TUNE): loop filter coefficient
    float brightness = 0.65f;         // TODO(TUNE): excitation brightness
    float pluckStrength = 0.8f;
    float pitchOffsetSemitones = 0.0f;

    // Note-off damping: how much to increase the loop filter coefficient
    // when the note is released, to simulate finger damping.
    float noteOffDampingBoost = 0.85f; // TODO(TUNE): note-off damping target

    // State
    bool active = false;
    bool noteHeld = false;

    // Preallocated excitation buffer — filled at noteOn time,
    // not during processBlock (no allocation in audio path).
    std::vector<float> excitationBuffer;

    // Silence detection
    float energyAccumulator = 0.0f;
    int silenceCheckCounter = 0;
    static constexpr float SILENCE_THRESHOLD = 1.0e-6f;
    static constexpr int SILENCE_CHECK_INTERVAL = 64;

    // Convert MIDI note (with pitch offset) to delay length in samples.
    float midiNoteToDelay(float midiNote) const;

    // Update the loop filter coefficient based on damping and note state.
    void updateLoopFilter();
};

} // namespace engine
