#pragma once

#include "expression/ParameterSmoother.h"

// Centralized MIDI CC → engine parameter routing.
// This file must NOT include any JUCE headers — pure C++ only.
//
// Maps standard MIDI CC messages to instrument parameters:
//   CC1  (Mod Wheel)  → vibrato depth (kampita)
//   CC11 (Expression)  → overall volume/energy
//   Aftertouch         → brightness modulation
//
// All values are smoothed via ParameterSmoother to prevent zipper noise
// when controllers send discrete step values.

namespace engine {

class MidiMapper
{
public:
    void prepare(float sampleRate);
    void reset();

    // --- Input: called from MIDI handler (not audio-rate) ---

    // Handle a MIDI CC message. cc = controller number, value = 0..127.
    void handleControlChange(int cc, int value);

    // Handle channel pressure (aftertouch). value = 0..127.
    void handleAftertouch(int value);

    // --- Per-sample output: called once per sample in processBlock ---

    // Vibrato depth in semitones (0..maxVibratoDepth). From CC1.
    float getVibratoDepth();

    // Expression gain multiplier (0..1). From CC11.
    float getExpressionGain();

    // Aftertouch value normalized (0..1). For brightness/damping modulation.
    float getAftertouchValue();

    // --- Configuration ---

    // Maximum vibrato depth when CC1 is at 127.
    // TODO(TUNE): 1.0 semitone is moderate kampita depth
    void setMaxVibratoDepth(float semitones);

private:
    ParameterSmoother vibratoSmoother;     // CC1 → vibrato depth
    ParameterSmoother expressionSmoother;  // CC11 → volume
    ParameterSmoother aftertouchSmoother;  // aftertouch → brightness

    float maxVibratoDepth = 1.0f;  // TODO(TUNE): max semitones at CC1=127
    float sampleRate = 48000.0f;

    // CC numbers
    static constexpr int CC_MOD_WHEEL = 1;
    static constexpr int CC_EXPRESSION = 11;
};

} // namespace engine
