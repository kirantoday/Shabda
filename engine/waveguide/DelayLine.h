#pragma once

#include <vector>

// Circular buffer delay line with allpass fractional delay interpolation.
// This file must NOT include any JUCE headers — pure C++ only.
//
// HOW IT WORKS:
// A delay line stores N past samples in a circular buffer. A write pointer
// advances forward each sample. To read a delayed sample, we look backwards
// from the write pointer by `delay` samples. Since musical pitches rarely
// correspond to exact integer sample delays, we use allpass interpolation
// for the fractional part.
//
// The buffer size is always a power of 2, so we can use fast bitwise AND
// masking instead of expensive modulo for circular wrapping.

namespace engine {

class DelayLine
{
public:
    DelayLine() = default;
    ~DelayLine() = default;

    // Allocate the internal buffer. Must be called before any audio processing.
    // maxDelaySamples: the longest delay this line needs to support.
    // The actual buffer will be rounded up to the next power of 2.
    void prepare(float sampleRate, int maxDelaySamples);

    // Clear the buffer and reset all state.
    void reset();

    // Write one sample at the current write position and advance the pointer.
    void pushSample(float sample);

    // Read a sample from `delaySamples` behind the write position,
    // using allpass interpolation for the fractional part.
    // delaySamples must be >= 1.0 and < bufferSize.
    float readSample(float delaySamples);

    int getBufferSize() const { return bufferSize; }

private:
    std::vector<float> buffer;
    int bufferSize = 0;
    int bufferMask = 0;       // bufferSize - 1, for fast circular wrapping
    int writeIndex = 0;

    // Allpass interpolation state (persistent between calls)
    float apPrevInput = 0.0f;
    float apPrevOutput = 0.0f;

    float sampleRate = 48000.0f;

    // Round up to the next power of 2.
    static int nextPowerOf2(int value);
};

} // namespace engine
