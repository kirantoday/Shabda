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

    // Drop shadow under the visualization panel for depth
    g.setColour(juce::Colour(0x25000000));
    g.fillRoundedRectangle(bounds.translated(0, 3).reduced(-1, -1),
                            static_cast<float>(theme::dim::panelRadius) + 1);

    g.setColour(theme::color::panelBorder);
    g.drawRoundedRectangle(bounds.reduced(0.5f), static_cast<float>(theme::dim::panelRadius), 1.0f);

    // Layout: fixed pixel splits for reliable sizing.
    // Right side: thalam 60px, kudam 130px, bridge 14px. Rest = neck.
    float totalW = bounds.getWidth();
    float thalamW = 60.0f;
    float kudamW = 130.0f;
    float bridgeW = 14.0f;
    float neckW = totalW - thalamW - kudamW - bridgeW;

    auto neckArea = bounds.removeFromLeft(neckW).reduced(15, 20);
    auto bridgeArea = bounds.removeFromLeft(bridgeW).reduced(0, 25);
    auto kudamArea = bounds.removeFromLeft(kudamW).reduced(4, 10);
    auto thalamArea = bounds.reduced(2, 25);

    drawNeck(g, neckArea);
    drawFrets(g, neckArea);
    drawSympatheticStrings(g, neckArea);
    drawBridge(g, bridgeArea, neckArea);
    drawMainStrings(g, neckArea, bridgeArea);
    drawFingerPosition(g, neckArea);
    drawKudam(g, kudamArea, bridgeArea);
    drawThalamStrings(g, thalamArea);
}

void VeenaVisualization::drawNeck(juce::Graphics& g, juce::Rectangle<float> area)
{
    auto neckRect = area.reduced(0, 4);

    // Polished walnut wood-grain gradient: dark walnut to medium brown.
    // Horizontal gradient simulates the grain direction along the neck.
    juce::ColourGradient woodGrain(
        juce::Colour(0xff3E2723), neckRect.getX(), neckRect.getCentreY(),       // dark walnut
        juce::Colour(0xff5D4037), neckRect.getRight(), neckRect.getCentreY(),   // medium brown
        false);
    // Add midpoint color stops for richer grain variation
    woodGrain.addColour(0.25, juce::Colour(0xff4A3228));
    woodGrain.addColour(0.5,  juce::Colour(0xff513830));
    woodGrain.addColour(0.75, juce::Colour(0xff573D34));
    g.setGradientFill(woodGrain);
    g.fillRoundedRectangle(neckRect, 5.0f);

    // 3D bevel: top edge lighter (highlight), bottom edge darker (shadow)
    juce::ColourGradient topBevel(
        juce::Colour(0x25FFFFFF), neckRect.getX(), neckRect.getY(),
        juce::Colour(0x00000000), neckRect.getX(), neckRect.getY() + 8, false);
    g.setGradientFill(topBevel);
    g.fillRoundedRectangle(neckRect.getX(), neckRect.getY(), neckRect.getWidth(), 8.0f, 5.0f);

    juce::ColourGradient bottomShadow(
        juce::Colour(0x00000000), neckRect.getX(), neckRect.getBottom() - 8,
        juce::Colour(0x30000000), neckRect.getX(), neckRect.getBottom(), false);
    g.setGradientFill(bottomShadow);
    g.fillRoundedRectangle(neckRect.getX(), neckRect.getBottom() - 8, neckRect.getWidth(), 8.0f, 5.0f);

    // Neck outline with subtle bevel
    g.setColour(juce::Colour(0xff4A3520).withAlpha(0.6f));
    g.drawRoundedRectangle(neckRect, 5.0f, 1.0f);

    // Inner highlight line (top edge gleam)
    g.setColour(juce::Colour(0x15FFFFFF));
    g.drawLine(neckRect.getX() + 6, neckRect.getY() + 1,
               neckRect.getRight() - 6, neckRect.getY() + 1, 0.5f);
}

