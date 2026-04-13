#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_audio_utils/juce_audio_utils.h>

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

    VeenaPluginProcessor& processorRef;

    juce::Label titleLabel;
    juce::MidiKeyboardComponent keyboardComponent;

    // --- String controls ---
    juce::Label stringGroupLabel;
    juce::Slider pluckPositionSlider;
    juce::Label pluckPositionLabel;
    juce::Slider dampingSlider;
    juce::Label dampingLabel;
    juce::Slider brightnessSlider;
    juce::Label brightnessLabel;

    // --- Body & resonance ---
    juce::Label bodyGroupLabel;
    juce::Slider bodyMixSlider;
    juce::Label bodyMixLabel;
    juce::Slider sympatheticSlider;
    juce::Label sympatheticLabel;

    // --- Thalam ---
    juce::Label thalamGroupLabel;
    juce::Slider thalamVolumeSlider;
    juce::Label thalamVolumeLabel;
    juce::Label thalamHintLabel;

    // Track Z/X/C key states for thalam noteOn/noteOff.
    bool thalamKeyState[3] = { false, false, false };

    // --- Expression controls ---
    juce::Label exprGroupLabel;
    juce::Slider pitchBendSlider;
    juce::Label pitchBendLabel;
    juce::Slider vibratoSlider;
    juce::Label vibratoLabel;
    juce::Slider expressionSlider;
    juce::Label expressionLabel;

    // --- Settings ---
    juce::Label settingsGroupLabel;
    juce::ComboBox bendRangeCombo;
    juce::Label bendRangeLabel;
    juce::ComboBox tuningCombo;
    juce::Label tuningLabel;
    juce::ToggleButton legatoToggle;
    juce::ComboBox glideCurveCombo;
    juce::Label glideCurveLabel;

    // --- Level meter ---
    juce::Label levelLabel;
    float currentPeak = 0.0f;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(VeenaPluginEditor)
};
