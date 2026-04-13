#include "VeenaVisualization.h"
#include <cmath>

VeenaVisualization::VeenaVisualization()
{
    startTimerHz(30);
}

VeenaVisualization::~VeenaVisualization()
{
    stopTimer();
}

void VeenaVisualization::timerCallback()
{
    // Smooth animation values with decay.
    for (int i = 0; i < 2; ++i)
        smoothedAmplitudes[i] = smoothedAmplitudes[i] * 0.88f + stringAmplitudes[i] * 0.12f;

    for (int i = 0; i < 3; ++i)
        thalamFlash[i] *= 0.85f;

    smoothedSympathetic = smoothedSympathetic * 0.9f + sympatheticLevel * 0.1f;
    smoothedPeak = smoothedPeak * 0.85f + peakLevel * 0.15f;
    animPhase += 0.15f;
    if (animPhase > 100.0f) animPhase -= 100.0f;

    repaint();
}

void VeenaVisualization::setStringAmplitude(int voiceIndex, float amplitude)
{
    if (voiceIndex >= 0 && voiceIndex < 2)
        stringAmplitudes[voiceIndex] = amplitude;
}

void VeenaVisualization::setActiveNote(int midiNote)   { activeNote = midiNote; }
void VeenaVisualization::setPitchOffset(float s)       { pitchOffset = s; }
void VeenaVisualization::setSympatheticLevel(float l)  { sympatheticLevel = l; }
void VeenaVisualization::setPeakLevel(float l)         { peakLevel = l; }

void VeenaVisualization::setThalamFlash(int stringIndex, float intensity)
{
    if (stringIndex >= 0 && stringIndex < 3)
        thalamFlash[stringIndex] = intensity;
}

void VeenaVisualization::paint(juce::Graphics& g)
{
    auto bounds = getLocalBounds().toFloat();

    // Dark background with subtle gradient
    g.setGradientFill(juce::ColourGradient(
        theme::color::panelSurface, bounds.getCentreX(), bounds.getY(),
        theme::color::background, bounds.getCentreX(), bounds.getBottom(), false));
    g.fillRoundedRectangle(bounds, static_cast<float>(theme::dim::panelRadius));

    // Subtle border
    g.setColour(theme::color::panelBorder);
    g.drawRoundedRectangle(bounds.reduced(0.5f), static_cast<float>(theme::dim::panelRadius), 1.0f);

    // Layout: neck on left 70%, kudam on right 25%, thalam in between
    float kudamWidth = bounds.getWidth() * 0.20f;
    float thalamWidth = bounds.getWidth() * 0.10f;
    auto neckArea = bounds.removeFromLeft(bounds.getWidth() - kudamWidth - thalamWidth).reduced(15, 20);
    auto thalamArea = bounds.removeFromLeft(thalamWidth).reduced(4, 30);
    auto kudamArea = bounds.reduced(8, 15);

    drawNeck(g, neckArea);
    drawFrets(g, neckArea);
    drawSympatheticStrings(g, neckArea);
    drawMainStrings(g, neckArea);
    drawFingerPosition(g, neckArea);
    drawThalamStrings(g, thalamArea);
    drawKudam(g, kudamArea);
}

void VeenaVisualization::drawNeck(juce::Graphics& g, juce::Rectangle<float> area)
{
    // Neck body — dark wood-toned rectangle
    g.setColour(theme::color::wood.withAlpha(0.15f));
    g.fillRoundedRectangle(area.reduced(0, 8), 4.0f);
}

void VeenaVisualization::drawFrets(juce::Graphics& g, juce::Rectangle<float> neckArea)
{
    // 24 frets evenly spaced along the neck.
    // Active fret glows gold.
    float fretSpacing = neckArea.getWidth() / static_cast<float>(NUM_FRETS + 1);

    for (int i = 0; i <= NUM_FRETS; ++i)
    {
        float fretX = neckArea.getX() + static_cast<float>(i) * fretSpacing;
        int fretNote = BASE_MIDI_NOTE + i;
        bool isActive = (fretNote == activeNote);

        g.setColour(isActive ? theme::color::fretActive.withAlpha(0.8f) : theme::color::fret.withAlpha(0.3f));
        g.drawLine(fretX, neckArea.getY() + 5, fretX, neckArea.getBottom() - 5, isActive ? 2.0f : 1.0f);

        // Active fret glow
        if (isActive)
        {
            g.setColour(theme::color::fretActive.withAlpha(0.15f));
            g.fillRect(fretX - fretSpacing * 0.4f, neckArea.getY(),
                       fretSpacing * 0.8f, neckArea.getHeight());
        }
    }
}

