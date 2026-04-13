#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include "Theme.h"

// SVG-based visualization of the Veena with interactive overlays.
//
// Architecture: static SVG base (the instrument illustration) with
// dynamic overlays painted on top (string glow, finger position,
// pluck flash, sympathetic shimmer). This separates art from animation.

class VeenaVisualization : public juce::Component,
                            private juce::Timer
{
public:
    VeenaVisualization();
    ~VeenaVisualization() override;

    void paint(juce::Graphics& g) override;
    void resized() override;

    void setStringAmplitude(int voiceIndex, float amplitude);
    void setActiveNote(int midiNote);
    void setPitchOffset(float semitones);
    void setThalamFlash(int stringIndex, float intensity);
    void setSympatheticLevel(float level);
    void setPeakLevel(float level);

private:
    void timerCallback() override;
    void loadSVG();

    // Map a position in SVG coordinates to component pixel coordinates.
    juce::Point<float> svgToComponent(float svgX, float svgY) const;

    // Map a fret/string position to component coordinates.
    // stringIndex 0-3, fretPosition 0.0 to 1.0 along the playing range.
    juce::Point<float> stringPosition(int stringIndex, float fretFraction) const;

    // Draw interactive overlays on top of the SVG
    void drawStringGlow(juce::Graphics& g);
    void drawFingerPosition(juce::Graphics& g);
    void drawPluckFlash(juce::Graphics& g);
    void drawKudamGlow(juce::Graphics& g);

    // The SVG drawable (loaded once)
    std::unique_ptr<juce::Drawable> svgDrawable;

    // Scale/offset for SVG → component mapping
    float svgScale = 1.0f;
    float svgOffsetX = 0.0f;
    float svgOffsetY = 0.0f;

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
