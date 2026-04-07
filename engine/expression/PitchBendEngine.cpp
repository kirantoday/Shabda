#include "expression/PitchBendEngine.h"
#include <cmath>
#include <algorithm>

namespace engine {

void PitchBendEngine::prepare(float newSampleRate, int /*maxBlockSize*/)
{
    sampleRate = newSampleRate;
    // TODO(TUNE): smoothing time — 5ms balances responsiveness vs. smoothness
    smoother.prepare(sampleRate, 5.0f);
    reset();
}

void PitchBendEngine::reset()
{
    smoother.snapTo(0.0f);
}

void PitchBendEngine::setMidiPitchBend(int midiValue)
{
    // MIDI pitch bend is 14-bit: 0..16383, center = 8192.
    // Normalize to -1..+1 range.
    float normalized = (static_cast<float>(midiValue) - 8192.0f) / 8192.0f;
    normalized = std::clamp(normalized, -1.0f, 1.0f);

    // Apply curve shaping BEFORE smoothing.
    // Why before? Because smoothing after shaping means the perceived
    // movement is smooth. Smoothing before shaping would distort the
    // smoother's linear trajectory through the nonlinear curve.
    float shaped = applyCurve(normalized);

    // Scale to semitones and set as smoother target.
    smoother.setTarget(shaped * bendRangeSemitones);
}

void PitchBendEngine::setSemitoneOffset(float semitones)
{
    // Direct semitone input — bypasses curve shaping.
    // Used by UI slider or internal modulation.
    float clamped = std::clamp(semitones, -bendRangeSemitones, bendRangeSemitones);
    smoother.setTarget(clamped);
}

void PitchBendEngine::setBendRangeSemitones(float range)
{
    bendRangeSemitones = std::clamp(range, 0.0f, 12.0f);
}

void PitchBendEngine::setCurve(BendCurve curve)
{
    currentCurve = curve;
}

void PitchBendEngine::setSmoothingTimeMs(float ms)
{
    smoother.prepare(sampleRate, ms);
}

float PitchBendEngine::getNextPitchOffset()
{
    return smoother.getNext();
}

void PitchBendEngine::snapToCurrentValue()
{
    smoother.reset();
}

float PitchBendEngine::applyCurve(float x) const
{
    // x is in -1..+1. Output is -1..+1.
    // All curves preserve sign symmetry and pass through (-1,-1), (0,0), (1,1).

    switch (currentCurve)
    {
        case BendCurve::Exponential:
        {
            // y = sign(x) * |x|^p
            // p = 2.0: small movements near center are precise (fine gamaka),
            // large movements accelerate (broad meend). Mimics the increasing
            // physical effort of pushing a veena string further across the fret.
            // TODO(TUNE): exponent value
            constexpr float p = 2.0f;
            float sign = (x >= 0.0f) ? 1.0f : -1.0f;
            return sign * std::pow(std::abs(x), p);
        }

        case BendCurve::SCurve:
        {
            // Hermite smoothstep: y = sign(x) * (3t^2 - 2t^3) where t = |x|
            // Flat near 0 AND near 1, steep in the middle.
            // Useful for landing precisely on target notes — the player gets
            // fine control both at the starting pitch and the target pitch,
            // with a quicker transition between them.
            // TODO(TUNE): S-curve steepness
            float t = std::abs(x);
            float s = t * t * (3.0f - 2.0f * t);
            return (x >= 0.0f) ? s : -s;
        }

        case BendCurve::Linear:
        default:
            return x;
    }
}

} // namespace engine
