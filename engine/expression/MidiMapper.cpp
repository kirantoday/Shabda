#include "expression/MidiMapper.h"

namespace engine {

void MidiMapper::prepare(float newSampleRate)
{
    sampleRate = newSampleRate;

    // CC messages arrive at ~30Hz (typical controller rate), so 10ms
    // smoothing is enough to eliminate staircase artifacts.
    vibratoSmoother.prepare(sampleRate, 10.0f);
    expressionSmoother.prepare(sampleRate, 10.0f);

    // Aftertouch can change quickly (finger pressure), use faster smoothing.
    aftertouchSmoother.prepare(sampleRate, 5.0f);

    reset();
}

void MidiMapper::reset()
{
    vibratoSmoother.snapTo(0.0f);       // No vibrato by default
    expressionSmoother.snapTo(1.0f);    // Full volume by default
    aftertouchSmoother.snapTo(0.0f);    // No aftertouch by default
}

void MidiMapper::handleControlChange(int cc, int value)
{
    // Normalize 0..127 → 0..1
    float normalized = static_cast<float>(value) / 127.0f;

    if (cc == CC_MOD_WHEEL)
    {
        // CC1: mod wheel → vibrato depth (scaled by maxVibratoDepth)
        vibratoSmoother.setTarget(normalized * maxVibratoDepth);
    }
    else if (cc == CC_EXPRESSION)
    {
        // CC11: expression → volume multiplier (0 = silent, 1 = full)
        expressionSmoother.setTarget(normalized);
    }
    // EXTENSION: add more CC mappings here (CC74 brightness, etc.)
}

void MidiMapper::handleAftertouch(int value)
{
    float normalized = static_cast<float>(value) / 127.0f;
    aftertouchSmoother.setTarget(normalized);
}

float MidiMapper::getVibratoDepth()
{
    return vibratoSmoother.getNext();
}

float MidiMapper::getExpressionGain()
{
    return expressionSmoother.getNext();
}

float MidiMapper::getAftertouchValue()
{
    return aftertouchSmoother.getNext();
}

void MidiMapper::setMaxVibratoDepth(float semitones)
{
    maxVibratoDepth = semitones;
}

} // namespace engine
