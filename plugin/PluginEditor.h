#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_audio_utils/juce_audio_utils.h>
#include "../ui/VeenaLookAndFeel.h"
#include "../ui/VeenaVisualization.h"
#include "../ui/Theme.h"

class VeenaPluginProcessor;

class VeenaPluginEditor : public juce::AudioProcessorEditor,
                           private juce::Slider::Listener,
                           private juce::ComboBox::Listener,
                           private juce::Timer
{
public:
    explicit VeenaPluginEditor(VeenaPluginProcessor&);
    ~VeenaPluginEditor() override;

    void paint(juce::Graphics&) override;
    void resized() override;

private:
    void sliderValueChanged(juce::Slider* slider) override;
    void sliderDragEnded(juce::Slider* slider) override;
    void comboBoxChanged(juce::ComboBox* combo) override;
    void timerCallback() override;
    bool keyPressed(const juce::KeyPress& key) override;
    bool keyStateChanged(bool isKeyDown) override;

    // Helper to create a styled label
    juce::Label* makeLabel(const juce::String& text, float fontSize, juce::Colour col,
                           juce::Justification just = juce::Justification::centred);

    VeenaPluginProcessor& processorRef;
    VeenaLookAndFeel veenaLnF;

    // --- Header ---
    std::unique_ptr<juce::Label> logoLabel;
    std::unique_ptr<juce::Label> instrumentLabel;
    juce::ComboBox presetCombo;
    juce::TextButton presetPrev { "<" };
    juce::TextButton presetNext { ">" };

    // --- Visualization ---
    VeenaVisualization visualization;

    // --- Controls: String ---
    juce::Slider pluckPositionKnob, dampingKnob, brightnessKnob;
    std::unique_ptr<juce::Label> pluckPosLabel, dampingLabel, brightnessLabel;
    std::unique_ptr<juce::Label> pluckPosValue, dampingValue, brightnessValue;
    std::unique_ptr<juce::Label> stringGroupLabel;

    // --- Controls: Expression ---
    juce::Slider pitchBendStrip, expressionStrip, vibratoKnob;
    std::unique_ptr<juce::Label> pitchBendLabel, expressionLabel, vibratoLabel;
    std::unique_ptr<juce::Label> pitchBendValue, expressionValue, vibratoValue;
    std::unique_ptr<juce::Label> exprGroupLabel;

    // --- Controls: Resonance & Settings ---
    juce::Slider bodyMixKnob, sympatheticKnob, thalamKnob;
    std::unique_ptr<juce::Label> bodyMixLabel, sympatheticLabel, thalamLabel;
    std::unique_ptr<juce::Label> bodyMixValue, sympatheticValue, thalamValue;
    std::unique_ptr<juce::Label> resonGroupLabel, settingsGroupLabel;

    juce::ComboBox bodyModeCombo, ragaCombo, tuningCombo, bendRangeCombo, glideCurveCombo;
    juce::ToggleButton legatoToggle;

    // --- Keyboard ---
    juce::MidiKeyboardComponent keyboardComponent;

    // --- Level meter ---
    float currentPeak = 0.0f;

    // Thalam key tracking
    bool thalamKeyState[3] = { false, false, false };

    // Owned labels collection for cleanup
    juce::OwnedArray<juce::Label> ownedLabels;
};
