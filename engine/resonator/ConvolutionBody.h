#pragma once

#include <vector>

// IR convolution body resonance — time-domain implementation.
// This file must NOT include any JUCE headers — pure C++ only.
//
// HOW CONVOLUTION BODY RESONANCE WORKS:
//
// An impulse response (IR) captures how a physical body (the veena kudam)
// responds to a single impulse — the body's "fingerprint." Convolving the
// dry string signal with this IR reproduces the body's resonance character
// exactly, including all modes, their interactions, and the decay envelope.
//
// Compared to the modal filter bank (which approximates body modes as
// independent biquad filters), convolution captures:
//   - Mode coupling (modes interact and transfer energy)
//   - Exact decay shape (not just exponential per mode)
//   - Phase relationships between modes
//
// IMPLEMENTATION: Overlap-save (also called overlap-scrap) method.
// The input is buffered in segments of `blockSize` samples. Each segment
// is convolved with the IR in the time domain. This avoids the latency
// and complexity of FFT-based convolution for short IRs (< 4096 samples).
//
// For IRs up to ~4096 samples at 48kHz (~85ms), direct time-domain
// convolution is efficient enough. Each output sample requires `irLength`
// multiply-accumulates. At 4096 taps and 48kHz mono, that's ~200M MACs/sec
// — feasible on modern CPUs but not free. Use 2048 for safety.
//
// DEFAULT IR: A synthetic IR is generated algorithmically from decaying
// sinusoids at the 6 modal frequencies of the veena kudam. This gives a
// working default without needing an external WAV file.
// TODO(TUNE): replace with a real recorded IR from an actual veena body.

namespace engine {

// Maximum IR length in samples. 2048 @ 48kHz ≈ 43ms — sufficient for
// body impulse responses (not room reverbs).
constexpr int MAX_CONVOLUTION_IR_LENGTH = 2048;

class ConvolutionBody
{
public:
    void prepare(float sampleRate, int maxBlockSize);
    void reset();

    // Load an IR from a raw float buffer. Length will be clamped to
    // MAX_CONVOLUTION_IR_LENGTH. The data is copied internally.
    void loadIR(const float* irData, int length);

    // Generate a synthetic veena-like body IR from decaying sinusoids.
    // Uses the 6 modal frequencies from the kudam body preset.
    void generateSyntheticIR(float sampleRate);

    // Dry/wet mix: 0.0 = fully dry, 1.0 = fully convolved.
    void setDryWetMix(float mix);
    float getDryWetMix() const { return dryWetMix; }

    // Process a buffer in-place.
    void processBlock(float* buffer, int numSamples);

    int getIRLength() const { return irLength; }

private:
    // The impulse response (preallocated to MAX_CONVOLUTION_IR_LENGTH).
    std::vector<float> ir;
    int irLength = 0;

    // Circular input history buffer for time-domain convolution.
    // Stores the last `irLength` input samples.
    std::vector<float> inputHistory;
    int historyWritePos = 0;

    float dryWetMix = 0.5f;
    float sampleRate = 48000.0f;
};

} // namespace engine
