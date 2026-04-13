#include "instruments/veena/VeenaVoice.h"
#include "instruments/veena/VeenaConfig.h"
#include <algorithm>
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

    convolutionBody.prepare(sampleRate, maxBlockSize);
    convolutionBody.setDryWetMix(DEFAULT_BODY_MIX);
    bodyMode = static_cast<int>(DEFAULT_BODY_MODE);

    sympatheticBank.prepare(sampleRate, maxBlockSize);
    sympatheticBank.setTunings(THALAM_STRING_NOTES, NUM_THALAM_STRINGS);
    sympatheticBank.setGain(DEFAULT_SYMPATHETIC_GAIN);
    sympatheticBank.setFeedback(DEFAULT_SYMPATHETIC_FEEDBACK);
    sympatheticBank.setDamping(DEFAULT_SYMPATHETIC_DAMPING);

    // Configure thalam (side drone) plucked strings.
    for (int i = 0; i < NUM_THALAM; ++i)
    {
        thalamStrings[static_cast<size_t>(i)].prepare(sampleRate, maxBlockSize);
        thalamStrings[static_cast<size_t>(i)].setDamping(THALAM_DAMPING);
        thalamStrings[static_cast<size_t>(i)].setBrightness(THALAM_BRIGHTNESS);
        thalamStrings[static_cast<size_t>(i)].setPluckPosition(THALAM_PLUCK_POSITION);
    }
    thalamVolume = DEFAULT_THALAM_VOLUME;

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
    for (auto& ts : thalamStrings)
        ts.reset();
    pitchBendEngine.reset();
    midiMapper.reset();
    vibratoLFO.reset();
    bodyResonator.reset();
    convolutionBody.reset();
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

        // Humanization: apply micro-variations on retrigger.
        if (humanizeEnabled)
        {
            // Pitch jitter: ±3 cents (converted to semitones: cents/100)
            voice.pitchJitterCents = nextRandom() * HUMANIZE_PITCH_CENTS / 100.0f;

            // Brightness jitter: ±10% of base brightness
            float brightnessJitter = 1.0f + nextRandom() * HUMANIZE_BRIGHTNESS_FRACTION;
            voice.string.setBrightness(baseBrightness * brightnessJitter);

            // Timing jitter: 0 to HUMANIZE_TIMING_MS delay before pluck fires.
            // Unipolar (only delay, never early) to avoid negative latency.
            int maxJitterSamples = static_cast<int>(sampleRate * HUMANIZE_TIMING_MS / 1000.0f);
            voice.jitterSamplesRemaining = static_cast<int>(nextRandomUnipolar() * static_cast<float>(maxJitterSamples));
            voice.pendingMidiNote = midiNote;
            voice.pendingVelocity = velocity;
        }
        else
        {
            voice.pitchJitterCents = 0.0f;
            voice.jitterSamplesRemaining = 0;
        }

        pitchBendEngine.snapToCurrentValue();

        // If no timing jitter, pluck immediately. Otherwise processBlock
        // will fire the pluck when the jitter countdown reaches 0.
        if (voice.jitterSamplesRemaining == 0)
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

            // Handle deferred pluck (humanization timing jitter).
            // The pluck fires when the jitter countdown reaches 0.
            if (voice.jitterSamplesRemaining > 0)
            {
                --voice.jitterSamplesRemaining;
                if (voice.jitterSamplesRemaining == 0)
                    voice.string.noteOn(voice.pendingMidiNote, voice.pendingVelocity);
            }

            if (!voice.string.isActive())
                continue;

            // Per-voice glide + shared pitch bend + vibrato + humanization pitch jitter
            float glideNote = voice.glideEngine.getNextNote();
            float glideOffset = glideNote - voice.excitedBaseNote;
            float totalPitch = glideOffset + pitchBendOffset + vibratoOffset + voice.pitchJitterCents;

            voice.string.setPitchOffset(totalPitch);
            voice.string.setBrightness(modulatedBrightness);

            voiceSum += voice.string.processSample();
        }

        // Add thalam strings (no pitch modulation — they're open strings).
        float thalamSum = 0.0f;
        for (int t = 0; t < NUM_THALAM; ++t)
        {
            if (thalamStrings[static_cast<size_t>(t)].isActive())
                thalamSum += thalamStrings[static_cast<size_t>(t)].processSample();
        }

        outputBuffer[i] = (voiceSum * outputGain + thalamSum * thalamVolume) * expressionGain;
        ++sampleCounter;
    }

    // Shared post-processing: body resonance.
    if (bodyMode == 0)
    {
        // Modal filters only.
        bodyResonator.processBlock(outputBuffer, numSamples);
    }
    else if (bodyMode == 1)
    {
        // Convolution only.
        convolutionBody.processBlock(outputBuffer, numSamples);
    }
    else
    {
        // Hybrid: process a copy through each, blend 50/50.
        // Use a stack buffer for the convolution path to avoid allocation.
        // For large block sizes, fall back to modal-only (safety).
        if (numSamples <= 2048)
        {
            float convBuf[2048];
            std::copy(outputBuffer, outputBuffer + numSamples, convBuf);

            bodyResonator.processBlock(outputBuffer, numSamples);
            convolutionBody.processBlock(convBuf, numSamples);

            for (int i = 0; i < numSamples; ++i)
                outputBuffer[i] = 0.5f * outputBuffer[i] + 0.5f * convBuf[i];
        }
        else
        {
            bodyResonator.processBlock(outputBuffer, numSamples);
        }
    }

    // Sympathetic resonance: shimmering drone halo.
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
    convolutionBody.setDryWetMix(mix);
}

