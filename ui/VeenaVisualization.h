#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include "Theme.h"

// Stylized side-view illustration of the Veena at ~30 degree angle.
// Neck upper-left → kudam lower-right (natural playing position).
// Clean vector-style rendering with gold/navy color scheme.

class VeenaVisualization : public juce::Component,
                            private juce::Timer
{
public:
    VeenaVisualization();
    ~VeenaVisualization() override;

    void paint(juce::Graphics& g) override;

    void setStringAmplitude(int voiceIndex, float amplitude);
    void setActiveNote(int midiNote);
    void setPitchOffset(float semitones);
    void setThalamFlash(int stringIndex, float intensity);
    void setSympatheticLevel(float level);
    void setPeakLevel(float level);

private:
    void timerCallback() override;

    // Coordinate helpers: map normalized 0..1 along the veena's axis
    // to actual pixel coordinates at the 30-degree angle.
    juce::Point<float> veenaPoint(float t, float perpOffset = 0.0f) const;

    // Draw sub-elements
    void drawShadow(juce::Graphics& g);
    void drawNeckBeam(juce::Graphics& g);
    void drawKudam(juce::Graphics& g);
    void drawUpperGourd(juce::Graphics& g);
    void drawYali(juce::Graphics& g);
    void drawFrets(juce::Graphics& g);
    void drawBridge(juce::Graphics& g);
    void drawMainStrings(juce::Graphics& g);
    void drawThalamStrings(juce::Graphics& g);
    void drawFingerPosition(juce::Graphics& g);

    // The veena's main axis: from yali (upper-left) to kudam (lower-right)
    juce::Point<float> axisStart;  // yali end
    juce::Point<float> axisEnd;    // kudam end
    float axisLength = 0.0f;
    float axisAngle = 0.0f;        // radians

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
    float animPhase = 0.0f;

    static constexpr int NUM_FRETS = 24;
    static constexpr int BASE_MIDI_NOTE = 48;
};
