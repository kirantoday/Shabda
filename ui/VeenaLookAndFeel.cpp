#include "VeenaLookAndFeel.h"

VeenaLookAndFeel::VeenaLookAndFeel()
{
    // Global color overrides for JUCE components.
    setColour(juce::PopupMenu::backgroundColourId, theme::color::panelSurface);
    setColour(juce::PopupMenu::textColourId, theme::color::textPrimary);
    setColour(juce::PopupMenu::highlightedBackgroundColourId, theme::color::gold.withAlpha(0.2f));
    setColour(juce::PopupMenu::highlightedTextColourId, theme::color::goldBright);
    setColour(juce::ComboBox::textColourId, theme::color::textPrimary);
    setColour(juce::ComboBox::arrowColourId, theme::color::gold);
    setColour(juce::Label::textColourId, theme::color::textPrimary);
}

// --- Rotary Slider (Knob) ---

void VeenaLookAndFeel::drawRotarySlider(juce::Graphics& g, int x, int y, int width, int height,
                                         float sliderPos, float rotaryStartAngle, float rotaryEndAngle,
                                         juce::Slider& /*slider*/)
{
    float radius = static_cast<float>(juce::jmin(width, height)) * 0.5f - 4.0f;
    float centreX = static_cast<float>(x) + static_cast<float>(width) * 0.5f;
    float centreY = static_cast<float>(y) + static_cast<float>(height) * 0.5f;

    // Background circle
    g.setColour(theme::color::knobBackground);
    g.fillEllipse(centreX - radius, centreY - radius, radius * 2.0f, radius * 2.0f);

    // Inactive ring (full arc)
    float ringThickness = 3.0f;
    juce::Path ringPath;
    ringPath.addCentredArc(centreX, centreY, radius - 1.0f, radius - 1.0f,
                           0.0f, rotaryStartAngle, rotaryEndAngle, true);
    g.setColour(theme::color::knobRing);
    g.strokePath(ringPath, juce::PathStrokeType(ringThickness, juce::PathStrokeType::curved,
                                                 juce::PathStrokeType::rounded));

    // Active arc (value)
    float angle = rotaryStartAngle + sliderPos * (rotaryEndAngle - rotaryStartAngle);
    juce::Path arcPath;
    arcPath.addCentredArc(centreX, centreY, radius - 1.0f, radius - 1.0f,
                          0.0f, rotaryStartAngle, angle, true);
    g.setColour(theme::color::knobArc);
    g.strokePath(arcPath, juce::PathStrokeType(ringThickness, juce::PathStrokeType::curved,
                                                juce::PathStrokeType::rounded));

    // Indicator line from center toward edge at current angle
    float indicatorLength = radius * 0.55f;
    float indicatorX = centreX + indicatorLength * std::cos(angle - juce::MathConstants<float>::halfPi);
    float indicatorY = centreY + indicatorLength * std::sin(angle - juce::MathConstants<float>::halfPi);
    g.setColour(theme::color::knobIndicator);
    g.drawLine(centreX, centreY, indicatorX, indicatorY, 2.0f);

    // Center dot
    g.fillEllipse(centreX - 2.0f, centreY - 2.0f, 4.0f, 4.0f);
}

// --- Linear Slider (Vertical Strip for Pitch Bend / Expression) ---

void VeenaLookAndFeel::drawLinearSlider(juce::Graphics& g, int x, int y, int width, int height,
                                         float sliderPos, float /*minSliderPos*/, float /*maxSliderPos*/,
                                         juce::Slider::SliderStyle style, juce::Slider& slider)
{
    if (style == juce::Slider::LinearVertical)
    {
        auto bounds = juce::Rectangle<float>(static_cast<float>(x), static_cast<float>(y),
                                              static_cast<float>(width), static_cast<float>(height));
        float cornerSize = 4.0f;

        // Background strip
        g.setColour(theme::color::stripFill);
        g.fillRoundedRectangle(bounds, cornerSize);
        g.setColour(theme::color::panelBorder);
        g.drawRoundedRectangle(bounds, cornerSize, 1.0f);

        // Fill from bottom (or center for pitch bend)
        float fillBottom = bounds.getBottom();
        float fillTop = sliderPos;

        bool isCentered = slider.getMinimum() < 0;  // pitch bend: -1..+1
        if (isCentered)
        {
            float center = bounds.getY() + bounds.getHeight() * 0.5f;
            if (sliderPos < center)
            {
                fillTop = sliderPos;
                fillBottom = center;
            }
            else
            {
                fillTop = center;
                fillBottom = sliderPos;
            }

            // Center notch line
            g.setColour(theme::color::knobRing);
            g.drawLine(bounds.getX() + 2, center, bounds.getRight() - 2, center, 1.0f);
        }
        else
        {
            fillTop = sliderPos;
        }

        // Gold fill
        auto fillRect = juce::Rectangle<float>(bounds.getX() + 2, fillTop,
                                                bounds.getWidth() - 4, fillBottom - fillTop);
        g.setColour(theme::color::gold.withAlpha(0.6f));
        g.fillRoundedRectangle(fillRect, 2.0f);

        // Thumb indicator
        g.setColour(theme::color::goldBright);
        g.fillRoundedRectangle(bounds.getX() + 1, sliderPos - 3,
                                bounds.getWidth() - 2, 6.0f, 3.0f);
    }
    else
    {
        // Horizontal slider fallback — simple bar
        auto bounds = juce::Rectangle<float>(static_cast<float>(x), static_cast<float>(y),
                                              static_cast<float>(width), static_cast<float>(height));
        g.setColour(theme::color::stripFill);
        g.fillRoundedRectangle(bounds, 3.0f);

        float fillWidth = sliderPos - bounds.getX();
        g.setColour(theme::color::gold.withAlpha(0.6f));
        g.fillRoundedRectangle(bounds.getX(), bounds.getY(), fillWidth, bounds.getHeight(), 3.0f);

        g.setColour(theme::color::goldBright);
        g.fillEllipse(sliderPos - 5, bounds.getCentreY() - 5, 10.0f, 10.0f);
    }
}