void VeenaVoice::setBodyMode(int mode)
{
    bodyMode = std::clamp(mode, 0, 2);
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

// --- Thalam strings ---

void VeenaVoice::thalamNoteOn(int midiNote, float velocity)
{
    // Find which thalam string this note triggers.
    for (int i = 0; i < NUM_THALAM; ++i)
    {
        if (THALAM_TRIGGER_NOTES[i] == midiNote)
        {
            // Pluck at the fixed thalam tuning note, not the trigger note
            // (they happen to be the same in default config, but this
            // allows remapping triggers independently of tuning).
            thalamStrings[static_cast<size_t>(i)].noteOn(THALAM_STRING_NOTES[i], velocity);
            return;
        }
    }
}

void VeenaVoice::thalamNoteOff(int midiNote)
{
    for (int i = 0; i < NUM_THALAM; ++i)
    {
        if (THALAM_TRIGGER_NOTES[i] == midiNote)
        {
            thalamStrings[static_cast<size_t>(i)].noteOff();
            return;
        }
    }
}

bool VeenaVoice::isThalamNote(int midiNote)
{
    for (int i = 0; i < NUM_THALAM_PLUCKED; ++i)
    {
        if (THALAM_TRIGGER_NOTES[i] == midiNote)
            return true;
    }
    return false;
}

void VeenaVoice::setThalamVolume(float volume)
{
    thalamVolume = std::clamp(volume, 0.0f, 1.0f);
}

void VeenaVoice::setRagaPreset(int presetIndex)
{
    if (presetIndex < 0 || presetIndex >= NUM_RAGA_PRESETS)
        return;

    const auto& preset = RAGA_PRESETS[presetIndex];

    // Compute absolute MIDI notes from Sa base + raga offsets.
    int notes[SYMPATHETIC_NOTES_PER_PRESET];
    for (int i = 0; i < SYMPATHETIC_NOTES_PER_PRESET; ++i)
        notes[i] = DEFAULT_SA_MIDI_NOTE + preset.offsets[i];

    sympatheticBank.setTunings(notes, SYMPATHETIC_NOTES_PER_PRESET);
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

// --- PRNG (xorshift32, no stdlib) ---

float VeenaVoice::nextRandom()
{
    // Returns -1..+1
    rngState ^= rngState << 13;
    rngState ^= rngState >> 17;
    rngState ^= rngState << 5;
    return static_cast<float>(rngState) / 2147483648.0f - 1.0f;
}

float VeenaVoice::nextRandomUnipolar()
{
    // Returns 0..+1
    return (nextRandom() + 1.0f) * 0.5f;
}

} // namespace veena
