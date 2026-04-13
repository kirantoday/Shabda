#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include "Theme.h"
#include <array>

// Animated top-down visualization of the Veena.
// Shows main strings, thalam strings, frets, kudam body, and
// real-time feedback: string vibration, finger position, sympathetic shimmer.
//
// Driven by the audio thread writing state into atomic floats,
// and a 30fps timer for smooth animation.

class VeenaVisualization : public juce::Component,
                            private juce::Timer
{
public:
    VeenaVisualization();
    ~VeenaVisualization() override;

    void paint(juce::Graphics& g) override;

    // --- State setters (called from processor/editor, thread-safe) ---

    // Main string vibration amplitude (0..1) for each of 2 polyphonic voices.
    void setStringAmplitude(int voiceIndex, float amplitude);

    // Current MIDI note being played (for fret highlighting).
    void setActiveNote(int midiNote);

    // Pitch offset in semitones (for finger position sliding).
    void setPitchOffset(float semitones);

    // Thalam string flash (0..1, decays to 0).
    void setThalamFlash(int stringIndex, float intensity);

    // Sympathetic resonance intensity (0..1).
    void setSympatheticLevel(float level);

    // Peak output level (for kudam body glow).
    void setPeakLevel(float level);

private:
    void timerCallback() override;

    // Draw sub-components
    void drawNeck(juce::Graphics& g, juce::Rectangle<float> area);
    void drawFrets(juce::Graphics& g, juce::Rectangle<float> neckArea);
    void drawMainStrings(juce::Graphics& g, juce::Rectangle<float> neckArea);
    void drawThalamStrings(juce::Graphics& g, juce::Rectangle<float> area);
    void drawKudam(juce::Graphics& g, juce::Rectangle<float> area);
    void drawSympatheticStrings(juce::Graphics& g, juce::Rectangle<float> neckArea);
    void drawFingerPosition(juce::Graphics& g, juce::Rectangle<float> neckArea);

    // Animation state
    float stringAmplitudes[2] = { 0.0f, 0.0f };
    float smoothedAmplitudes[2] = { 0.0f, 0.0f };
    int activeNote = -1;
    float pitchOffset = 0.0f;
    float thalamFlash[3] = { 0.0f, 0.0f, 0.0f };
    float sympatheticLevel = 0.0f;
    float smoothedSympathetic = 0.0f;
    float peakLevel = 0.0f;
    float smoothedPeak = 0.0f;

    // Animation phase for string vibration
    float animPhase = 0.0f;

    static constexpr int NUM_FRETS = 24;
    static constexpr int BASE_MIDI_NOTE = 48;  // Sa = C3
};
