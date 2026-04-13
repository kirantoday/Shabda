#include "VeenaVisualization.h"
#include <cmath>

VeenaVisualization::VeenaVisualization() { startTimerHz(30); }
VeenaVisualization::~VeenaVisualization() { stopTimer(); }

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

// Map a position t (0=yali, 1=kudam) along the veena axis to pixel coords.
// perpOffset shifts perpendicular to the axis (positive = below/right).
juce::Point<float> VeenaVisualization::veenaPoint(float t, float perpOffset) const
{
    float x = axisStart.x + (axisEnd.x - axisStart.x) * t;
    float y = axisStart.y + (axisEnd.y - axisStart.y) * t;
    // Perpendicular direction (rotated 90 degrees from axis)
    float px = -std::sin(axisAngle) * perpOffset;
    float py =  std::cos(axisAngle) * perpOffset;
    return { x + px, y + py };
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

    // Compute the veena's main axis: 30 degrees, yali upper-left, kudam lower-right.
    // Leave margins so the instrument fits nicely.
    float margin = 25.0f;
    axisStart = { bounds.getX() + margin + 30, bounds.getY() + margin };          // yali
    axisEnd   = { bounds.getRight() - margin - 20, bounds.getBottom() - margin };  // kudam
    axisLength = axisStart.getDistanceFrom(axisEnd);
    axisAngle = std::atan2(axisEnd.y - axisStart.y, axisEnd.x - axisStart.x);

    drawShadow(g);
    drawNeckBeam(g);
    drawUpperGourd(g);
    drawKudam(g);
    drawYali(g);
    drawFrets(g);
    drawBridge(g);
    drawMainStrings(g);
    drawThalamStrings(g);
    drawFingerPosition(g);
}

void VeenaVisualization::drawShadow(juce::Graphics& g)
{
    // Subtle drop shadow under the instrument
    juce::Path shadow;
    auto p0 = veenaPoint(0.0f, 12);
    auto p1 = veenaPoint(1.0f, 12);
    shadow.startNewSubPath(p0);
    for (float t = 0.05f; t <= 1.0f; t += 0.05f)
        shadow.lineTo(veenaPoint(t, 14 + std::sin(t * 3.14f) * 8));
    g.setColour(juce::Colour(0x18000000));
    g.strokePath(shadow, juce::PathStrokeType(20.0f));
}

void VeenaVisualization::drawNeckBeam(juce::Graphics& g)
{
    // Tapered wooden beam: wider at kudam end, narrower at yali end.
    // t=0 (yali): half-width ~8px. t=0.85 (before kudam): half-width ~14px.
    juce::Path neckTop, neckBottom;
    int steps = 40;

    for (int i = 0; i <= steps; ++i)
    {
        float t = static_cast<float>(i) / static_cast<float>(steps);
        if (t > 0.88f) break;  // stop before kudam

        // Taper: narrow at yali, wider toward kudam
        float halfW = 7.0f + t * 8.0f;

        auto top = veenaPoint(t, -halfW);
        auto bot = veenaPoint(t, halfW);

        if (i == 0) { neckTop.startNewSubPath(top); neckBottom.startNewSubPath(bot); }
        else        { neckTop.lineTo(top); neckBottom.lineTo(bot); }
    }

    // Close the shape: go back along the bottom
    juce::Path neckShape;
    neckShape.startNewSubPath(veenaPoint(0.0f, -7.0f));
    for (int i = 1; i <= steps; ++i)
    {
        float t = static_cast<float>(i) / static_cast<float>(steps);
        if (t > 0.88f) { t = 0.88f; neckShape.lineTo(veenaPoint(t, -(7.0f + t * 8.0f))); break; }
        neckShape.lineTo(veenaPoint(t, -(7.0f + t * 8.0f)));
    }
    for (int i = steps; i >= 0; --i)
    {
        float t = static_cast<float>(i) / static_cast<float>(steps);
        if (t > 0.88f) continue;
        neckShape.lineTo(veenaPoint(t, 7.0f + t * 8.0f));
    }
    neckShape.closeSubPath();

    // Wood gradient fill along the neck
    auto nStart = veenaPoint(0.0f, 0);
    auto nEnd = veenaPoint(0.88f, 0);
    juce::ColourGradient woodGrad(juce::Colour(0xff3E2723), nStart.x, nStart.y,
                                   juce::Colour(0xff5D4037), nEnd.x, nEnd.y, false);
    woodGrad.addColour(0.4, juce::Colour(0xff4E342E));
    g.setGradientFill(woodGrad);
    g.fillPath(neckShape);

    // Top edge highlight
    g.setColour(juce::Colour(0x20FFFFFF));
    g.strokePath(neckTop, juce::PathStrokeType(0.8f));

    // Outline
    g.setColour(juce::Colour(0xff6D4C41).withAlpha(0.5f));
    g.strokePath(neckShape, juce::PathStrokeType(0.8f));
}

