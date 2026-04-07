#include "instruments/veena/VeenaVoice.h"
#include "instruments/veena/VeenaConfig.h"

namespace veena {

void VeenaVoice::prepare(float sampleRate, int maxBlockSize)
{
    string.prepare(sampleRate, maxBlockSize);

    // Apply veena default parameters.
    string.setPluckPosition(DEFAULT_PLUCK_POSITION);
    string.setDamping(DEFAULT_DAMPING);
    string.setBrightness(DEFAULT_BRIGHTNESS);
    string.setPluckStrength(DEFAULT_PLUCK_STRENGTH);
    baseBrightness = DEFAULT_BRIGHTNESS;

    // Configure pitch bend engine with veena defaults.
    pitchBendEngine.prepare(sampleRate, maxBlockSize);
    pitchBendEngine.setBendRangeSemitones(DEFAULT_BEND_RANGE_SEMITONES);
    pitchBendEngine.setCurve(DEFAULT_BEND_CURVE);
    pitchBendEngine.setSmoothingTimeMs(DEFAULT_BEND_SMOOTHING_MS);

    // Configure MIDI CC mapper.
    midiMapper.prepare(sampleRate);
    midiMapper.setMaxVibratoDepth(DEFAULT_MAX_VIBRATO_DEPTH);

    // Configure vibrato LFO (kampita).
    vibratoLFO.prepare(sampleRate);
    vibratoLFO.setRate(DEFAULT_VIBRATO_RATE);

    // Configure body resonance with veena kudam preset.
    bodyResonator.prepare(sampleRate, maxBlockSize);
    bodyResonator.setPreset(VEENA_BODY_MODES, VEENA_BODY_NUM_MODES);
    bodyResonator.setDryWetMix(DEFAULT_BODY_MIX);

    // Configure sympathetic drone strings with veena thalam tuning.
    sympatheticBank.prepare(sampleRate, maxBlockSize);
    sympatheticBank.setTunings(THALAM_STRING_NOTES, NUM_THALAM_STRINGS);
    sympatheticBank.setGain(DEFAULT_SYMPATHETIC_GAIN);
    sympatheticBank.setFeedback(DEFAULT_SYMPATHETIC_FEEDBACK);
    sympatheticBank.setDamping(DEFAULT_SYMPATHETIC_DAMPING);

    aftertouchBrightnessRange = DEFAULT_AFTERTOUCH_BRIGHTNESS_RANGE;
    outputGain = OUTPUT_GAIN;
    lastMidiNote = -1;
}

void VeenaVoice::reset()
{
    string.reset();
    pitchBendEngine.reset();
    midiMapper.reset();
    vibratoLFO.reset();
    bodyResonator.reset();
    sympatheticBank.reset();
    lastMidiNote = -1;
}

void VeenaVoice::noteOn(int midiNote, float velocity)
{
    lastMidiNote = midiNote;
    pitchBendEngine.snapToCurrentValue();
    string.noteOn(midiNote, velocity);
}

void VeenaVoice::noteOff(int midiNote)
{
    if (midiNote == lastMidiNote)
    {
        string.noteOff();
        lastMidiNote = -1;
    }
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
    for (int i = 0; i < numSamples; ++i)
    {
        // --- Per-sample modulation ---

        // Pitch: combine pitch bend (meend) + vibrato LFO (kampita).
        float pitchOffset = pitchBendEngine.getNextPitchOffset();

        // Update vibrato depth from CC1 (smoothed by MidiMapper).
        vibratoLFO.setDepth(midiMapper.getVibratoDepth());
        pitchOffset += vibratoLFO.getNextSample();

        string.setPitchOffset(pitchOffset);

        // Expression gain from CC11 (smoothed).
        float expressionGain = midiMapper.getExpressionGain();

        // Aftertouch → brightness modulation.
        float aftertouch = midiMapper.getAftertouchValue();
        float modulatedBrightness = baseBrightness + aftertouch * aftertouchBrightnessRange;
        string.setBrightness(modulatedBrightness);

        // Render one sample.
        outputBuffer[i] = string.processSample() * outputGain * expressionGain;
    }

    // Body resonance: warm, woody kudam character.
    bodyResonator.processBlock(outputBuffer, numSamples);

    // Sympathetic resonance: shimmering drone halo.
    sympatheticBank.processBlock(outputBuffer, numSamples);
}

void VeenaVoice::setPluckPosition(float position)
{
    string.setPluckPosition(position);
}

void VeenaVoice::setDamping(float damping)
{
    string.setDamping(damping);
}

void VeenaVoice::setBrightness(float brightness)
{
    baseBrightness = brightness;
    string.setBrightness(brightness);
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
    vibratoLFO.setDepth(semitones);
}

void VeenaVoice::setExpressionGain(float gain)
{
    // Direct expression gain — bypasses MidiMapper smoothing.
    // Used by UI slider.
    midiMapper.handleControlChange(11, static_cast<int>(gain * 127.0f));
}

} // namespace veena
