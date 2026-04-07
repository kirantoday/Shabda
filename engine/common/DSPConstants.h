#pragma once

#include <cmath>

// DSP constants for the Veena physical modeling engine.
// This file must NOT include any JUCE headers — pure C++ only.

namespace engine {

constexpr float PI = 3.14159265358979323846f;
constexpr float TWO_PI = 2.0f * PI;

// Default sample rate — always overridden by prepare()
constexpr float DEFAULT_SAMPLE_RATE = 48000.0f;

// A4 reference frequency for tuning (Hz)
constexpr float A4_FREQUENCY = 440.0f;

// MIDI note number for A4
constexpr int A4_MIDI_NOTE = 69;

// Maximum number of main strings on any instrument in this engine
constexpr int MAX_MAIN_STRINGS = 7;

// Maximum number of sympathetic strings
constexpr int MAX_SYMPATHETIC_STRINGS = 40;

// Maximum pitch bend range in semitones (veena gamaka can reach +-7)
constexpr float MAX_PITCH_BEND_SEMITONES = 12.0f;

// Convert MIDI note number to frequency in Hz.
// Uses standard equal temperament: freq = 440 * 2^((note - 69) / 12)
// Shared utility — used by PluckedString, SympatheticBank, and others.
inline float midiNoteToFrequency(float midiNote)
{
    return A4_FREQUENCY * std::pow(2.0f, (midiNote - static_cast<float>(A4_MIDI_NOTE)) / 12.0f);
}

} // namespace engine