void VeenaVisualization::drawKudam(juce::Graphics& g)
{
    // Large gourd body at the kudam end (t ≈ 0.92).
    auto center = veenaPoint(0.92f, 0);
    float rx = 38.0f;
    float ry = 32.0f;

    // Glow when active
    if (smoothedPeak > 0.01f)
    {
        g.setColour(theme::color::woodLight.withAlpha(smoothedPeak * 0.2f));
        g.fillEllipse(center.x - rx - 5, center.y - ry - 5, (rx + 5) * 2, (ry + 5) * 2);
    }

    // Wood gradient body
    juce::ColourGradient bodyGrad(juce::Colour(0xffA0662B), center.x - rx * 0.3f, center.y - ry * 0.3f,
                                   juce::Colour(0xff4E2C0A), center.x + rx * 0.3f, center.y + ry * 0.3f, true);
    g.setGradientFill(bodyGrad);
    g.fillEllipse(center.x - rx, center.y - ry, rx * 2, ry * 2);

    // Wood grain lines
    g.setColour(juce::Colour(0x0A000000));
    for (int i = -2; i <= 2; ++i)
        g.drawLine(center.x - rx * 0.6f, center.y + static_cast<float>(i) * 10,
                   center.x + rx * 0.6f, center.y + static_cast<float>(i) * 10 + 2, 0.5f);

    // Outline
    g.setColour(theme::color::woodLight.withAlpha(0.3f));
    g.drawEllipse(center.x - rx, center.y - ry, rx * 2, ry * 2, 1.0f);

    // Sound hole
    float hr = 7.0f;
    g.setColour(theme::color::background.withAlpha(0.8f));
    g.fillEllipse(center.x - hr, center.y - hr * 0.7f, hr * 2, hr * 1.4f);
    g.setColour(theme::color::gold.withAlpha(0.2f));
    g.drawEllipse(center.x - hr, center.y - hr * 0.7f, hr * 2, hr * 1.4f, 0.5f);
}

void VeenaVisualization::drawUpperGourd(juce::Graphics& g)
{
    // Small gourd near the yali (t ≈ 0.06).
    auto center = veenaPoint(0.06f, -18);
    float r = 14.0f;

    juce::ColourGradient gourdGrad(juce::Colour(0xff8B6914), center.x - 4, center.y - 4,
                                    juce::Colour(0xff5D4037), center.x + 4, center.y + 4, true);
    g.setGradientFill(gourdGrad);
    g.fillEllipse(center.x - r, center.y - r, r * 2, r * 2);
    g.setColour(juce::Colour(0xff8B7D5A).withAlpha(0.3f));
    g.drawEllipse(center.x - r, center.y - r, r * 2, r * 2, 0.7f);
}

void VeenaVisualization::drawYali(juce::Graphics& g)
{
    // Stylized dragon head (yali) silhouette at the top-left end.
    // Simple decorative shape: curved head + jaw.
    auto tip = veenaPoint(-0.02f, 0);
    auto neckJoin = veenaPoint(0.03f, 0);

    juce::Path yali;
    // Head crown (upper curve)
    yali.startNewSubPath(neckJoin.x, neckJoin.y - 8);
    yali.cubicTo(tip.x + 5, tip.y - 18,
                 tip.x - 8, tip.y - 14,
                 tip.x - 5, tip.y - 2);
    // Jaw (lower curve)
    yali.cubicTo(tip.x - 6, tip.y + 6,
                 tip.x + 2, tip.y + 12,
                 neckJoin.x, neckJoin.y + 8);
    yali.closeSubPath();

    g.setColour(theme::color::gold.withAlpha(0.5f));
    g.fillPath(yali);
    g.setColour(theme::color::goldBright.withAlpha(0.3f));
    g.strokePath(yali, juce::PathStrokeType(0.8f));

    // Eye dot
    g.setColour(theme::color::goldBright.withAlpha(0.7f));
    g.fillEllipse(tip.x - 2, tip.y - 7, 3.0f, 3.0f);
}