void VeenaVisualization::drawMainStrings(juce::Graphics& g, juce::Rectangle<float> neckArea)
{
    // 4 main strings rendered as horizontal lines with vibration animation.
    // When active, they oscillate sinusoidally.
    const char* stringNames[] = { "Sa", "Pa", "sa", "Pa" };
    float stringSpacing = neckArea.getHeight() / 5.0f;

    for (int s = 0; s < 4; ++s)
    {
        float stringY = neckArea.getY() + stringSpacing * static_cast<float>(s + 1);

        // Use voice 0 amplitude for strings 0-1, voice 1 for strings 2-3
        int voiceIdx = (s < 2) ? 0 : 1;
        float amp = smoothedAmplitudes[voiceIdx];
        bool isActive = (amp > 0.01f) && (s == 0 || s == 2);  // only first string of each voice

        // Draw the string with vibration displacement
        juce::Path stringPath;
        float startX = neckArea.getX();
        float endX = neckArea.getRight();
        int segments = 80;

        stringPath.startNewSubPath(startX, stringY);
        for (int i = 1; i <= segments; ++i)
        {
            float t = static_cast<float>(i) / static_cast<float>(segments);
            float x = startX + t * (endX - startX);

            // Sine displacement: amplitude * sin(position * freq + time)
            // Envelope: larger in the middle, zero at endpoints
            float envelope = std::sin(t * juce::MathConstants<float>::pi);
            float vibration = amp * 6.0f * envelope *
                std::sin(t * 12.0f + animPhase * (2.0f + static_cast<float>(s) * 0.3f));

            stringPath.lineTo(x, stringY + vibration);
        }

        // String color: brighter and thicker when active
        juce::Colour stringCol = isActive
            ? theme::color::stringActive.withAlpha(0.8f + amp * 0.2f)
            : theme::color::stringIdle.withAlpha(0.6f);

        g.setColour(stringCol);
        g.strokePath(stringPath, juce::PathStrokeType(isActive ? 2.5f : 1.5f));

        // Glow for active strings — wider, more prominent
        if (isActive && amp > 0.02f)
        {
            g.setColour(theme::color::stringGlow.withAlpha(amp * 0.5f));
            g.strokePath(stringPath, juce::PathStrokeType(7.0f));

            // Inner bright glow
            g.setColour(theme::color::goldBright.withAlpha(amp * 0.15f));
            g.strokePath(stringPath, juce::PathStrokeType(12.0f));
        }

        // String label
        g.setColour(theme::color::textSecondary.withAlpha(0.5f));
        g.setFont(juce::FontOptions(9.0f));
        g.drawText(stringNames[s], static_cast<int>(neckArea.getX()) - 22, static_cast<int>(stringY) - 5, 20, 10,
                   juce::Justification::centredRight, false);
    }
}

void VeenaVisualization::drawThalamStrings(juce::Graphics& g, juce::Rectangle<float> area)
{
    // 3 thalam strings — short, angled, flash on pluck.
    const char* labels[] = { "Z", "X", "C" };
    float spacing = area.getHeight() / 4.0f;

    for (int i = 0; i < 3; ++i)
    {
        float y = area.getY() + spacing * static_cast<float>(i + 1);
        float flash = thalamFlash[i];

        juce::Colour col = (flash > 0.05f)
            ? theme::color::goldBright.withAlpha(0.5f + flash * 0.5f)
            : theme::color::stringIdle.withAlpha(0.4f);

        g.setColour(col);
        g.drawLine(area.getX(), y, area.getRight(), y - 8, flash > 0.05f ? 2.0f : 1.0f);

        // Glow
        if (flash > 0.1f)
        {
            g.setColour(theme::color::goldBright.withAlpha(flash * 0.25f));
            g.drawLine(area.getX(), y, area.getRight(), y - 8, 5.0f);
        }

        // Label — bright gold, larger text for readability
        g.setColour(theme::color::gold.withAlpha(0.85f));
        g.setFont(juce::FontOptions(11.0f, juce::Font::bold));
        g.drawText(labels[i], static_cast<int>(area.getRight() + 3), static_cast<int>(y) - 8, 16, 14,
                   juce::Justification::centredLeft, false);
    }
}

