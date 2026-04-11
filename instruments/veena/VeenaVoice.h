#pragma once

#include "waveguide/PluckedString.h"
#include "expression/PitchBendEngine.h"
#include "expression/MidiMapper.h"
#include "expression/VibratoLFO.h"
#include "expression/GlideEngine.h"
#include "resonator/BodyResonator.h"
#include "sympathetic/SympatheticBank.h"
#include <array>
#include <cstdint>

// VeenaVoice: the top-level instrument class for the Saraswati Veena.
// This file must NOT include any JUCE headers — pure C++ only.
//
// ARCHITECTURE:
// Multiple Voice instances (each with PluckedString + GlideEngine) share a
// single BodyResonator + SympatheticBank. This is physically correct — the
// veena has one body that resonates all strings.
//
// Signal chain per voice: PluckedString → sum all voices → BodyResonator → SympatheticBank
// Modulation (shared):    PitchBend + VibratoLFO → pitch offset (all voices)
//                         MidiMapper → CC/aftertouch
//
// LEGATO: When a new note arrives while a voice is sounding, the pitch
// glides smoothly using GlideEngine (with configurable curve shape).
//
// POLYPHONY: Up to MAX_VOICES simultaneous notes. Voice allocation uses
// last-note priority with oldest-voice stealing when all voices are busy.

namespace veena {

class VeenaVoice
{
public:
    void prepare(float sampleRate, int maxBlockSize);
    void reset();

    // --- MIDI event handlers ---

    void noteOn(int midiNote, float velocity);
    void noteOff(int midiNote);
    void pitchBendMidi(int midiValue);
    void pitchBend(float semitones);
    void handleCC(int cc, int value);
    void handleAftertouch(int value);

    // --- Audio rendering ---

    void processBlock(float* outputBuffer, int numSamples);

    // --- Parameter setters ---

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
    void setLegatoEnabled(bool enabled);
    void setGlideCurve(engine::GlideCurve curve);

private:
    // --- Per-voice state ---
    struct Voice
    {
        engine::PluckedString string;
        engine::GlideEngine glideEngine;
        float excitedBaseNote = 60.0f;  // note the string was plucked at
        int midiNote = -1;              // currently assigned MIDI note (-1 = free)
        int64_t noteOnSample = 0;       // when this voice was triggered
    };

    static constexpr int NUM_VOICES = 2;  // matches VeenaConfig::MAX_VOICES
    std::array<Voice, NUM_VOICES> voices;

    // --- Shared components (one body, all strings resonate through it) ---
    engine::PitchBendEngine pitchBendEngine;
    engine::MidiMapper midiMapper;
    engine::VibratoLFO vibratoLFO;
    engine::BodyResonator bodyResonator;
    engine::SympatheticBank sympatheticBank;

    // --- Legato state ---
    bool legatoEnabled = true;
    int lastMidiNote = -1;             // last note played (for legato tracking)
    int lastVoiceIndex = -1;           // voice that played lastMidiNote
    int64_t lastNoteOnSample = 0;
    int64_t sampleCounter = 0;
    float legatoGapThresholdSamples = 9600.0f;
    float glideTimeMs = 80.0f;

    float sampleRate = 48000.0f;
    float outputGain = 0.7f;
    float baseBrightness = 0.65f;
    float aftertouchBrightnessRange = 0.3f;

    // Voice allocation helpers
    int findFreeVoice() const;
    int findOldestVoice() const;
    int findVoiceForNote(int midiNote) const;
};

} // namespace veena