void VeenaVisualization::drawFrets(juce::Graphics& g)
{
    // Brass fret dots/lines along the neck from t=0.05 to t=0.85.
    float fretStart = 0.05f;
    float fretEnd = 0.85f;

    for (int i = 0; i <= NUM_FRETS; ++i)
    {
        float t = fretStart + (fretEnd - fretStart) * static_cast<float>(i) / static_cast<float>(NUM_FRETS);
        int fretNote = BASE_MIDI_NOTE + i;
        bool isActive = (fretNote == activeNote);

        float halfW = 7.0f + t * 8.0f;  // match neck taper

        if (isActive)
        {
            // Active fret: brighter, wider
            auto top = veenaPoint(t, -halfW);
            auto bot = veenaPoint(t, halfW);
            g.setColour(theme::color::fretActive.withAlpha(0.6f));
            g.drawLine(top.x, top.y, bot.x, bot.y, 1.5f);

            // Glow
            g.setColour(theme::color::fretActive.withAlpha(0.1f));
            g.drawLine(top.x, top.y, bot.x, bot.y, 6.0f);
        }
        else
        {
            // Small gold dot on the neck centerline
            auto pt = veenaPoint(t, 0);
            g.setColour(theme::color::gold.withAlpha(0.25f));
            g.fillEllipse(pt.x - 1.2f, pt.y - 1.2f, 2.4f, 2.4f);
        }
    }
}

void VeenaVisualization::drawBridge(juce::Graphics& g)
{
    // Brass bridge where strings meet the kudam (t ≈ 0.87).
    float t = 0.87f;
    float halfW = 7.0f + t * 8.0f;
    auto top = veenaPoint(t, -halfW - 2);
    auto bot = veenaPoint(t, halfW + 2);

    // Brass bar
    g.setColour(theme::color::goldBright.withAlpha(0.7f));
    g.drawLine(top.x, top.y, bot.x, bot.y, 3.0f);

    // Highlight
    g.setColour(juce::Colour(0x30FFFFFF));
    g.drawLine(top.x - 0.5f, top.y, bot.x - 0.5f, bot.y, 0.5f);
}

void VeenaVisualization::drawMainStrings(juce::Graphics& g)
{
    // 4 main strings along the neck, different thicknesses.
    const float thickness[] = { 3.0f, 2.5f, 2.0f, 1.5f };
    const float activeThick[] = { 3.8f, 3.0f, 2.5f, 2.0f };
    const char* names[] = { "Sa", "Pa", "sa", "Pa" };

    // Strings spread: perpendicular offsets from center axis
    float stringSpread[] = { -5.0f, -1.7f, 1.7f, 5.0f };

    float stringStart = 0.04f;  // just past yali
    float stringEnd = 0.87f;    // at the bridge

    for (int s = 0; s < 4; ++s)
    {
        int voiceIdx = (s < 2) ? 0 : 1;
        float amp = smoothedAmplitudes[voiceIdx];
        bool isActive = (amp > 0.01f) && (s == 0 || s == 2);

        float thick = isActive ? activeThick[s] : thickness[s];
        float offset = stringSpread[s];

        // Build string path
        juce::Path stringPath;
        int segs = 60;
        for (int i = 0; i <= segs; ++i)
        {
            float frac = static_cast<float>(i) / static_cast<float>(segs);
            float t = stringStart + frac * (stringEnd - stringStart);

            // Vibration: perpendicular displacement
            float envelope = std::sin(frac * juce::MathConstants<float>::pi);
            float vibration = amp * 4.0f * envelope *
                std::sin(frac * 12.0f + animPhase * (2.0f + static_cast<float>(s) * 0.3f));

            auto pt = veenaPoint(t, offset + vibration);
            if (i == 0) stringPath.startNewSubPath(pt);
            else        stringPath.lineTo(pt);
        }

        // Shadow
        {
            juce::Path shadowPath;
            for (int i = 0; i <= segs; ++i)
            {
                float frac = static_cast<float>(i) / static_cast<float>(segs);
                float t = stringStart + frac * (stringEnd - stringStart);
                float envelope = std::sin(frac * juce::MathConstants<float>::pi);
                float vibration = amp * 4.0f * envelope *
                    std::sin(frac * 12.0f + animPhase * (2.0f + static_cast<float>(s) * 0.3f));
                auto pt = veenaPoint(t, offset + vibration + 1.5f);
                if (i == 0) shadowPath.startNewSubPath(pt);
                else        shadowPath.lineTo(pt);
            }
            g.setColour(juce::Colour(0x18000000));
            g.strokePath(shadowPath, juce::PathStrokeType(thick + 0.5f));
        }

        // String
        juce::Colour col = isActive
            ? theme::color::stringActive.withAlpha(0.85f + amp * 0.15f)
            : theme::color::stringIdle.withAlpha(0.5f);
        g.setColour(col);
        g.strokePath(stringPath, juce::PathStrokeType(thick));

        // Glow
        if (isActive && amp > 0.02f)
        {
            g.setColour(theme::color::stringGlow.withAlpha(amp * 0.4f));
            g.strokePath(stringPath, juce::PathStrokeType(thick + 4.0f));
            g.setColour(theme::color::goldBright.withAlpha(amp * 0.1f));
            g.strokePath(stringPath, juce::PathStrokeType(thick + 9.0f));
        }

        // String name label near yali end
        auto labelPt = veenaPoint(stringStart - 0.02f, offset);
        g.setColour(theme::color::textSecondary.withAlpha(0.4f));
        g.setFont(juce::FontOptions(8.0f));
        g.drawText(names[s], static_cast<int>(labelPt.x) - 14, static_cast<int>(labelPt.y) - 5,
                   14, 10, juce::Justification::centredRight, false);
    }
}

