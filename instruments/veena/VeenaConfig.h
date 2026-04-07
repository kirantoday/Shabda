#pragma once

#include "expression/PitchBendEngine.h"
#include "resonator/ModalFilter.h"

// Configuration constants for the Saraswati Veena.
// This file must NOT include any JUCE headers — pure C++ only.
//
// All defaults here are initial guesses based on the instrument's character.
// They WILL need ear-tuning against real veena recordings.
//
// EXTENSION: future instruments (sitar, dilruba, etc.) will have their own
// config headers with different defaults, sharing the same engine classes.

namespace veena {

// --- Default parameter values ---

// TODO(TUNE): pluck position — veena is plucked near the bridge for brightness
constexpr float DEFAULT_PLUCK_POSITION = 0.15f;

// TODO(TUNE): damping — moderate sustain, veena has good sustain from the kudam body
constexpr float DEFAULT_DAMPING = 0.4f;

// TODO(TUNE): brightness — veena has a bright, singing tone with prominent attack
constexpr float DEFAULT_BRIGHTNESS = 0.65f;

// TODO(TUNE): default pluck strength (scaled by velocity)
constexpr float DEFAULT_PLUCK_STRENGTH = 0.8f;

// TODO(TUNE): how much damping increases on note-off (finger damping)
constexpr float NOTE_OFF_DAMPING_BOOST = 0.85f;

// TODO(TUNE): master output gain to prevent clipping from a single string
constexpr float OUTPUT_GAIN = 0.7f;

// --- Tuning ---

// Standard veena Sa = C3 (MIDI 48), but this should be configurable.
constexpr int DEFAULT_SA_MIDI_NOTE = 48;

// Playable range for the main melody string
constexpr int LOWEST_MIDI_NOTE = 36;   // C2
constexpr int HIGHEST_MIDI_NOTE = 84;  // C6

// --- Pitch bend / meend ---

// TODO(TUNE): default pitch bend range in semitones for veena meend
// Veena gamaka can reach a 4th or 5th — 7 semitones covers most ornaments.
constexpr float DEFAULT_BEND_RANGE_SEMITONES = 7.0f;

// TODO(TUNE): pitch bend smoothing time in ms
// 5ms eliminates zipper noise while keeping bends responsive.
constexpr float DEFAULT_BEND_SMOOTHING_MS = 5.0f;

// Default curve shape for pitch bend
constexpr engine::BendCurve DEFAULT_BEND_CURVE = engine::BendCurve::Linear;

// --- Body resonance (kudam) ---

// TODO(TUNE): dry/wet mix for body resonance.
// 0.5 = balanced blend of raw string and body-resonated signal.
constexpr float DEFAULT_BODY_MIX = 0.5f;

// Number of modal resonances in the veena kudam body preset.
constexpr int VEENA_BODY_NUM_MODES = 6;

// TODO(TUNE): Veena kudam body resonance preset.
// The kudam is a large jackfruit wood gourd. These modal frequencies
// approximate its warm, woody character. All values need ear-tuning
// against real veena recordings.
//
// Mode 0: Fundamental body mode — the main low-end warmth of the kudam
// Mode 1: Air resonance (Helmholtz-like) — air cavity of the gourd
// Mode 2: Second wood mode — low-mid warmth and sustain
// Mode 3: Third wood mode — mid-range body color
// Mode 4: Wood "knock" — percussive attack presence
// Mode 5: Upper body mode — definition and clarity
constexpr engine::ModalFilterParams VEENA_BODY_MODES[VEENA_BODY_NUM_MODES] = {
    { 150.0f,  4.0f,  6.0f },   // TODO(TUNE): Mode 0 — fundamental body
    { 250.0f,  6.0f,  4.5f },   // TODO(TUNE): Mode 1 — air/Helmholtz
    { 420.0f,  5.0f,  3.0f },   // TODO(TUNE): Mode 2 — wood warmth
    { 680.0f,  7.0f,  2.0f },   // TODO(TUNE): Mode 3 — mid color
    { 1000.0f, 8.0f,  3.5f },   // TODO(TUNE): Mode 4 — wood knock
    { 2200.0f, 6.0f,  1.5f },   // TODO(TUNE): Mode 5 — upper definition
};

// --- Engine limits ---

// Silence threshold for deactivating a voice (saves CPU)
constexpr float SILENCE_THRESHOLD = 1.0e-6f;

// Max delay in samples — covers lowest note + pitch bend at 96kHz
constexpr int MAX_DELAY_SAMPLES = 4096;

// --- Sympathetic / thalam drone strings ---

// Thalam (side drone) string MIDI notes: Sa, Pa, sa.
// These are the 3 open strings that vibrate sympathetically when
// the main melody string plays harmonically related notes.
constexpr int THALAM_STRING_NOTES[] = { 48, 55, 60 };  // Sa(C3), Pa(G3), sa(C4)
constexpr int NUM_THALAM_STRINGS = 3;

// TODO(TUNE): sympathetic resonance gain — how loud the drone halo is.
// 0.15 = subtle shimmer, 0.3 = clearly audible, 0.5+ = prominent drone.
constexpr float DEFAULT_SYMPATHETIC_GAIN = 0.15f;

// TODO(TUNE): feedback controls how long the sympathetic strings ring.
// 0.98 = moderate ring (~1-2 sec), 0.995 = very long, 0.999 = near-infinite.
constexpr float DEFAULT_SYMPATHETIC_FEEDBACK = 0.98f;

// TODO(TUNE): damping in the sympathetic string feedback loop.
// Higher = darker sympathetic tone (high harmonics decay faster).
constexpr float DEFAULT_SYMPATHETIC_DAMPING = 0.3f;

// --- Expression mapping ---

// TODO(TUNE): default vibrato (kampita) rate in Hz.
// Typical kampita is 4-7 Hz. 5.5 Hz is a moderate starting point.
constexpr float DEFAULT_VIBRATO_RATE = 5.5f;

// TODO(TUNE): max vibrato depth in semitones when CC1 is at 127.
// 1.0 = moderate kampita, 2.0 = wide oscillation.
constexpr float DEFAULT_MAX_VIBRATO_DEPTH = 1.0f;

// TODO(TUNE): aftertouch brightness modulation range.
// At full aftertouch (127), brightness increases by this amount (0..1 scale).
constexpr float DEFAULT_AFTERTOUCH_BRIGHTNESS_RANGE = 0.3f;

} // namespace veena
