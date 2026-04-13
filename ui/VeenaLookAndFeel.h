#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include "Theme.h"

// Custom LookAndFeel for the Shabda Veena plugin.
// Replaces all default JUCE control rendering with premium Indian-aesthetic controls:
//   - Circular knobs with gold value arcs
//   - Vertical strips for pitch bend and expression
//   - Pill toggles
//   - Styled combo boxes with gold accents

class VeenaLookAndFeel : public juce::LookAndFeel_V4
{
public:
    VeenaLookAndFeel();

    // --- Rotary Slider (Knob) ---
    void drawRotarySlider(juce::Graphics& g, int x, int y, int width, int height,
                          float sliderPos, float rotaryStartAngle, float rotaryEndAngle,
                          juce::Slider& slider) override;

    // --- Linear Slider (Vertical Strip) ---
    void drawLinearSlider(juce::Graphics& g, int x, int y, int width, int height,
                          float sliderPos, float minSliderPos, float maxSliderPos,
                          juce::Slider::SliderStyle style, juce::Slider& slider) override;

    // --- Toggle Button (Pill) ---
    void drawToggleButton(juce::Graphics& g, juce::ToggleButton& button,
                          bool shouldDrawButtonAsHighlighted,
                          bool shouldDrawButtonAsDown) override;

    // --- Combo Box ---
    void drawComboBox(juce::Graphics& g, int width, int height,
                      bool isButtonDown, int buttonX, int buttonY,
                      int buttonW, int buttonH, juce::ComboBox& box) override;

    void drawPopupMenuItem(juce::Graphics& g, const juce::Rectangle<int>& area,
                           bool isSeparator, bool isActive, bool isHighlighted,
                           bool isTicked, bool hasSubMenu,
                           const juce::String& text, const juce::String& shortcutKeyText,
                           const juce::Drawable* icon, const juce::Colour* textColour) override;

    // --- Label ---
    void drawLabel(juce::Graphics& g, juce::Label& label) override;

    // --- Popup Menu background ---
    void drawPopupMenuBackground(juce::Graphics& g, int width, int height) override;

    juce::Font getComboBoxFont(juce::ComboBox&) override;
    juce::Font getPopupMenuFont() override;
};
