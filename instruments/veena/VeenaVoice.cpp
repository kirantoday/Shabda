#include "instruments/veena/VeenaVoice.h"
#include "instruments/veena/VeenaConfig.h"
#include <algorithm>

namespace veena {

void VeenaVoice::prepare(float newSampleRate, int maxBlockSize)
{
    sampleRate = newSampleRate;

    // Prepare all voices.
    for (auto& voice : voices)
    {
        voice.string.prepare(sampleRate, maxBlockSize);
        voice.string.setPluckPosition(DEFAULT_PLUCK_POSITION);
        voice.string.setDamping(DEFAULT_DAMPING);
        voice.string.setBrightness(DEFAULT_BRIGHTNESS);
        voice.string.setPluckStrength(DEFAULT_PLUCK_STRENGTH);
        voice.glideEngine.prepare(sampleRate);
        voice.glideEngine.setCurve(DEFAULT_GLIDE_CURVE);
        voice.midiNote = -1;
    }

    baseBrightness = DEFAULT_BRIGHTNESS;

    pitchBendEngine.prepare(sampleRate, maxBlockSize);
    pitchBendEngine.setBendRangeSemitones(DEFAULT_BEND_RANGE_SEMITONES);
    pitchBendEngine.setCurve(DEFAULT_BEND_CURVE);
    pitchBendEngine.setSmoothingTimeMs(DEFAULT_BEND_SMOOTHING_MS);

    midiMapper.prepare(sampleRate);
    midiMapper.setMaxVibratoDepth(DEFAULT_MAX_VIBRATO_DEPTH);

    vibratoLFO.prepare(sampleRate);
    vibratoLFO.setRate(DEFAULT_VIBRATO_RATE);

    bodyResonator.prepare(sampleRate, maxBlockSize);
    bodyResonator.setPreset(VEENA_BODY_MODES, VEENA_BODY_NUM_MODES);
    bodyResonator.setDryWetMix(DEFAULT_BODY_MIX);

    sympatheticBank.prepare(sampleRate, maxBlockSize);
    sympatheticBank.setTunings(THALAM_STRING_NOTES, NUM_THALAM_STRINGS);
    sympatheticBank.setGain(DEFAULT_SYMPATHETIC_GAIN);
    sympatheticBank.setFeedback(DEFAULT_SYMPATHETIC_FEEDBACK);
    sympatheticBank.setDamping(DEFAULT_SYMPATHETIC_DAMPING);

    legatoEnabled = DEFAULT_LEGATO_ENABLED;
    glideTimeMs = DEFAULT_LEGATO_GLIDE_MS;
    legatoGapThresholdSamples = sampleRate * LEGATO_GAP_THRESHOLD_MS / 1000.0f;
    aftertouchBrightnessRange = DEFAULT_AFTERTOUCH_BRIGHTNESS_RANGE;
    outputGain = OUTPUT_GAIN;
    lastMidiNote = -1;
    lastVoiceIndex = -1;
    lastNoteOnSample = 0;
    sampleCounter = 0;
}

void VeenaVoice::reset()
{
    for (auto& voice : voices)
    {
        voice.string.reset();
        voice.glideEngine.reset();
        voice.midiNote = -1;
    }
    pitchBendEngine.reset();
    midiMapper.reset();
    vibratoLFO.reset();
    bodyResonator.reset();
    sympatheticBank.reset();
    lastMidiNote = -1;
    lastVoiceIndex = -1;
    sampleCounter = 0;
    lastNoteOnSample = 0;
}

void VeenaVoice::noteOn(int midiNote, float velocity)
{
    float targetNote = static_cast<float>(midiNote);
    int64_t samplesSinceLastNote = sampleCounter - lastNoteOnSample;
    float gapMs = static_cast<float>(samplesSinceLastNote) * 1000.0f / sampleRate;

    // Decide: legato glide or retrigger pluck?
    bool shouldGlide = legatoEnabled
                       && lastVoiceIndex >= 0
                       && voices[static_cast<size_t>(lastVoiceIndex)].string.isActive()
                       && velocity < RETRIGGER_VELOCITY_THRESHOLD
                       && gapMs < LEGATO_GAP_THRESHOLD_MS;

    lastNoteOnSample = sampleCounter;

    if (shouldGlide)
    {
        // LEGATO: glide the most recent voice to the new note.
        auto& voice = voices[static_cast<size_t>(lastVoiceIndex)];
        voice.glideEngine.setGlide(voice.glideEngine.getCurrentNote(), targetNote, glideTimeMs);
        voice.midiNote = midiNote;
        lastMidiNote = midiNote;
    }
    else
    {
        // RETRIGGER: allocate a voice and pluck.
        int vi = findFreeVoice();
        if (vi < 0)
            vi = findOldestVoice();

        auto& voice = voices[static_cast<size_t>(vi)];
        voice.excitedBaseNote = targetNote;
        voice.glideEngine.snapTo(targetNote);
        voice.midiNote = midiNote;
        voice.noteOnSample = sampleCounter;

        pitchBendEngine.snapToCurrentValue();
        voice.string.noteOn(midiNote, velocity);

        lastMidiNote = midiNote;
        lastVoiceIndex = vi;
    }
}

void VeenaVoice::noteOff(int midiNote)
{
    // Release all voices playing this note.
    // We mark the voice as available for reallocation (midiNote = -1)
    // but DO NOT reset lastVoiceIndex — the string is still ringing
    // and should remain available for legato glide if the next note
    // arrives within the gap window.
    for (int i = 0; i < NUM_VOICES; ++i)
    {
        if (voices[static_cast<size_t>(i)].midiNote == midiNote)
        {
            voices[static_cast<size_t>(i)].string.noteOff();
            voices[static_cast<size_t>(i)].midiNote = -1;
        }
    }

    // Note: lastMidiNote and lastVoiceIndex are intentionally NOT cleared
    // here. They remain valid for legato until either (a) the gap timer
    // expires, or (b) the string fully decays (isActive() returns false).
}

void VeenaVoice::pitchBendMidi(int midiValue)
{
    pitchBendEngine.setMidiPitchBend(midiValue);
}

void VeenaVoice::pitchBend(float semitones)
{
    pitchBendEngine.setSemitoneOffset(semitones);
}

void VeenaVoice::handleCC(int cc, int value)
{
    midiMapper.handleControlChange(cc, value);
}

void VeenaVoice::handleAftertouch(int value)
{
    midiMapper.handleAftertouch(value);
}

void VeenaVoice::processBlock(float* outputBuffer, int numSamples)
{
    // Clear output — we'll sum voices into it.
    std::fill(outputBuffer, outputBuffer + numSamples, 0.0f);

    for (int i = 0; i < numSamples; ++i)
    {
        // --- Shared per-sample modulation ---
        float pitchBendOffset = pitchBendEngine.getNextPitchOffset();
        vibratoLFO.setDepth(midiMapper.getVibratoDepth());
        float vibratoOffset = vibratoLFO.getNextSample();
        float expressionGain = midiMapper.getExpressionGain();

        float aftertouch = midiMapper.getAftertouchValue();
        float modulatedBrightness = baseBrightness + aftertouch * aftertouchBrightnessRange;

        // --- Render each voice ---
        float voiceSum = 0.0f;

        for (int v = 0; v < NUM_VOICES; ++v)
        {
            auto& voice = voices[static_cast<size_t>(v)];

            if (!voice.string.isActive())
                continue;

            // Per-voice glide + shared pitch bend + vibrato
            float glideNote = voice.glideEngine.getNextNote();
            float glideOffset = glideNote - voice.excitedBaseNote;
            float totalPitch = glideOffset + pitchBendOffset + vibratoOffset;

            voice.string.setPitchOffset(totalPitch);
            voice.string.setBrightness(modulatedBrightness);

            voiceSum += voice.string.processSample();
        }

        outputBuffer[i] = voiceSum * outputGain * expressionGain;
        ++sampleCounter;
    }

    // Shared post-processing: body resonance and sympathetic drones.
    bodyResonator.processBlock(outputBuffer, numSamples);
    sympatheticBank.processBlock(outputBuffer, numSamples);
}

// --- Parameter setters ---

void VeenaVoice::setPluckPosition(float position)
{
    for (auto& voice : voices)
        voice.string.setPluckPosition(position);
}

void VeenaVoice::setDamping(float damping)
{
    for (auto& voice : voices)
        voice.string.setDamping(damping);
}

void VeenaVoice::setBrightness(float brightness)
{
    baseBrightness = brightness;
    for (auto& voice : voices)
        voice.string.setBrightness(brightness);
}

void VeenaVoice::setBendRange(float semitones)
{
    pitchBendEngine.setBendRangeSemitones(semitones);
}

void VeenaVoice::setBendCurve(engine::BendCurve curve)
{
    pitchBendEngine.setCurve(curve);
}

float VeenaVoice::getBendRange() const
{
    return pitchBendEngine.getBendRangeSemitones();
}

void VeenaVoice::setBodyMix(float mix)
{
    bodyResonator.setDryWetMix(mix);
}

void VeenaVoice::setSympatheticGain(float gain)
{
    sympatheticBank.setGain(gain);
}

void VeenaVoice::setVibratoDepth(float semitones)
{
    midiMapper.setVibratoDirect(semitones);
}

void VeenaVoice::setExpressionGain(float gain)
{
    midiMapper.setExpressionDirect(gain);
}

void VeenaVoice::setLegatoEnabled(bool enabled)
{
    legatoEnabled = enabled;
}

void VeenaVoice::setGlideCurve(engine::GlideCurve curve)
{
    for (auto& voice : voices)
        voice.glideEngine.setCurve(curve);
}

// --- Voice allocation ---

int VeenaVoice::findFreeVoice() const
{
    for (int i = 0; i < NUM_VOICES; ++i)
    {
        if (!voices[static_cast<size_t>(i)].string.isActive()
            && voices[static_cast<size_t>(i)].midiNote == -1)
            return i;
    }
    return -1;
}

int VeenaVoice::findOldestVoice() const
{
    int oldest = 0;
    int64_t oldestSample = voices[0].noteOnSample;

    for (int i = 1; i < NUM_VOICES; ++i)
    {
        if (voices[static_cast<size_t>(i)].noteOnSample < oldestSample)
        {
            oldest = i;
            oldestSample = voices[static_cast<size_t>(i)].noteOnSample;
        }
    }
    return oldest;
}

int VeenaVoice::findVoiceForNote(int midiNote) const
{
    for (int i = 0; i < NUM_VOICES; ++i)
    {
        if (voices[static_cast<size_t>(i)].midiNote == midiNote)
            return i;
    }
    return -1;
}

} // namespace veena
