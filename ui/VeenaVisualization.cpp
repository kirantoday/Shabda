#include "VeenaVisualization.h"
#include "VeenaSVG.h"
#include <cmath>

VeenaVisualization::VeenaVisualization()
{
    loadSVG();
    startTimerHz(30);
}

VeenaVisualization::~VeenaVisualization()
{
    stopTimer();
}

void VeenaVisualization::loadSVG()
{
    auto xml = juce::XmlDocument::parse(veenaSVG::getSVG());
    if (xml != nullptr)
        svgDrawable = juce::Drawable::createFromSVG(*xml);
}

void VeenaVisualization::resized()
{
    if (svgDrawable == nullptr) return;

    // Fit the tall SVG into the component, maintaining aspect ratio.
    auto bounds = getLocalBounds().toFloat().reduced(4, 4);
    float svgAspect = veenaSVG::viewBoxWidth / veenaSVG::viewBoxHeight;
    float compAspect = bounds.getWidth() / bounds.getHeight();

    if (compAspect > svgAspect)
    {
        // Component wider — fit by height (veena is tall)
        svgScale = bounds.getHeight() / veenaSVG::viewBoxHeight;
        svgOffsetX = bounds.getX() + (bounds.getWidth() - veenaSVG::viewBoxWidth * svgScale) * 0.5f;
        svgOffsetY = bounds.getY();
    }
    else
    {
        svgScale = bounds.getWidth() / veenaSVG::viewBoxWidth;
        svgOffsetX = bounds.getX();
        svgOffsetY = bounds.getY() + (bounds.getHeight() - veenaSVG::viewBoxHeight * svgScale) * 0.5f;
    }
}

void VeenaVisualization::timerCallback()
{
    for (int i = 0; i < 2; ++i)
        smoothedAmplitudes[i] = smoothedAmplitudes[i] * 0.85f + stringAmplitudes[i] * 0.15f;
    for (int i = 0; i < 3; ++i)
        thalamFlash[i] *= 0.85f;
    smoothedSympathetic = smoothedSympathetic * 0.9f + sympatheticLevel * 0.1f;
    smoothedPeak = smoothedPeak * 0.85f + peakLevel * 0.15f;
    animPhase += 0.18f;
    if (animPhase > 100.0f) animPhase -= 100.0f;
    repaint();
}

void VeenaVisualization::setStringAmplitude(int vi, float a) { if (vi >= 0 && vi < 2) stringAmplitudes[vi] = a; }
void VeenaVisualization::setActiveNote(int n)       { activeNote = n; }
void VeenaVisualization::setPitchOffset(float s)    { pitchOffset = s; }
void VeenaVisualization::setSympatheticLevel(float l) { sympatheticLevel = l; }
void VeenaVisualization::setPeakLevel(float l)      { peakLevel = l; }
void VeenaVisualization::setThalamFlash(int i, float v) { if (i >= 0 && i < 3) thalamFlash[i] = v; }

juce::Point<float> VeenaVisualization::svgToComponent(float svgX, float svgY) const
{
    return { svgOffsetX + svgX * svgScale, svgOffsetY + svgY * svgScale };
}

juce::Point<float> VeenaVisualization::stringPosition(int stringIndex, float fretFraction) const
{
    // Strings are vertical — X is fixed per string, Y varies with fret position.
    int s = juce::jlimit(0, 3, stringIndex);
    float f = juce::jlimit(0.0f, 1.0f, fretFraction);

    float svgX = veenaSVG::stringX[s];
    float svgY = veenaSVG::stringTopY + f * (veenaSVG::stringBottomY - veenaSVG::stringTopY);

    return svgToComponent(svgX, svgY);
}

void VeenaVisualization::paint(juce::Graphics& g)
{
    auto bounds = getLocalBounds().toFloat();

    // Background
    g.setGradientFill(juce::ColourGradient(
        theme::color::panelSurface, bounds.getCentreX(), bounds.getY(),
        theme::color::background, bounds.getCentreX(), bounds.getBottom(), false));
    g.fillRoundedRectangle(bounds, static_cast<float>(theme::dim::panelRadius));
    g.setColour(theme::color::panelBorder);
    g.drawRoundedRectangle(bounds.reduced(0.5f), static_cast<float>(theme::dim::panelRadius), 1.0f);

    // Render SVG base
    if (svgDrawable != nullptr)
    {
        auto svgBounds = juce::Rectangle<float>(
            svgOffsetX, svgOffsetY,
            veenaSVG::viewBoxWidth * svgScale,
            veenaSVG::viewBoxHeight * svgScale);
        svgDrawable->drawWithin(g, svgBounds, juce::RectanglePlacement::stretchToFit, 1.0f);
    }

    // Interactive overlays
    drawKudamGlow(g);
    drawStringGlow(g);
    drawPluckFlash(g);
    drawFingerPosition(g);
}

