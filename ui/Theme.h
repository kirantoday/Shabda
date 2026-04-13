#pragma once

#include <juce_gui_basics/juce_gui_basics.h>

// Central theme definition for the Shabda Veena plugin UI.
// All colors, dimensions, and font specs in one place.
// Indian-inspired premium aesthetic — gold, brass, warm wood tones.

namespace theme {

// --- Color Palette ---

namespace color {
    // Backgrounds
    inline const juce::Colour background     { 0xff1A1A2E };  // Deep charcoal
    inline const juce::Colour panelSurface   { 0xff16213E };  // Dark navy
    inline const juce::Colour panelBorder    { 0xff2A2A4E };  // Subtle border

    // Accents
    inline const juce::Colour gold           { 0xffC4A265 };  // Primary — brass frets
    inline const juce::Colour goldBright     { 0xffD4AF37 };  // Active/highlight
    inline const juce::Colour goldDim        { 0xff8B7D4A };  // Inactive gold
    inline const juce::Colour wood           { 0xff8B4513 };  // Jackfruit wood kudam
    inline const juce::Colour woodLight      { 0xffA0662B };  // Wood highlight

    // Text
    inline const juce::Colour textPrimary    { 0xffF0E6D3 };  // Warm cream
    inline const juce::Colour textSecondary  { 0xff9A8C78 };  // Muted gold
    inline const juce::Colour textValue      { 0xffC4A265 };  // Value readouts

    // Strings
    inline const juce::Colour stringIdle     { 0xff8B7D5A };  // Dim gold
    inline const juce::Colour stringActive   { 0xffE8D5A3 };  // Bright gold
    inline const juce::Colour stringGlow     { 0x40E8D5A3 };  // Transparent glow

    // Frets
    inline const juce::Colour fret           { 0xff555566 };  // Dim fret
    inline const juce::Colour fretActive     { 0xffC4A265 };  // Active fret

    // Status
    inline const juce::Colour alert          { 0xffC0392B };  // Clip/alert
    inline const juce::Colour safe           { 0xff27AE60 };  // Safe level
    inline const juce::Colour meterMid       { 0xffD4AF37 };  // Mid level

    // Controls
    inline const juce::Colour knobBackground { 0xff0F0F1E };  // Dark knob fill
    inline const juce::Colour knobRing       { 0xff333355 };  // Inactive ring
    inline const juce::Colour knobArc        { 0xffC4A265 };  // Value arc
    inline const juce::Colour knobIndicator  { 0xffF0E6D3 };  // Pointer/dot
    inline const juce::Colour stripFill      { 0xff0F0F1E };  // Vertical strip bg
    inline const juce::Colour toggleOn       { 0xffC4A265 };  // Pill toggle on
    inline const juce::Colour toggleOff      { 0xff333355 };  // Pill toggle off
}

// --- Dimensions ---

namespace dim {
    constexpr int windowWidth      = 940;
    constexpr int windowHeight     = 720;
    constexpr int minWidth         = 700;
    constexpr int minHeight        = 500;

    constexpr int headerHeight     = 40;
    constexpr int vizHeight        = 220;   // Instrument visualization
    constexpr int controlsHeight   = 240;   // Knobs and controls
    constexpr int keyboardHeight   = 90;
    constexpr int padding          = 10;
    constexpr int panelRadius      = 6;     // Rounded corners

    constexpr int knobSize         = 56;    // Diameter
    constexpr int knobLabelGap     = 2;
    constexpr int stripWidth       = 30;
    constexpr int stripHeight      = 140;

    constexpr int comboHeight      = 22;
    constexpr int toggleWidth      = 44;
    constexpr int toggleHeight     = 22;
}

// --- Font Sizes ---

namespace font {
    constexpr float title       = 20.0f;
    constexpr float subtitle    = 14.0f;
    constexpr float label       = 11.0f;
    constexpr float value       = 11.0f;
    constexpr float groupHeader = 12.0f;
    constexpr float presetName  = 13.0f;
}

} // namespace theme