void VeenaVisualization::drawFrets(juce::Graphics& g, juce::Rectangle<float> neckArea)
{
    float fretSpacing = neckArea.getWidth() / static_cast<float>(NUM_FRETS + 1);

    for (int i = 0; i <= NUM_FRETS; ++i)
    {
        float fretX = neckArea.getX() + static_cast<float>(i) * fretSpacing;
        int fretNote = BASE_MIDI_NOTE + i;
        bool isActive = (fretNote == activeNote);

        if (isActive)
        {
            // Active fret: brighter gold, thicker
            g.setColour(theme::color::fretActive.withAlpha(0.6f));
            g.drawLine(fretX, neckArea.getY() + 6, fretX, neckArea.getBottom() - 6, 1.5f);

            // Soft glow behind active fret
            g.setColour(theme::color::fretActive.withAlpha(0.1f));
            g.fillRect(fretX - fretSpacing * 0.35f, neckArea.getY() + 4,
                       fretSpacing * 0.7f, neckArea.getHeight() - 8);
        }
        else
        {
            // Subtle gold fret lines at 15% opacity — thin, not a grid
            g.setColour(theme::color::gold.withAlpha(0.15f));
            g.drawLine(fretX, neckArea.getY() + 8, fretX, neckArea.getBottom() - 8, 0.5f);
        }
    }
}

void VeenaVisualization::drawBridge(juce::Graphics& g, juce::Rectangle<float> bridgeArea,
                                     juce::Rectangle<float> neckArea)
{
    // Brass bridge bar where strings meet the kudam body.
    float bridgeX = bridgeArea.getCentreX();
    float bridgeTop = neckArea.getY() + 10;
    float bridgeBottom = neckArea.getBottom() - 10;
    float bridgeW = 5.0f;

    // Brass gradient
    juce::ColourGradient brassGrad(
        theme::color::goldBright, bridgeX - bridgeW * 0.5f, bridgeTop,
        theme::color::gold.darker(0.3f), bridgeX + bridgeW * 0.5f, bridgeTop, false);
    g.setGradientFill(brassGrad);
    g.fillRoundedRectangle(bridgeX - bridgeW * 0.5f, bridgeTop, bridgeW, bridgeBottom - bridgeTop, 2.0f);

    // Highlight edge
    g.setColour(juce::Colour(0x30FFFFFF));
    g.drawLine(bridgeX - bridgeW * 0.5f, bridgeTop + 2,
               bridgeX - bridgeW * 0.5f, bridgeBottom - 2, 0.5f);
}

