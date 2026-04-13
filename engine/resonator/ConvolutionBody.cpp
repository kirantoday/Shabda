#include "resonator/ConvolutionBody.h"
#include "common/DSPConstants.h"
#include <algorithm>
#include <cmath>

namespace engine {

void ConvolutionBody::prepare(float newSampleRate, int /*maxBlockSize*/)
{
    sampleRate = newSampleRate;

    // Preallocate IR and history buffers to max size.
    ir.resize(static_cast<size_t>(MAX_CONVOLUTION_IR_LENGTH), 0.0f);
    inputHistory.resize(static_cast<size_t>(MAX_CONVOLUTION_IR_LENGTH), 0.0f);

    // Generate default synthetic IR.
    generateSyntheticIR(sampleRate);

    reset();
}

void ConvolutionBody::reset()
{
    std::fill(inputHistory.begin(), inputHistory.end(), 0.0f);
    historyWritePos = 0;
}

void ConvolutionBody::loadIR(const float* irData, int length)
{
    irLength = std::min(length, MAX_CONVOLUTION_IR_LENGTH);

    std::copy(irData, irData + irLength, ir.begin());
    // Zero the rest.
    std::fill(ir.begin() + irLength, ir.end(), 0.0f);

    reset();
}

void ConvolutionBody::generateSyntheticIR(float sr)
{
    // Synthesize a veena kudam body IR from decaying sinusoids at the
    // 6 modal frequencies used in the biquad filter bank preset.
    //
    // Each mode is a damped sine: A * exp(-decay * t) * sin(2*pi*f*t)
    // The result captures mode interactions and phase relationships
    // that independent biquad filters cannot reproduce.
    //
    // TODO(TUNE): frequencies, amplitudes, and decay rates are initial
    // guesses. Replace with a real recorded IR from an actual veena.

    struct Mode {
        float freqHz;
        float amplitude;
        float decayRate;  // higher = faster decay
    };

    // TODO(TUNE): synthetic IR modal parameters
    const Mode modes[] = {
        { 150.0f,  0.30f, 15.0f },   // Fundamental body mode
        { 250.0f,  0.25f, 20.0f },   // Air/Helmholtz resonance
        { 420.0f,  0.15f, 30.0f },   // Second wood mode
        { 680.0f,  0.10f, 40.0f },   // Third wood mode
        { 1000.0f, 0.12f, 50.0f },   // Wood "knock"
        { 2200.0f, 0.05f, 70.0f },   // Upper definition
    };

    // Use a shorter IR length for CPU efficiency.
    // 1024 samples @ 48kHz ≈ 21ms — captures the body's initial response.
    irLength = std::min(1024, MAX_CONVOLUTION_IR_LENGTH);

    // Clear IR buffer.
    std::fill(ir.begin(), ir.begin() + irLength, 0.0f);

    // Sum decaying sinusoids for each mode.
    for (const auto& mode : modes)
    {
        float w = TWO_PI * mode.freqHz / sr;

        for (int i = 0; i < irLength; ++i)
        {
            float t = static_cast<float>(i) / sr;
            float envelope = std::exp(-mode.decayRate * t);
            ir[static_cast<size_t>(i)] += mode.amplitude * envelope * std::sin(w * static_cast<float>(i));
        }
    }

    // Normalize the IR so the peak doesn't exceed 1.0.
    float peak = 0.0f;
    for (int i = 0; i < irLength; ++i)
        peak = std::max(peak, std::abs(ir[static_cast<size_t>(i)]));

    if (peak > 0.0f)
    {
        float normFactor = 1.0f / peak;
        for (int i = 0; i < irLength; ++i)
            ir[static_cast<size_t>(i)] *= normFactor;
    }
}

void ConvolutionBody::setDryWetMix(float mix)
{
    dryWetMix = std::clamp(mix, 0.0f, 1.0f);
}

void ConvolutionBody::processBlock(float* buffer, int numSamples)
{
    if (irLength == 0 || dryWetMix < 1.0e-6f)
        return;

    for (int i = 0; i < numSamples; ++i)
    {
        float dryInput = buffer[i];

        // Write the input sample into the circular history buffer.
        inputHistory[static_cast<size_t>(historyWritePos)] = dryInput;

        // Time-domain convolution: dot product of input history and IR.
        // The IR is applied in reverse: ir[0] * newest + ir[1] * previous...
        float wet = 0.0f;
        int readPos = historyWritePos;

        for (int j = 0; j < irLength; ++j)
        {
            wet += inputHistory[static_cast<size_t>(readPos)] * ir[static_cast<size_t>(j)];

            --readPos;
            if (readPos < 0)
                readPos = irLength - 1;
        }

        // Advance write position.
        ++historyWritePos;
        if (historyWritePos >= irLength)
            historyWritePos = 0;

        // Mix dry and wet.
        buffer[i] = (1.0f - dryWetMix) * dryInput + dryWetMix * wet;
    }
}

} // namespace engine