void VeenaVisualization::drawStringGlow(juce::Graphics& g)
{
    // Strings are VERTICAL. Vibration = horizontal displacement (side to side).
    // This is the most important visual — must be OBVIOUS.
    const float glowWidth[] = { 10.0f, 8.0f, 6.0f, 5.0f };

    for (int s = 0; s < 4; ++s)
    {
        int voiceIdx = (s < 2) ? 0 : 1;
        float amp = smoothedAmplitudes[voiceIdx];
        bool isActive = (amp > 0.005f) && (s == 0 || s == 2);

        if (!isActive) continue;

        // Build vibrating string path — horizontal sine wave displacement
        juce::Path glowPath;
        int segs = 50;
        for (int i = 0; i <= segs; ++i)
        {
            float frac = static_cast<float>(i) / static_cast<float>(segs);
            auto pt = stringPosition(s, frac);

            // Vibration: horizontal displacement, max amplitude in the middle
            float envelope = std::sin(frac * juce::MathConstants<float>::pi);
            float vibrationPx = amp * 8.0f * svgScale * envelope *
                std::sin(frac * 10.0f + animPhase * (2.5f + static_cast<float>(s) * 0.4f));

            float px = pt.x + vibrationPx;
            float py = pt.y;

            if (i == 0) glowPath.startNewSubPath(px, py);
            else        glowPath.lineTo(px, py);
        }

        // Bright gold string overlay
        g.setColour(juce::Colour(0xffE8D5A3).withAlpha(0.5f + amp * 0.5f));
        g.strokePath(glowPath, juce::PathStrokeType((2.0f + amp * 2.0f) * svgScale));

        // Outer glow
        g.setColour(juce::Colour(0xffE8D5A3).withAlpha(amp * 0.3f));
        g.strokePath(glowPath, juce::PathStrokeType(glowWidth[s] * svgScale));

        // Wide soft glow
        g.setColour(theme::color::goldBright.withAlpha(amp * 0.1f));
        g.strokePath(glowPath, juce::PathStrokeType(glowWidth[s] * svgScale * 2.5f));
    }
}

void VeenaVisualization::drawFingerPosition(juce::Graphics& g)
{
    if (activeNote < 0) return;

    float notePos = static_cast<float>(activeNote - BASE_MIDI_NOTE) + pitchOffset;
    notePos = juce::jlimit(0.0f, static_cast<float>(NUM_FRETS), notePos);
    float fretFrac = notePos / static_cast<float>(NUM_FRETS);

    // Finger on the Sa string (string 0)
    auto pt = stringPosition(0, fretFrac);
    float amp = smoothedAmplitudes[0];

    // Add vibration offset to finger position too
    float envelope = std::sin(fretFrac * juce::MathConstants<float>::pi);
    float vibPx = amp * 8.0f * svgScale * envelope *
        std::sin(fretFrac * 10.0f + animPhase * 2.5f);
    pt.x += vibPx;

    float dotSize = (6.0f + amp * 4.0f) * svgScale;

    // Wide glow
    g.setColour(theme::color::goldBright.withAlpha(0.2f + amp * 0.3f));
    g.fillEllipse(pt.x - dotSize * 2, pt.y - dotSize * 2, dotSize * 4, dotSize * 4);

    // Bright dot
    float innerSize = 4.0f * svgScale;
    g.setColour(theme::color::goldBright);
    g.fillEllipse(pt.x - innerSize, pt.y - innerSize, innerSize * 2, innerSize * 2);

    // White center
    float centerSize = 1.5f * svgScale;
    g.setColour(juce::Colour(0xffFFFFDD));
    g.fillEllipse(pt.x - centerSize, pt.y - centerSize, centerSize * 2, centerSize * 2);
}

void VeenaVisualization::drawPluckFlash(juce::Graphics& g)
{
    // Flash on thalam strings
    for (int i = 0; i < 3; ++i)
    {
        float flash = thalamFlash[i];
        if (flash < 0.05f) continue;

        // Approximate thalam string positions from SVG
        float svgX = 60.0f - static_cast<float>(i) * 6.0f;
        float svgY = 520.0f + static_cast<float>(i) * 14.0f;
        auto pt = svgToComponent(svgX, svgY);

        float r = (8.0f + flash * 10.0f) * svgScale;
        g.setColour(theme::color::goldBright.withAlpha(flash * 0.5f));
        g.fillEllipse(pt.x - r, pt.y - r, r * 2, r * 2);
    }

    // Pluck flash on main string at note position
    float amp0 = smoothedAmplitudes[0];
    if (amp0 > 0.3f && activeNote >= 0)
    {
        float notePos = static_cast<float>(activeNote - BASE_MIDI_NOTE) + pitchOffset;
        notePos = juce::jlimit(0.0f, static_cast<float>(NUM_FRETS), notePos);
        float fretFrac = notePos / static_cast<float>(NUM_FRETS);
        auto pt = stringPosition(0, fretFrac);

        float flashR = amp0 * 12.0f * svgScale;
        g.setColour(juce::Colour(0xffFFFFDD).withAlpha((amp0 - 0.3f) * 0.6f));
        g.fillEllipse(pt.x - flashR, pt.y - flashR, flashR * 2, flashR * 2);
    }
}

void VeenaVisualization::drawKudamGlow(juce::Graphics& g)
{
    if (smoothedPeak < 0.01f) return;

    auto center = svgToComponent(veenaSVG::kudamCenterX, veenaSVG::kudamCenterY);
    float r = (55.0f + smoothedPeak * 12.0f) * svgScale;

    g.setColour(theme::color::woodLight.withAlpha(smoothedPeak * 0.12f));
    g.fillEllipse(center.x - r, center.y - r, r * 2, r * 2);
}
