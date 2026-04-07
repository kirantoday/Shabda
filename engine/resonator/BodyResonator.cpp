#include "resonator/BodyResonator.h"
#include <algorithm>

namespace engine {

void BodyResonator::prepare(float newSampleRate, int /*maxBlockSize*/)
{
    sampleRate = newSampleRate;

    // Prepare all modes (even inactive ones, so they're ready if activated).
    for (auto& mode : modes)
        mode.prepare(sampleRate);

    reset();
}

void BodyResonator::reset()
{
    for (auto& mode : modes)
        mode.reset();
}

void BodyResonator::setPreset(const ModalFilterParams* params, int count)
{
    numActiveModes = std::min(count, MAX_BODY_MODES);

    for (int i = 0; i < numActiveModes; ++i)
        modes[static_cast<size_t>(i)].setParams(params[i]);
}

void BodyResonator::setModeParams(int modeIndex, const ModalFilterParams& params)
{
    if (modeIndex >= 0 && modeIndex < numActiveModes)
        modes[static_cast<size_t>(modeIndex)].setParams(params);
}

void BodyResonator::setDryWetMix(float mix)
{
    dryWetMix = std::clamp(mix, 0.0f, 1.0f);
}

void BodyResonator::processBlock(float* buffer, int numSamples)
{
    for (int i = 0; i < numSamples; ++i)
        buffer[i] = processSample(buffer[i]);
}

float BodyResonator::processSample(float input)
{
    if (numActiveModes == 0)
        return input;

    // Parallel processing: each mode independently filters the dry input.
    // Sum their outputs to form the wet signal.
    float wet = 0.0f;
    for (int i = 0; i < numActiveModes; ++i)
        wet += modes[static_cast<size_t>(i)].processSample(input);

    // Normalize wet signal to prevent gain buildup from summing multiple modes.
    // Each biquad peaking EQ passes the full signal plus a resonant boost,
    // so the sum of N modes has roughly N times the input level plus boosts.
    // Dividing by numActiveModes keeps the average level close to unity.
    wet /= static_cast<float>(numActiveModes);

    // Mix dry and wet signals.
    return (1.0f - dryWetMix) * input + dryWetMix * wet;
}

} // namespace engine