// --- Toggle Button (Pill) ---

void VeenaLookAndFeel::drawToggleButton(juce::Graphics& g, juce::ToggleButton& button,
                                         bool /*shouldDrawButtonAsHighlighted*/,
                                         bool /*shouldDrawButtonAsDown*/)
{
    bool isOn = button.getToggleState();
    auto bounds = button.getLocalBounds().toFloat();

    float pillW = static_cast<float>(theme::dim::toggleWidth);
    float pillH = static_cast<float>(theme::dim::toggleHeight);
    float pillX = bounds.getX();
    float pillY = bounds.getCentreY() - pillH * 0.5f;
    float cornerRadius = pillH * 0.5f;

    // Pill background
    g.setColour(isOn ? theme::color::toggleOn : theme::color::toggleOff);
    g.fillRoundedRectangle(pillX, pillY, pillW, pillH, cornerRadius);

    // Thumb circle
    float thumbDiam = pillH - 4.0f;
    float thumbX = isOn ? (pillX + pillW - thumbDiam - 2.0f) : (pillX + 2.0f);
    float thumbY = pillY + 2.0f;
    g.setColour(theme::color::textPrimary);
    g.fillEllipse(thumbX, thumbY, thumbDiam, thumbDiam);

    // Label text to the right of the pill
    g.setColour(isOn ? theme::color::textPrimary : theme::color::textSecondary);
    g.setFont(juce::FontOptions(theme::font::label));
    g.drawText(button.getButtonText(),
               juce::Rectangle<float>(pillX + pillW + 6.0f, bounds.getY(),
                                       bounds.getWidth() - pillW - 6.0f, bounds.getHeight()),
               juce::Justification::centredLeft, false);
}

// --- Combo Box ---

void VeenaLookAndFeel::drawComboBox(juce::Graphics& g, int width, int height,
                                     bool /*isButtonDown*/, int /*buttonX*/, int /*buttonY*/,
                                     int /*buttonW*/, int /*buttonH*/, juce::ComboBox& /*box*/)
{
    auto bounds = juce::Rectangle<float>(0, 0, static_cast<float>(width), static_cast<float>(height));
    float corner = 4.0f;

    g.setColour(theme::color::knobBackground);
    g.fillRoundedRectangle(bounds, corner);
    g.setColour(theme::color::gold.withAlpha(0.4f));
    g.drawRoundedRectangle(bounds.reduced(0.5f), corner, 1.0f);

    // Down arrow
    float arrowSize = 6.0f;
    float arrowX = static_cast<float>(width) - 16.0f;
    float arrowY = static_cast<float>(height) * 0.5f - 2.0f;
    juce::Path arrow;
    arrow.addTriangle(arrowX, arrowY, arrowX + arrowSize * 2, arrowY,
                      arrowX + arrowSize, arrowY + arrowSize);
    g.setColour(theme::color::gold);
    g.fillPath(arrow);
}

void VeenaLookAndFeel::drawPopupMenuBackground(juce::Graphics& g, int width, int height)
{
    g.fillAll(theme::color::panelSurface);
    g.setColour(theme::color::gold.withAlpha(0.3f));
    g.drawRect(0, 0, width, height, 1);
}

void VeenaLookAndFeel::drawPopupMenuItem(juce::Graphics& g, const juce::Rectangle<int>& area,
                                          bool /*isSeparator*/, bool isActive, bool isHighlighted,
                                          bool isTicked, bool /*hasSubMenu*/,
                                          const juce::String& text, const juce::String& /*shortcutKeyText*/,
                                          const juce::Drawable* /*icon*/, const juce::Colour* /*textColour*/)
{
    if (isHighlighted && isActive)
    {
        g.setColour(theme::color::gold.withAlpha(0.15f));
        g.fillRect(area);
    }

    g.setColour(isActive ? (isHighlighted ? theme::color::goldBright : theme::color::textPrimary)
                          : theme::color::textSecondary);
    g.setFont(juce::FontOptions(theme::font::label));

    auto textArea = area.reduced(10, 0);
    if (isTicked)
    {
        g.setColour(theme::color::gold);
        g.fillEllipse(static_cast<float>(area.getX() + 4),
                       static_cast<float>(area.getCentreY() - 3), 6.0f, 6.0f);
    }
    g.drawText(text, textArea, juce::Justification::centredLeft, true);
}

// --- Label ---

void VeenaLookAndFeel::drawLabel(juce::Graphics& g, juce::Label& label)
{
    g.setColour(label.findColour(juce::Label::textColourId));
    g.setFont(label.getFont());
    g.drawText(label.getText(), label.getLocalBounds(), label.getJustificationType(), false);
}

juce::Font VeenaLookAndFeel::getComboBoxFont(juce::ComboBox&)
{
    return juce::FontOptions(theme::font::label);
}

juce::Font VeenaLookAndFeel::getPopupMenuFont()
{
    return juce::FontOptions(theme::font::label);
}