void VeenaVisualization::drawKudam(juce::Graphics& g, juce::Rectangle<float> area)
{
    // Stylized gourd resonator body — elliptical shape.
    float cx = area.getCentreX();
    float cy = area.getCentreY();
    float rx = area.getWidth() * 0.45f;
    float ry = area.getHeight() * 0.4f;

    // Body glow when sound is active
    if (smoothedPeak > 0.01f)
    {
        float glowAlpha = smoothedPeak * 0.2f;
        g.setColour(theme::color::woodLight.withAlpha(glowAlpha));
        g.fillEllipse(cx - rx - 4, cy - ry - 4, (rx + 4) * 2, (ry + 4) * 2);
    }

    // Main body shape with wood gradient
    juce::ColourGradient bodyGrad(theme::color::wood, cx - rx * 0.3f, cy - ry * 0.3f,
                                   theme::color::wood.darker(0.5f), cx + rx * 0.5f, cy + ry * 0.5f, true);
    g.setGradientFill(bodyGrad);
    g.fillEllipse(cx - rx, cy - ry, rx * 2, ry * 2);

    // Outline
    g.setColour(theme::color::woodLight.withAlpha(0.4f));
    g.drawEllipse(cx - rx, cy - ry, rx * 2, ry * 2, 1.5f);

    // Sound hole (small dark circle)
    float holeR = rx * 0.2f;
    g.setColour(theme::color::background.withAlpha(0.7f));
    g.fillEllipse(cx - holeR, cy - holeR * 0.5f, holeR * 2, holeR);

    // Label
    g.setColour(theme::color::textSecondary.withAlpha(0.4f));
    g.setFont(juce::FontOptions(9.0f));
    g.drawText("Kudam", area.removeFromBottom(14), juce::Justification::centred, false);
}

void VeenaVisualization::drawSympatheticStrings(juce::Graphics& g, juce::Rectangle<float> neckArea)
{
    // Thin lines below main strings, shimmer when resonating.
    float baseY = neckArea.getBottom() - 12;

    for (int i = 0; i < 3; ++i)
    {
        float y = baseY + static_cast<float>(i) * 3.0f;
        float shimmer = smoothedSympathetic;

        juce::Colour col = (shimmer > 0.01f)
            ? theme::color::gold.withAlpha(0.15f + shimmer * 0.35f)
            : theme::color::stringIdle.withAlpha(0.15f);

        g.setColour(col);
        g.drawLine(neckArea.getX() + 10, y, neckArea.getRight() - 10, y, 0.5f);
    }
}

void VeenaVisualization::drawFingerPosition(juce::Graphics& g, juce::Rectangle<float> neckArea)
{
    if (activeNote < 0)
        return;

    // Finger dot at the fret position of the active note.
    float fretSpacing = neckArea.getWidth() / static_cast<float>(NUM_FRETS + 1);
    float notePosition = static_cast<float>(activeNote - BASE_MIDI_NOTE) + pitchOffset;
    notePosition = juce::jlimit(0.0f, static_cast<float>(NUM_FRETS), notePosition);

    float fingerX = neckArea.getX() + notePosition * fretSpacing;
    float fingerY = neckArea.getY() + neckArea.getHeight() * 0.25f;  // on first string area

    float amp = smoothedAmplitudes[0];
    float dotSize = 8.0f + amp * 4.0f;

    // Glow
    g.setColour(theme::color::goldBright.withAlpha(0.2f + amp * 0.3f));
    g.fillEllipse(fingerX - dotSize, fingerY - dotSize, dotSize * 2, dotSize * 2);

    // Dot
    g.setColour(theme::color::goldBright);
    g.fillEllipse(fingerX - 4, fingerY - 4, 8.0f, 8.0f);
}