void VeenaVisualization::drawMainStrings(juce::Graphics& g, juce::Rectangle<float> neckArea,
                                          juce::Rectangle<float> bridgeArea)
{
    // 4 main strings with different thicknesses (Sa thickest, upper Pa thinnest).
    // Real veena strings get thinner as pitch goes up.
    const char* stringNames[] = { "Sa", "Pa", "sa", "Pa" };
    const float stringThickness[] = { 3.0f, 2.5f, 2.0f, 1.5f };       // idle thickness
    const float stringActiveThickness[] = { 3.8f, 3.0f, 2.5f, 2.0f }; // active thickness
    float stringSpacing = neckArea.getHeight() / 5.0f;

    // Strings extend from neck to bridge
    float endX = bridgeArea.getCentreX();

    for (int s = 0; s < 4; ++s)
    {
        float stringY = neckArea.getY() + stringSpacing * static_cast<float>(s + 1);

        int voiceIdx = (s < 2) ? 0 : 1;
        float amp = smoothedAmplitudes[voiceIdx];
        bool isActive = (amp > 0.01f) && (s == 0 || s == 2);

        // Build string path with catenary and vibration
        juce::Path stringPath;
        float startX = neckArea.getX();
        int segments = 80;

        stringPath.startNewSubPath(startX, stringY);
        for (int i = 1; i <= segments; ++i)
        {
            float t = static_cast<float>(i) / static_cast<float>(segments);
            float x = startX + t * (endX - startX);

            // Catenary sag: thicker strings sag more
            float sagAmount = 1.5f + stringThickness[s] * 0.3f;
            float catenary = sagAmount * std::sin(t * juce::MathConstants<float>::pi);

            // Vibration
            float envelope = std::sin(t * juce::MathConstants<float>::pi);
            float vibration = amp * 5.0f * envelope *
                std::sin(t * 12.0f + animPhase * (2.0f + static_cast<float>(s) * 0.3f));

            stringPath.lineTo(x, stringY + catenary + vibration);
        }

        float thickness = isActive ? stringActiveThickness[s] : stringThickness[s];

        // Shadow under string (1px dark shadow, offset down)
        {
            juce::Path shadowPath;
            shadowPath.startNewSubPath(startX, stringY + 1.5f);
            for (int i = 1; i <= segments; ++i)
            {
                float t = static_cast<float>(i) / static_cast<float>(segments);
                float x = startX + t * (endX - startX);
                float sagAmount = 1.5f + stringThickness[s] * 0.3f;
                float catenary = sagAmount * std::sin(t * juce::MathConstants<float>::pi);
                float envelope = std::sin(t * juce::MathConstants<float>::pi);
                float vibration = amp * 5.0f * envelope *
                    std::sin(t * 12.0f + animPhase * (2.0f + static_cast<float>(s) * 0.3f));
                shadowPath.lineTo(x, stringY + 1.5f + catenary + vibration);
            }
            g.setColour(juce::Colour(0x20000000));
            g.strokePath(shadowPath, juce::PathStrokeType(thickness + 1.0f));
        }

        // String color
        juce::Colour stringCol = isActive
            ? theme::color::stringActive.withAlpha(0.85f + amp * 0.15f)
            : theme::color::stringIdle.withAlpha(0.55f);

        g.setColour(stringCol);
        g.strokePath(stringPath, juce::PathStrokeType(thickness));

        // Gold glow for active strings
        if (isActive && amp > 0.02f)
        {
            g.setColour(theme::color::stringGlow.withAlpha(amp * 0.45f));
            g.strokePath(stringPath, juce::PathStrokeType(thickness + 5.0f));

            g.setColour(theme::color::goldBright.withAlpha(amp * 0.12f));
            g.strokePath(stringPath, juce::PathStrokeType(thickness + 10.0f));
        }

        // String label
        g.setColour(theme::color::textSecondary.withAlpha(0.5f));
        g.setFont(juce::FontOptions(9.0f));
        g.drawText(stringNames[s], static_cast<int>(neckArea.getX()) - 22,
                   static_cast<int>(stringY) - 5, 20, 10,
                   juce::Justification::centredRight, false);
    }
}

void VeenaVisualization::drawThalamStrings(juce::Graphics& g, juce::Rectangle<float> area)
{
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
        g.drawLine(area.getX(), y, area.getRight(), y - 6, flash > 0.05f ? 2.0f : 1.2f);

        if (flash > 0.1f)
        {
            g.setColour(theme::color::goldBright.withAlpha(flash * 0.25f));
            g.drawLine(area.getX(), y, area.getRight(), y - 6, 5.0f);
        }

        // Label
        g.setColour(theme::color::gold.withAlpha(0.85f));
        g.setFont(juce::FontOptions(11.0f, juce::Font::bold));
        g.drawText(labels[i], static_cast<int>(area.getRight() + 3),
                   static_cast<int>(y) - 8, 16, 14,
                   juce::Justification::centredLeft, false);
    }
}

