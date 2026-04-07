#include "waveguide/DelayLine.h"
#include "common/Interpolation.h"
#include <algorithm>
#include <cmath>

namespace engine {

void DelayLine::prepare(float newSampleRate, int maxDelaySamples)
{
    sampleRate = newSampleRate;

    // Round up to power of 2 for efficient masking.
    // Add a small margin so we never read at the exact edge.
    bufferSize = nextPowerOf2(maxDelaySamples + 4);
    bufferMask = bufferSize - 1;

    // One-time heap allocation — this is called from prepareToPlay,
    // never from processBlock.
    buffer.resize(static_cast<size_t>(bufferSize), 0.0f);

    reset();
}

void DelayLine::reset()
{
    std::fill(buffer.begin(), buffer.end(), 0.0f);
    writeIndex = 0;
    apPrevInput = 0.0f;
    apPrevOutput = 0.0f;
}

void DelayLine::pushSample(float sample)
{
    buffer[static_cast<size_t>(writeIndex)] = sample;
    writeIndex = (writeIndex + 1) & bufferMask;
}

float DelayLine::readSample(float delaySamples)
{
    // Split the delay into integer and fractional parts.
    // Example: delaySamples = 183.49
    //   intDelay = 183  (read 183 samples back)
    //   frac     = 0.49 (allpass handles the remaining 0.49 samples)
    int intDelay = static_cast<int>(delaySamples);
    float frac = delaySamples - static_cast<float>(intDelay);

    // Read the sample at the integer delay position.
    // writeIndex points to where the NEXT sample will be written,
    // so we subtract (intDelay + 1) to read intDelay samples behind
    // the most recently written sample.
    int readPos = (writeIndex - intDelay - 1) & bufferMask;
    float sample = buffer[static_cast<size_t>(readPos)];

    // Apply allpass interpolation for the fractional part.
    // This gives us sub-sample accuracy for precise pitch tuning.
    float alpha = Interpolation::allpassCoefficient(frac);
    return Interpolation::allpassInterpolate(sample, alpha, apPrevInput, apPrevOutput);
}

int DelayLine::nextPowerOf2(int value)
{
    int result = 1;
    while (result < value)
        result <<= 1;
    return result;
}

} // namespace engine
