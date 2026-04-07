#pragma once

#include "expression/ParameterSmoother.h"

// High-resolution pitch bend engine with curve shaping for meend/gamaka.
// This file must NOT include any JUCE headers — pure C++ only.
//
// MEEND/GAMAKA ON THE VEENA:
// The player pushes the string laterally across the fret to bend pitch.
// This is the soul of veena music — not optional decoration, but the
// primary expressive technique. Bends can reach a 4th or 5th (5-7 semitones).
//
// The pitch bend engine provides:
// 1. 14-bit MIDI pitch bend support (16384 values for high resolution)
// 2. Configurable bend range (default ±7 semitones)
// 3. Curve shaping to control the feel of the bend
// 4. Per-sample smoothing to eliminate zipper noise
//
// CURVE SHAPING:
// The curve transforms the normalized input (-1..+1) before scaling.
// All curves pass through (-1,-1), (0,0), and (1,1).
//
// Linear:      y = x             Direct proportional mapping.
// Exponential: y = sign(x)*|x|^2 Fine control near center, accelerates outward.
//                                 Mimics physical effort of pushing string further.
// S-Curve:     Hermite smoothstep Precise at both extremes, faster in middle.
//                                 Useful for landing precisely on target notes.

namespace engine {

enum class BendCurve : int
{
    Linear = 0,
    Exponential,
    SCurve
};

class PitchBendEngine
{
public:
    void prepare(float sampleRate, int maxBlockSize);
    void reset();

    // --- Input methods (called from MIDI handler, not audio thread) ---

    // Raw 14-bit MIDI pitch bend: 0..16383, center = 8192.
    void setMidiPitchBend(int midiValue);

    // Direct semitone offset (for MPE, internal modulation, or UI slider).
    void setSemitoneOffset(float semitones);

    // --- Configuration ---

    void setBendRangeSemitones(float range);
    void setCurve(BendCurve curve);
    void setSmoothingTimeMs(float ms);

    // --- Per-sample output (call once per sample in processBlock) ---

    // Returns the smoothed pitch offset in semitones for this sample.
    float getNextPitchOffset();

    // Snap smoother to current target — call on noteOn to prevent
    // the new note from gliding from the previous bend position.
    void snapToCurrentValue();

    float getBendRangeSemitones() const { return bendRangeSemitones; }

private:
    ParameterSmoother smoother;

    // TODO(TUNE): ±7 semitones default — covers veena gamaka range (up to a 5th)
    float bendRangeSemitones = 7.0f;
    BendCurve currentCurve = BendCurve::Linear;
    float sampleRate = 48000.0f;

    // Apply curve shaping to normalized input (-1..+1).
    float applyCurve(float normalizedInput) const;
};

} // namespace engine