void VeenaVisualization::drawThalamStrings(juce::Graphics& g)
{
    // 3 shorter thalam strings branching off near the kudam (t ≈ 0.82..0.87).
    const char* labels[] = { "Z", "X", "C" };

    for (int i = 0; i < 3; ++i)
    {
        float t = 0.82f + static_cast<float>(i) * 0.015f;
        float perpStart = 10.0f + static_cast<float>(i) * 4.0f;  // angled outward
        float perpEnd = 20.0f + static_cast<float>(i) * 6.0f;

        auto start = veenaPoint(t, perpStart);
        auto end = veenaPoint(t + 0.06f, perpEnd);
        float flash = thalamFlash[i];

        juce::Colour col = (flash > 0.05f)
            ? theme::color::goldBright.withAlpha(0.5f + flash * 0.5f)
            : theme::color::stringIdle.withAlpha(0.35f);

        g.setColour(col);
        g.drawLine(start.x, start.y, end.x, end.y, flash > 0.05f ? 2.0f : 1.2f);

        if (flash > 0.1f)
        {
            g.setColour(theme::color::goldBright.withAlpha(flash * 0.2f));
            g.drawLine(start.x, start.y, end.x, end.y, 5.0f);
        }

        // Label
        g.setColour(theme::color::gold.withAlpha(0.8f));
        g.setFont(juce::FontOptions(10.0f, juce::Font::bold));
        g.drawText(labels[i], static_cast<int>(end.x) + 2, static_cast<int>(end.y) - 6,
                   14, 12, juce::Justification::centredLeft, false);
    }
}

void VeenaVisualization::drawFingerPosition(juce::Graphics& g)
{
    if (activeNote < 0) return;

    float fretStart = 0.05f;
    float fretEnd = 0.85f;
    float notePos = static_cast<float>(activeNote - BASE_MIDI_NOTE) + pitchOffset;
    notePos = juce::jlimit(0.0f, static_cast<float>(NUM_FRETS), notePos);
    float t = fretStart + (fretEnd - fretStart) * notePos / static_cast<float>(NUM_FRETS);

    // Place finger on the first string (Sa)
    auto pt = veenaPoint(t, -5.0f);
    float amp = smoothedAmplitudes[0];
    float dotSize = 6.0f + amp * 3.0f;

    // Glow
    g.setColour(theme::color::goldBright.withAlpha(0.25f + amp * 0.3f));
    g.fillEllipse(pt.x - dotSize, pt.y - dotSize, dotSize * 2, dotSize * 2);

    // Dot
    g.setColour(theme::color::goldBright);
    g.fillEllipse(pt.x - 3.5f, pt.y - 3.5f, 7.0f, 7.0f);
}
