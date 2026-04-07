#pragma once

#include "waveguide/PluckedString.h"
#include "expression/PitchBendEngine.h"
#include "expression/MidiMapper.h"
#include "expression/VibratoLFO.h"
#include "resonator/BodyResonator.h"
#include "sympathetic/SympatheticBank.h"

// VeenaVoice: the top-level instrument class for the Saraswati Veena.
// This file must NOT include any JUCE headers — pure C++ only.
//
// VeenaVoice assembles all engine components into a complete instrument
// with veena-specific defaults and full MIDI expression mapping.
//
// Signal chain: PluckedString → BodyResonator → SympatheticBank → output
// Modulation:   PitchBend + VibratoLFO → pitch, MidiMapper → CC/aftertouch

namespace veena {

class VeenaVoice
{
public:
    void prepare(float sampleRate, int maxBlockSize);
    void reset();

    // --- MIDI event handlers (called from the plugin layer) ---

    void noteOn(int midiNote, float velocity);
    void noteOff(int midiNote);
    void pitchBendMidi(int midiValue);
    void pitchBend(float semitones);
    void handleCC(int cc, int value);
    void handleAftertouch(int value);

    // --- Audio rendering ---

    void processBlock(float* outputBuffer, int numSamples);

    // --- Parameter setters (called from plugin layer / UI) ---

    void setPluckPosition(float position);
    void setDamping(float damping);
    void setBrightness(float brightness);
    void setBendRange(float semitones);
    void setBendCurve(engine::BendCurve curve);
    float getBendRange() const;
    void setBodyMix(float mix);
    void setSympatheticGain(float gain);
    void setVibratoDepth(float semitones);
    void setExpressionGain(float gain);

private:
    engine::PluckedString string;
    engine::PitchBendEngine pitchBendEngine;
    engine::MidiMapper midiMapper;
    engine::VibratoLFO vibratoLFO;
    engine::BodyResonator bodyResonator;
    engine::SympatheticBank sympatheticBank;

    int lastMidiNote = -1;
    float outputGain = 0.7f;
    float baseBrightness = 0.65f;                     // base brightness before aftertouch
    float aftertouchBrightnessRange = 0.3f;           // TODO(TUNE): aftertouch modulation range
};

} // namespace veena
