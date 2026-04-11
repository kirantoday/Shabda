#pragma once

#include "common/DSPConstants.h"
#include <cmath>
#include <algorithm>

// Curve-shaped pitch glide engine for legato meend transitions.
// This file must NOT include any JUCE headers — pure C++ only.
//
// MEEND GLIDE SHAPES:
//
// When the veena player slides from one note to another (meend), the
// pitch transition has a specific shape that depends on the musical
// context. Different ragas and phrases call for different glide characters.
//
// This engine tracks progress from 0 to 1 over a configurable time,
// applies a curve function to shape the transition, and interpolates
// between the start and end MIDI notes.
//
// Available curves:
//   Linear:      y = t                   Constant speed, mechanical feel.
//   Exponential: y = 1 - exp(-5t)        Fast departure, slow arrival (default).
//   SCurve:      y = 3t² - 2t³           Smooth start & end, musical. Best for
//                                         landing precisely on target swaras.
//   Late:        y = t²                  Slow start, accelerating. Dramatic meend
//                                         that builds tension before resolving.

namespace engine {

enum class GlideCurve : int
{
    Linear = 0,
    Exponential,
    SCurve,
    Late
};

class GlideEngine
{
public:
    void prepare(float newSampleRate)
    {
        sampleRate = newSampleRate;
    }

    void reset()
    {
        currentNote = targetNote;
        progress = 1.0f;  // not gliding
        gliding = false;
    }

    // Start a new glide from start to end note over the given time.
    void setGlide(float fromNote, float toNote, float timeMs)
    {
        startNote = fromNote;
        targetNote = toNote;
        totalSamples = sampleRate * timeMs / 1000.0f;
        if (totalSamples < 1.0f)
            totalSamples = 1.0f;
        elapsedSamples = 0.0f;
        progress = 0.0f;
        gliding = true;
    }

    // Snap immediately to a note (no glide). Used on retrigger.
    void snapTo(float note)
    {
        currentNote = note;
        targetNote = note;
        startNote = note;
        progress = 1.0f;
        gliding = false;
    }

    // Get the current note value. Call once per sample.
    float getNextNote()
    {
        if (!gliding)
            return currentNote;

        elapsedSamples += 1.0f;
        progress = std::min(elapsedSamples / totalSamples, 1.0f);

        float shaped = applyCurve(progress);
        currentNote = startNote + (targetNote - startNote) * shaped;

        if (progress >= 1.0f)
        {
            currentNote = targetNote;
            gliding = false;
        }

        return currentNote;
    }

    void setCurve(GlideCurve newCurve) { curve = newCurve; }
    GlideCurve getCurve() const { return curve; }
    bool isGliding() const { return gliding; }
    float getCurrentNote() const { return currentNote; }

private:
    float applyCurve(float t) const
    {
        switch (curve)
        {
            case GlideCurve::Linear:
                return t;

            case GlideCurve::Exponential:
            {
                // Fast departure, slow arrival.
                // y = (1 - exp(-5t)) / (1 - exp(-5))  — normalized to [0,1]
                const float k = 5.0f;
                const float normFactor = 1.0f / (1.0f - std::exp(-k));  // ≈ 1.0068
                return (1.0f - std::exp(-k * t)) * normFactor;
            }

            case GlideCurve::SCurve:
            {
                // Hermite smoothstep: smooth at both ends, fast in middle.
                // Most musical for precise swara landing.
                return t * t * (3.0f - 2.0f * t);
            }

            case GlideCurve::Late:
            {
                // Slow start, accelerating arrival. Dramatic tension.
                return t * t;
            }

            default:
                return t;
        }
    }

    float sampleRate = 48000.0f;
    float startNote = 60.0f;
    float targetNote = 60.0f;
    float currentNote = 60.0f;
    float totalSamples = 1.0f;
    float elapsedSamples = 0.0f;
    float progress = 1.0f;
    bool gliding = false;
    GlideCurve curve = GlideCurve::Exponential;
};

} // namespace engine
