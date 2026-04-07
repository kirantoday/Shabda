#include "waveguide/PluckedString.h"
#include "common/DSPConstants.h"
#include <cmath>
#include <algorithm>

namespace engine {

void PluckedString::prepare(float newSampleRate, int maxBlockSize)
{
    sampleRate = newSampleRate;

    // Maximum delay needed: lowest MIDI note (21 = A0 ≈ 27.5 Hz) at current
    // sample rate, plus headroom for pitch bend down by MAX_PITCH_BEND_SEMITONES.
    // At 48kHz: 48000 / 27.5 ≈ 1745 samples. With 12 semitones down (halve freq),
    // that doubles to ~3490. We use 4096 for comfortable margin.
    int maxDelay = static_cast<int>(sampleRate / 20.0f) + 4; // ~20 Hz floor
    delayLine.prepare(sampleRate, maxDelay);

    loopFilter.prepare(sampleRate);
    exciter.prepare(sampleRate, maxBlockSize);

    // Preallocate excitation buffer for the longest possible period.
    excitationBuffer.resize(static_cast<size_t>(maxDelay), 0.0f);

    reset();
}

void PluckedString::reset()
{
    delayLine.reset();
    loopFilter.reset();
    dcBlocker.reset();
    exciter.reset();

    active = false;
    noteHeld = false;
    energyAccumulator = 0.0f;
    silenceCheckCounter = 0;
}

void PluckedString::noteOn(int midiNote, float velocity)
{
    currentMidiNote = midiNote;
    currentVelocity = velocity;
    noteHeld = true;

    // Calculate the base delay length for this note (no pitch offset).
    // baseDelayLength is stored so that setPitchOffset() can efficiently
    // compute the bent delay without recalculating from MIDI note each time.
    baseDelayLength = midiNoteToDelay(static_cast<float>(midiNote));
    currentDelayLength = baseDelayLength * std::exp2(-pitchOffsetSemitones / 12.0f);

    // Reset the delay line and filters for a clean start.
    // TODO: anti-click crossfade on retrigger (acceptable click for monophonic v1)
    delayLine.reset();
    loopFilter.reset();
    dcBlocker.reset();

    // Set up the loop filter for this note's damping.
    updateLoopFilter();

    // Generate the excitation signal (the "pluck").
    // Velocity maps to pluck strength: harder pluck = louder + brighter.
    float velocityStrength = pluckStrength * velocity;
    float velocityBrightness = brightness * (0.7f + 0.3f * velocity); // TODO(TUNE): velocity→brightness curve

    int excitationLength = exciter.generateExcitation(
        excitationBuffer.data(),
        static_cast<int>(excitationBuffer.size()),
        currentDelayLength,
        pluckPosition,
        velocityStrength,
        velocityBrightness
    );

    // Inject the excitation into the delay line.
    // This is the initial displacement of the string — from here,
    // the Karplus-Strong loop takes over and the sound evolves naturally.
    for (int i = 0; i < excitationLength; ++i)
        delayLine.pushSample(excitationBuffer[static_cast<size_t>(i)]);

    active = true;
    energyAccumulator = 1.0f;  // assume energy present
    silenceCheckCounter = 0;
}

void PluckedString::noteOff()
{
    noteHeld = false;
    // Increase damping to simulate the player damping the string with their finger.
    updateLoopFilter();
}

float PluckedString::processSample()
{
    if (!active)
        return 0.0f;

    // --- The Karplus-Strong loop ---
    //
    // 1. Read the sample that has traveled the full length of the string
    //    (one full period of the fundamental frequency).
    float sample = delayLine.readSample(currentDelayLength);

    // 2. Pass through the loop filter. This simulates the string's
    //    frequency-dependent energy loss. High harmonics are attenuated
    //    more on each pass, just like a real string.
    float filtered = loopFilter.processSample(sample);

    // 3. Write the filtered sample back into the delay line.
    //    This creates the feedback loop — the signal keeps circulating,
    //    getting slightly darker and quieter each time around.
    delayLine.pushSample(filtered);

    // 4. Remove any DC offset that may have accumulated.
    float output = dcBlocker.processSample(sample);

    // --- Silence detection ---
    energyAccumulator = std::max(energyAccumulator, std::abs(output));
    ++silenceCheckCounter;
    if (silenceCheckCounter >= SILENCE_CHECK_INTERVAL)
    {
        if (energyAccumulator < SILENCE_THRESHOLD)
            active = false;
        energyAccumulator = 0.0f;
        silenceCheckCounter = 0;
    }

    return output;
}

void PluckedString::processBlock(float* outputBuffer, int numSamples)
{
    // Convenience wrapper — when no per-sample pitch modulation is needed.
    // For pitch bending, the caller should use processSample() directly.
    for (int i = 0; i < numSamples; ++i)
        outputBuffer[i] = processSample();
}

void PluckedString::setPluckPosition(float position)
{
    pluckPosition = std::clamp(position, 0.05f, 0.5f);
}

void PluckedString::setDamping(float newDamping)
{
    damping = std::clamp(newDamping, 0.0f, 0.95f);
    if (active)
        updateLoopFilter();
}

void PluckedString::setBrightness(float newBrightness)
{
    brightness = std::clamp(newBrightness, 0.0f, 1.0f);
}

void PluckedString::setPluckStrength(float strength)
{
    pluckStrength = std::clamp(strength, 0.0f, 1.0f);
}

void PluckedString::setPitchOffset(float semitones)
{
    // Per-sample pitch modulation for meend/gamaka.
    //
    // Instead of calling midiNoteToDelay() (which uses std::pow and does
    // frequency clamping), we use the precomputed baseDelayLength and scale
    // it directly. This is efficient enough for per-sample calling.
    //
    // The math: bending up by S semitones multiplies frequency by 2^(S/12),
    // which divides the delay by the same factor:
    //   currentDelay = baseDelay * 2^(-S/12) = baseDelay * exp2(-S/12)
    pitchOffsetSemitones = semitones;

    if (active)
    {
        currentDelayLength = baseDelayLength * std::exp2(-semitones / 12.0f);

        // Clamp to safe range (prevent reading past buffer or zero-length delay).
        float minDelay = 2.0f;
        float maxDelay = static_cast<float>(delayLine.getBufferSize() - 4);
        currentDelayLength = std::clamp(currentDelayLength, minDelay, maxDelay);
    }
}

float PluckedString::midiNoteToDelay(float midiNote) const
{
    // Standard equal temperament tuning:
    //   frequency = 440 * 2^((midiNote - 69) / 12)
    //   delay = sampleRate / frequency
    //
    // MIDI note 69 = A4 = 440 Hz
    // Each semitone up multiplies frequency by 2^(1/12) ≈ 1.0595
    // Each semitone down divides frequency by the same factor
    float frequency = A4_FREQUENCY * std::pow(2.0f, (midiNote - static_cast<float>(A4_MIDI_NOTE)) / 12.0f);

    // Clamp to a reasonable range to avoid buffer overrun or zero-length delay.
    frequency = std::clamp(frequency, 20.0f, sampleRate * 0.45f);

    // TODO(TUNE): loop filter delay compensation
    // The loop filter and allpass interpolator each add a small amount of
    // delay (roughly 0.5 samples for the one-pole filter at low frequencies).
    // For perfect pitch accuracy, subtract this from the delay length.
    // For now, the error is small enough to be inaudible on most notes.
    return sampleRate / frequency;
}

void PluckedString::updateLoopFilter()
{
    // The loop filter coefficient controls how quickly high frequencies decay.
    // When the note is held, use the base damping value.
    // When released, boost toward noteOffDampingBoost for faster decay.
    float targetCoefficient = noteHeld ? damping : std::max(damping, noteOffDampingBoost);
    loopFilter.setCoefficient(targetCoefficient);
}

} // namespace engine
