#include "exciter/PluckExciter.h"
#include "common/DSPConstants.h"
#include <algorithm>
#include <cmath>

namespace engine {

void PluckExciter::prepare(float newSampleRate, int /*maxBlockSize*/)
{
    sampleRate = newSampleRate;
    brightnessFilter.prepare(sampleRate);

    // Preallocate work buffer for the longest possible excitation.
    // Longest delay = lowest note at lowest sample rate.
    // 48000 / 27.5 Hz (A0) ≈ 1745 samples. 4096 covers everything with margin.
    workBuffer.resize(4096, 0.0f);

    reset();
}

void PluckExciter::reset()
{
    brightnessFilter.reset();
    rngState = 123456789u;
}

int PluckExciter::generateExcitation(float* outputBuffer, int bufferSize,
                                      float delayLength,
                                      float pluckPosition,
                                      float pluckStrength,
                                      float brightness)
{
    // The excitation length is one period of the fundamental.
    int excitationLength = static_cast<int>(delayLength);
    excitationLength = std::min(excitationLength, bufferSize);
    excitationLength = std::min(excitationLength, static_cast<int>(workBuffer.size()));

    if (excitationLength <= 0)
        return 0;

    // --- Step 1: Generate raw noise ---
    for (int i = 0; i < excitationLength; ++i)
        workBuffer[static_cast<size_t>(i)] = nextRandom();

    // --- Step 2: Apply brightness filter ---
    // Map brightness (0..1) to a cutoff frequency.
    // brightness=1.0 → cutoff at Nyquist (no filtering, full brightness)
    // brightness=0.0 → cutoff at ~200 Hz (very dull)
    // We use an exponential mapping for perceptual linearity.
    float minCutoff = 200.0f;
    float maxCutoff = sampleRate * 0.45f;  // just below Nyquist
    // TODO(TUNE): brightness curve — exponential feels more natural
    float cutoff = minCutoff * std::pow(maxCutoff / minCutoff, brightness);

    brightnessFilter.reset();
    brightnessFilter.setCutoffFrequency(cutoff);

    for (int i = 0; i < excitationLength; ++i)
        workBuffer[static_cast<size_t>(i)] = brightnessFilter.processSample(workBuffer[static_cast<size_t>(i)]);

    // --- Step 3: Apply pluck position comb filter ---
    // y[n] = x[n] - x[n - P]  where P = round(delayLength * pluckPosition)
    //
    // This creates nulls in the spectrum at multiples of sampleRate/P,
    // simulating the suppression of harmonics that have a node at the
    // pluck point on the string.
    int combDelay = std::max(1, static_cast<int>(std::round(delayLength * pluckPosition)));

    // Apply in-place, processing backwards to avoid overwriting needed values.
    // Actually, since we're subtracting a past value, we can go forward
    // if we keep the original values in a separate read. The workBuffer
    // already has the data, so we copy to output while applying the comb.
    for (int i = 0; i < excitationLength; ++i)
    {
        float delayed = (i >= combDelay)
            ? workBuffer[static_cast<size_t>(i - combDelay)]
            : 0.0f;  // zero-pad for samples before the comb delay
        outputBuffer[i] = workBuffer[static_cast<size_t>(i)] - delayed;
    }

    // --- Step 4: Apply amplitude scaling ---
    for (int i = 0; i < excitationLength; ++i)
        outputBuffer[i] *= pluckStrength;

    return excitationLength;
}

float PluckExciter::nextRandom()
{
    // Xorshift32 — fast, decent quality for noise generation.
    // Produces a full-range uint32, which we map to [-1, +1].
    rngState ^= rngState << 13;
    rngState ^= rngState >> 17;
    rngState ^= rngState << 5;

    // Convert to float in [-1, 1] range.
    // Divide by half the uint32 range to get [0, 2], then subtract 1.
    return static_cast<float>(rngState) / 2147483648.0f - 1.0f;
}

} // namespace engine
