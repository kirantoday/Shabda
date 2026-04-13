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

    // Fit the SVG into the component bounds, maintaining aspect ratio.
    auto bounds = getLocalBounds().toFloat().reduced(8, 8);
    float svgAspect = veenaSVG::viewBoxWidth / veenaSVG::viewBoxHeight;
    float compAspect = bounds.getWidth() / bounds.getHeight();

    if (compAspect > svgAspect)
    {
        // Component wider than SVG — fit by height
        svgScale = bounds.getHeight() / veenaSVG::viewBoxHeight;
        svgOffsetX = bounds.getX() + (bounds.getWidth() - veenaSVG::viewBoxWidth * svgScale) * 0.5f;
        svgOffsetY = bounds.getY();
    }
    else
    {
        // Component taller — fit by width
        svgScale = bounds.getWidth() / veenaSVG::viewBoxWidth;
        svgOffsetX = bounds.getX();
        svgOffsetY = bounds.getY() + (bounds.getHeight() - veenaSVG::viewBoxHeight * svgScale) * 0.5f;
    }
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
    // Interpolate between the string start and end positions defined in VeenaSVG.h.
    int s = juce::jlimit(0, 3, stringIndex);
    float f = juce::jlimit(0.0f, 1.0f, fretFraction);

    float svgX = veenaSVG::stringStartX + f * (veenaSVG::stringEndX - veenaSVG::stringStartX);
    float svgY = veenaSVG::stringStartY[s] + f * (veenaSVG::stringEndY[s] - veenaSVG::stringStartY[s]);

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

    // Render the SVG base layer
    if (svgDrawable != nullptr)
    {
        auto svgBounds = juce::Rectangle<float>(
            svgOffsetX, svgOffsetY,
            veenaSVG::viewBoxWidth * svgScale,
            veenaSVG::viewBoxHeight * svgScale);
        svgDrawable->drawWithin(g, svgBounds, juce::RectanglePlacement::stretchToFit, 1.0f);
    }

    // Interactive overlays on top
    drawKudamGlow(g);
    drawStringGlow(g);
    drawPluckFlash(g);
    drawFingerPosition(g);
}

void VeenaVisualization::drawStringGlow(juce::Graphics& g)
{
    // Glow on active strings — bright gold highlight following the string path.
    const float glowWidth[] = { 8.0f, 6.0f, 5.0f, 4.0f };

    for (int s = 0; s < 4; ++s)
    {
        int voiceIdx = (s < 2) ? 0 : 1;
        float amp = smoothedAmplitudes[voiceIdx];
        bool isActive = (amp > 0.01f) && (s == 0 || s == 2);

        if (!isActive) continue;

        // Draw glowing line along the string
        juce::Path glowPath;
        int segs = 40;
        for (int i = 0; i <= segs; ++i)
        {
            float frac = static_cast<float>(i) / static_cast<float>(segs);
            auto pt = stringPosition(s, frac);

            // Add vibration displacement perpendicular to the string
            float envelope = std::sin(frac * juce::MathConstants<float>::pi);
            float vibration = amp * 3.0f * svgScale * envelope *
                std::sin(frac * 12.0f + animPhase * (2.0f + static_cast<float>(s) * 0.3f));

            // Perpendicular offset (roughly downward for the diagonal)
            float px = pt.x - vibration * 0.38f;  // sin of ~22 degrees
            float py = pt.y + vibration * 0.92f;   // cos of ~22 degrees

            if (i == 0) glowPath.startNewSubPath(px, py);
            else        glowPath.lineTo(px, py);
        }

        // Outer glow
        g.setColour(juce::Colour(0xffE8D5A3).withAlpha(amp * 0.35f));
        g.strokePath(glowPath, juce::PathStrokeType(glowWidth[s] * svgScale));

        // Inner bright glow
        g.setColour(theme::color::goldBright.withAlpha(amp * 0.15f));
        g.strokePath(glowPath, juce::PathStrokeType(glowWidth[s] * svgScale * 2.0f));
    }
}

void VeenaVisualization::drawFingerPosition(juce::Graphics& g)
{
    if (activeNote < 0) return;

    float notePos = static_cast<float>(activeNote - BASE_MIDI_NOTE) + pitchOffset;
    notePos = juce::jlimit(0.0f, static_cast<float>(NUM_FRETS), notePos);
    float fretFrac = notePos / static_cast<float>(NUM_FRETS);

    // Place finger on the Sa string (string 0)
    auto pt = stringPosition(0, fretFrac);
    float amp = smoothedAmplitudes[0];
    float dotSize = (5.0f + amp * 3.0f) * svgScale;

    // Outer glow
    g.setColour(theme::color::goldBright.withAlpha(0.25f + amp * 0.35f));
    g.fillEllipse(pt.x - dotSize * 1.5f, pt.y - dotSize * 1.5f, dotSize * 3.0f, dotSize * 3.0f);

    // Inner dot
    float innerSize = 4.0f * svgScale;
    g.setColour(theme::color::goldBright);
    g.fillEllipse(pt.x - innerSize, pt.y - innerSize, innerSize * 2.0f, innerSize * 2.0f);

    // Tiny bright center
    float centerSize = 1.5f * svgScale;
    g.setColour(juce::Colour(0xffFFFFDD));
    g.fillEllipse(pt.x - centerSize, pt.y - centerSize, centerSize * 2.0f, centerSize * 2.0f);
}

void VeenaVisualization::drawPluckFlash(juce::Graphics& g)
{
    // Brief flash when a thalam string is plucked.
    // Thalam strings branch off near the bridge in the SVG.
    for (int i = 0; i < 3; ++i)
    {
        float flash = thalamFlash[i];
        if (flash < 0.05f) continue;

        // Flash along thalam string area (approximate positions from SVG)
        float svgX = 730.0f - static_cast<float>(i) * 10.0f;
        float svgY = 270.0f + static_cast<float>(i) * 12.0f;
        auto pt = svgToComponent(svgX, svgY);

        float r = (10.0f + flash * 8.0f) * svgScale;
        g.setColour(theme::color::goldBright.withAlpha(flash * 0.4f));
        g.fillEllipse(pt.x - r, pt.y - r, r * 2, r * 2);
    }
}

void VeenaVisualization::drawKudamGlow(juce::Graphics& g)
{
    // Subtle glow around the kudam when sound is active.
    if (smoothedPeak < 0.01f) return;

    auto center = svgToComponent(790.0f, 260.0f);
    float r = (80.0f + smoothedPeak * 15.0f) * svgScale;

    g.setColour(theme::color::woodLight.withAlpha(smoothedPeak * 0.12f));
    g.fillEllipse(center.x - r, center.y - r, r * 2, r * 2);
}
