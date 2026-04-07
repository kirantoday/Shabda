#include "sympathetic/SympatheticBank.h"
#include "common/DSPConstants.h"
#include <algorithm>
#include <cmath>

namespace engine {

// --- SympatheticString ---

void SympatheticBank::SympatheticString::prepare(float sampleRate, int maxDelaySamples)
{
    delayLine.prepare(sampleRate, maxDelaySamples);
    loopFilter.prepare(sampleRate);
    reset();
}

void SympatheticBank::SympatheticString::reset()
{
    delayLine.reset();
    loopFilter.reset();
    dcBlocker.reset();
    active = false;
}

void SympatheticBank::SympatheticString::setTuning(int midiNote, float sampleRate)
{
    float frequency = midiNoteToFrequency(static_cast<float>(midiNote));
    delayLengthSamples = sampleRate / frequency;
    active = true;
}

float SympatheticBank::SympatheticString::processSample(float input)
{
    // Feedback comb filter with lowpass in the feedback loop.
    //
    // 1. Read the delayed sample (one full period of this drone's fundamental).
    //    This creates resonance peaks at f, 2f, 3f, 4f...
    float delayed = delayLine.readSample(delayLengthSamples);

    // 2. Lowpass in feedback: higher harmonics of the drone decay faster,
    //    just like a real string.
    float filtered = loopFilter.processSample(delayed);

    // 3. Combine input with feedback — the comb filter equation.
    //    When the input has energy at the drone's harmonics, the feedback
    //    reinforces it. Otherwise, it decays naturally.
    float combOutput = input + feedback * filtered;

    // 4. Write back into the delay line for the next cycle.
    delayLine.pushSample(combOutput);

    // 5. DC block to prevent accumulation in the feedback loop.
    //    Return only the RESONANT part (subtract the input to isolate
    //    the sympathetic contribution).
    return dcBlocker.processSample(feedback * filtered);
}

// --- SympatheticBank ---

void SympatheticBank::prepare(float newSampleRate, int /*maxBlockSize*/)
{
    sampleRate = newSampleRate;

    // Max delay for lowest possible drone note (MIDI 36 = C2 ≈ 65 Hz).
    // At 48kHz: 48000/65 ≈ 738 samples. Use generous headroom.
    int maxDelay = static_cast<int>(sampleRate / 30.0f) + 4;

    for (auto& s : strings)
        s.prepare(sampleRate, maxDelay);
}

void SympatheticBank::reset()
{
    for (auto& s : strings)
        s.reset();
}

void SympatheticBank::setTunings(const int* midiNotes, int count)
{
    numActiveStrings = std::min(count, MAX_SYMPATHETIC_BANK_SIZE);

    for (int i = 0; i < numActiveStrings; ++i)
    {
        strings[static_cast<size_t>(i)].setTuning(midiNotes[i], sampleRate);
        strings[static_cast<size_t>(i)].feedback = feedbackValue;
        strings[static_cast<size_t>(i)].loopFilter.setCoefficient(dampingValue);
    }
}

void SympatheticBank::setGain(float newGain)
{
    gain = std::clamp(newGain, 0.0f, 1.0f);
}

void SympatheticBank::setFeedback(float newFeedback)
{
    feedbackValue = std::clamp(newFeedback, 0.0f, 0.999f);
    for (int i = 0; i < numActiveStrings; ++i)
        strings[static_cast<size_t>(i)].feedback = feedbackValue;
}

void SympatheticBank::setDamping(float newDamping)
{
    dampingValue = std::clamp(newDamping, 0.0f, 0.95f);
    for (int i = 0; i < numActiveStrings; ++i)
        strings[static_cast<size_t>(i)].loopFilter.setCoefficient(dampingValue);
}

void SympatheticBank::processBlock(float* buffer, int numSamples)
{
    if (numActiveStrings == 0 || gain < 1.0e-6f)
        return;

    for (int i = 0; i < numSamples; ++i)
    {
        float input = buffer[i];
        float sympatheticSum = 0.0f;

        for (int s = 0; s < numActiveStrings; ++s)
            sympatheticSum += strings[static_cast<size_t>(s)].processSample(input);

        // Add the sympathetic contribution to the output.
        // The gain controls how prominent the drone shimmer is.
        buffer[i] = input + gain * sympatheticSum;
    }
}

} // namespace engine