void VeenaVisualization::drawKudam(juce::Graphics& g, juce::Rectangle<float> area,
                                    juce::Rectangle<float> bridgeArea)
{
    // Gourd body positioned so it connects to the bridge.
    // The left edge of the kudam aligns with the bridge area.
    float cx = area.getCentreX() + 4.0f;
    float cy = area.getCentreY();
    float rx = area.getWidth() * 0.48f;
    float ry = area.getHeight() * 0.42f;

    // Connection piece: neck-to-kudam transition
    // Small trapezoidal wood piece between bridge and kudam
    {
        float connLeft = bridgeArea.getRight();
        float connRight = cx - rx + 4;
        float connTop = cy - ry * 0.35f;
        float connBottom = cy + ry * 0.35f;

        juce::Path connector;
        connector.startNewSubPath(connLeft, connTop + 2);
        connector.lineTo(connRight, connTop - 4);
        connector.lineTo(connRight, connBottom + 4);
        connector.lineTo(connLeft, connBottom - 2);
        connector.closeSubPath();

        g.setColour(juce::Colour(0xff3D2817));
        g.fillPath(connector);
        g.setColour(juce::Colour(0xff4A3520).withAlpha(0.5f));
        g.strokePath(connector, juce::PathStrokeType(0.5f));
    }

    // Body glow when sound is active
    if (smoothedPeak > 0.01f)
    {
        float glowAlpha = smoothedPeak * 0.2f;
        g.setColour(theme::color::woodLight.withAlpha(glowAlpha));
        g.fillEllipse(cx - rx - 5, cy - ry - 5, (rx + 5) * 2, (ry + 5) * 2);
    }

    // Main body with rich wood gradient
    juce::ColourGradient bodyGrad(
        juce::Colour(0xffA0662B), cx - rx * 0.4f, cy - ry * 0.4f,     // warm highlight
        juce::Colour(0xff5A2D0C), cx + rx * 0.4f, cy + ry * 0.4f,     // deep shadow
        true);
    g.setGradientFill(bodyGrad);
    g.fillEllipse(cx - rx, cy - ry, rx * 2, ry * 2);

    // Wood grain lines on kudam (subtle horizontal arcs)
    g.setColour(juce::Colour(0x08000000));
    for (int i = -3; i <= 3; ++i)
    {
        float grainY = cy + static_cast<float>(i) * ry * 0.22f;
        g.drawLine(cx - rx * 0.7f, grainY, cx + rx * 0.7f, grainY + 1, 0.5f);
    }

    // Outline with highlight
    g.setColour(theme::color::woodLight.withAlpha(0.35f));
    g.drawEllipse(cx - rx, cy - ry, rx * 2, ry * 2, 1.2f);

    // Sound hole
    float holeR = rx * 0.18f;
    g.setColour(theme::color::background.withAlpha(0.8f));
    g.fillEllipse(cx - holeR, cy - holeR * 0.6f, holeR * 2, holeR * 1.2f);
    // Sound hole rim
    g.setColour(theme::color::gold.withAlpha(0.2f));
    g.drawEllipse(cx - holeR, cy - holeR * 0.6f, holeR * 2, holeR * 1.2f, 0.5f);

    // Label
    g.setColour(theme::color::textSecondary.withAlpha(0.35f));
    g.setFont(juce::FontOptions(9.0f));
    auto labelArea = juce::Rectangle<float>(cx - 20, area.getBottom() - 14, 40, 14);
    g.drawText("Kudam", labelArea, juce::Justification::centred, false);
}

void VeenaVisualization::drawSympatheticStrings(juce::Graphics& g, juce::Rectangle<float> neckArea)
{
    float baseY = neckArea.getBottom() - 10;

    for (int i = 0; i < 3; ++i)
    {
        float y = baseY + static_cast<float>(i) * 2.5f;
        float shimmer = smoothedSympathetic;

        juce::Colour col = (shimmer > 0.01f)
            ? theme::color::gold.withAlpha(0.12f + shimmer * 0.3f)
            : theme::color::stringIdle.withAlpha(0.12f);

        g.setColour(col);
        g.drawLine(neckArea.getX() + 10, y, neckArea.getRight() - 10, y, 0.5f);
    }
}

void VeenaVisualization::drawFingerPosition(juce::Graphics& g, juce::Rectangle<float> neckArea)
{
    if (activeNote < 0)
        return;

    float fretSpacing = neckArea.getWidth() / static_cast<float>(NUM_FRETS + 1);
    float notePosition = static_cast<float>(activeNote - BASE_MIDI_NOTE) + pitchOffset;
    notePosition = juce::jlimit(0.0f, static_cast<float>(NUM_FRETS), notePosition);

    float fingerX = neckArea.getX() + notePosition * fretSpacing;
    float fingerY = neckArea.getY() + neckArea.getHeight() * 0.25f;

    float amp = smoothedAmplitudes[0];
    float dotSize = 8.0f + amp * 4.0f;

    // Glow
    g.setColour(theme::color::goldBright.withAlpha(0.2f + amp * 0.3f));
    g.fillEllipse(fingerX - dotSize, fingerY - dotSize, dotSize * 2, dotSize * 2);

    // Dot
    g.setColour(theme::color::goldBright);
    g.fillEllipse(fingerX - 4, fingerY - 4, 8.0f, 8.0f);
}
