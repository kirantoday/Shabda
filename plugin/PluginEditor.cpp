#include "PluginEditor.h"
#include "PluginProcessor.h"

// --- Helper: create knob with common settings ---
static void setupKnob(juce::Slider& knob, double min, double max, double defaultVal, double step = 0.01)
{
    knob.setSliderStyle(juce::Slider::RotaryVerticalDrag);
    knob.setRange(min, max, step);
    knob.setValue(defaultVal, juce::dontSendNotification);
    knob.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
}

static void setupVerticalStrip(juce::Slider& strip, double min, double max, double defaultVal)
{
    strip.setSliderStyle(juce::Slider::LinearVertical);
    strip.setRange(min, max, 0.001);
    strip.setValue(defaultVal, juce::dontSendNotification);
    strip.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
}

static void setupCombo(juce::ComboBox& combo)
{
    combo.setJustificationType(juce::Justification::centredLeft);
}

// --- Constructor ---

VeenaPluginEditor::VeenaPluginEditor(VeenaPluginProcessor& p)
    : AudioProcessorEditor(&p),
      processorRef(p),
      keyboardComponent(p.keyboardState, juce::MidiKeyboardComponent::horizontalKeyboard)
{
    setLookAndFeel(&veenaLnF);

    // --- Header ---
    logoLabel = std::unique_ptr<juce::Label>(makeLabel("SHABDA", 24.0f,
                                                        theme::color::gold, juce::Justification::centredLeft));
    logoLabel->setFont(juce::FontOptions(24.0f, juce::Font::bold));
    instrumentLabel = std::unique_ptr<juce::Label>(makeLabel("Saraswati Veena", theme::font::subtitle,
                                                              theme::color::textSecondary, juce::Justification::centred));
    addAndMakeVisible(*logoLabel);
    addAndMakeVisible(*instrumentLabel);

    // Preset selector
    setupCombo(presetCombo);
    presetCombo.addItem("Classic Carnatic", 1);
    presetCombo.addItem("Film Score", 2);
    presetCombo.addItem("Bright Pluck", 3);
    presetCombo.addItem("Deep Meend", 4);
    presetCombo.addItem("Devotional", 5);
    presetCombo.addItem("Fusion", 6);
    presetCombo.setSelectedId(1, juce::dontSendNotification);
    addAndMakeVisible(presetCombo);

    presetPrev.setColour(juce::TextButton::buttonColourId, theme::color::panelSurface);
    presetPrev.setColour(juce::TextButton::textColourOffId, theme::color::gold);
    presetPrev.onClick = [this]() {
        int id = presetCombo.getSelectedId();
        if (id > 1) presetCombo.setSelectedId(id - 1);
    };
    addAndMakeVisible(presetPrev);

    presetNext.setColour(juce::TextButton::buttonColourId, theme::color::panelSurface);
    presetNext.setColour(juce::TextButton::textColourOffId, theme::color::gold);
    presetNext.onClick = [this]() {
        int id = presetCombo.getSelectedId();
        if (id < presetCombo.getNumItems()) presetCombo.setSelectedId(id + 1);
    };
    addAndMakeVisible(presetNext);

    // --- Visualization ---
    addAndMakeVisible(visualization);

    // --- String controls ---
    stringGroupLabel = std::unique_ptr<juce::Label>(
        makeLabel("STRING", theme::font::groupHeader, theme::color::textSecondary));
    addAndMakeVisible(*stringGroupLabel);

    setupKnob(pluckPositionKnob, 0.05, 0.5, 0.15);
    pluckPositionKnob.addListener(this);
    addAndMakeVisible(pluckPositionKnob);
    pluckPosLabel = std::unique_ptr<juce::Label>(makeLabel("Position", theme::font::label, theme::color::textSecondary));
    pluckPosValue = std::unique_ptr<juce::Label>(makeLabel("0.15", theme::font::value, theme::color::textValue));
    addAndMakeVisible(*pluckPosLabel);
    addAndMakeVisible(*pluckPosValue);

    setupKnob(dampingKnob, 0.0, 0.95, 0.4);
    dampingKnob.addListener(this);
    addAndMakeVisible(dampingKnob);
    dampingLabel = std::unique_ptr<juce::Label>(makeLabel("Damping", theme::font::label, theme::color::textSecondary));
    dampingValue = std::unique_ptr<juce::Label>(makeLabel("0.40", theme::font::value, theme::color::textValue));
    addAndMakeVisible(*dampingLabel);
    addAndMakeVisible(*dampingValue);

    setupKnob(brightnessKnob, 0.0, 1.0, 0.65);
    brightnessKnob.addListener(this);
    addAndMakeVisible(brightnessKnob);
    brightnessLabel = std::unique_ptr<juce::Label>(makeLabel("Bright", theme::font::label, theme::color::textSecondary));
    brightnessValue = std::unique_ptr<juce::Label>(makeLabel("0.65", theme::font::value, theme::color::textValue));
    addAndMakeVisible(*brightnessLabel);
    addAndMakeVisible(*brightnessValue);

    // --- Expression controls ---
    exprGroupLabel = std::unique_ptr<juce::Label>(
        makeLabel("EXPRESSION", theme::font::groupHeader, theme::color::textSecondary));
    addAndMakeVisible(*exprGroupLabel);

    setupVerticalStrip(pitchBendStrip, -1.0, 1.0, 0.0);
    pitchBendStrip.setDoubleClickReturnValue(true, 0.0);
    pitchBendStrip.addListener(this);
    addAndMakeVisible(pitchBendStrip);
    pitchBendLabel = std::unique_ptr<juce::Label>(makeLabel("Bend", theme::font::label, theme::color::textSecondary));
    pitchBendValue = std::unique_ptr<juce::Label>(makeLabel("0.0", theme::font::value, theme::color::textValue));
    addAndMakeVisible(*pitchBendLabel);
    addAndMakeVisible(*pitchBendValue);

    setupKnob(vibratoKnob, 0.0, 1.0, 0.0);
    vibratoKnob.addListener(this);
    addAndMakeVisible(vibratoKnob);
    vibratoLabel = std::unique_ptr<juce::Label>(makeLabel("Vibrato", theme::font::label, theme::color::textSecondary));
    vibratoValue = std::unique_ptr<juce::Label>(makeLabel("0.00", theme::font::value, theme::color::textValue));
    addAndMakeVisible(*vibratoLabel);
    addAndMakeVisible(*vibratoValue);

    setupVerticalStrip(expressionStrip, 0.0, 1.0, 1.0);
    expressionStrip.addListener(this);
    addAndMakeVisible(expressionStrip);
    expressionLabel = std::unique_ptr<juce::Label>(makeLabel("Expr", theme::font::label, theme::color::textSecondary));
    expressionValue = std::unique_ptr<juce::Label>(makeLabel("100%", theme::font::value, theme::color::textValue));
    addAndMakeVisible(*expressionLabel);
    addAndMakeVisible(*expressionValue);

    // --- Resonance & Settings ---
    resonGroupLabel = std::unique_ptr<juce::Label>(
        makeLabel("RESONANCE", theme::font::groupHeader, theme::color::textSecondary));
    addAndMakeVisible(*resonGroupLabel);

    setupKnob(bodyMixKnob, 0.0, 1.0, 0.5);
    bodyMixKnob.addListener(this);
    addAndMakeVisible(bodyMixKnob);
    bodyMixLabel = std::unique_ptr<juce::Label>(makeLabel("Body", theme::font::label, theme::color::textSecondary));
    bodyMixValue = std::unique_ptr<juce::Label>(makeLabel("0.50", theme::font::value, theme::color::textValue));
    addAndMakeVisible(*bodyMixLabel);
    addAndMakeVisible(*bodyMixValue);

    setupKnob(sympatheticKnob, 0.0, 0.5, 0.15);
    sympatheticKnob.addListener(this);
    addAndMakeVisible(sympatheticKnob);
    sympatheticLabel = std::unique_ptr<juce::Label>(makeLabel("Sympath", theme::font::label, theme::color::textSecondary));
    sympatheticValue = std::unique_ptr<juce::Label>(makeLabel("0.15", theme::font::value, theme::color::textValue));
    addAndMakeVisible(*sympatheticLabel);
    addAndMakeVisible(*sympatheticValue);

    setupKnob(thalamKnob, 0.0, 1.0, 0.6);
    thalamKnob.addListener(this);
    addAndMakeVisible(thalamKnob);
    thalamLabel = std::unique_ptr<juce::Label>(makeLabel("Thalam", theme::font::label, theme::color::textSecondary));
    thalamValue = std::unique_ptr<juce::Label>(makeLabel("0.60", theme::font::value, theme::color::textValue));
    addAndMakeVisible(*thalamLabel);
    addAndMakeVisible(*thalamValue);

    settingsGroupLabel = std::unique_ptr<juce::Label>(
        makeLabel("SETTINGS", theme::font::groupHeader, theme::color::textSecondary,
                  juce::Justification::centredLeft));
    addAndMakeVisible(*settingsGroupLabel);

    // Combo boxes
    setupCombo(bodyModeCombo);
    bodyModeCombo.addItem("Modal Filters", 1);
    bodyModeCombo.addItem("Convolution", 2);
    bodyModeCombo.addItem("Hybrid", 3);
    bodyModeCombo.setSelectedId(3, juce::dontSendNotification);
    bodyModeCombo.addListener(this);
    addAndMakeVisible(bodyModeCombo);

    setupCombo(ragaCombo);
    ragaCombo.addItem("Free", 1);
    ragaCombo.addItem("Shankarabharanam", 2);
    ragaCombo.addItem("Kalyani", 3);
    ragaCombo.addItem("Todi", 4);
    ragaCombo.addItem("Bhairavi", 5);
    ragaCombo.addItem("Kharaharapriya", 6);
    ragaCombo.addItem("Mohanam", 7);
    ragaCombo.setSelectedId(1, juce::dontSendNotification);
    ragaCombo.addListener(this);
    addAndMakeVisible(ragaCombo);

    setupCombo(tuningCombo);
    const char* noteNames[] = { "C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B" };
    for (int i = 0; i < 12; ++i)
        tuningCombo.addItem(juce::String("Sa=") + noteNames[i] + "3", i + 1);
    tuningCombo.setSelectedId(1, juce::dontSendNotification);
    tuningCombo.addListener(this);
    addAndMakeVisible(tuningCombo);

    setupCombo(bendRangeCombo);
    for (int i = 1; i <= 12; ++i)
        bendRangeCombo.addItem(juce::String(i) + "st", i);
    bendRangeCombo.setSelectedId(7, juce::dontSendNotification);
    bendRangeCombo.addListener(this);
    addAndMakeVisible(bendRangeCombo);

    setupCombo(glideCurveCombo);
    glideCurveCombo.addItem("Linear", 1);
    glideCurveCombo.addItem("Exponential", 2);
    glideCurveCombo.addItem("S-Curve", 3);
    glideCurveCombo.addItem("Late", 4);
    glideCurveCombo.setSelectedId(2, juce::dontSendNotification);
    glideCurveCombo.addListener(this);
    addAndMakeVisible(glideCurveCombo);

    legatoToggle.setButtonText("Legato");
    legatoToggle.setToggleState(true, juce::dontSendNotification);
    legatoToggle.onClick = [this]() {
        processorRef.uiLegatoEnabled.store(legatoToggle.getToggleState(), std::memory_order_relaxed);
    };
    addAndMakeVisible(legatoToggle);

    // --- Keyboard ---
    keyboardComponent.setKeyPressBaseOctave(5);
    keyboardComponent.setAvailableRange(36, 96);
    // Dark-themed keyboard with gold highlights
    keyboardComponent.setColour(juce::MidiKeyboardComponent::whiteNoteColourId, juce::Colour(0xff2A2A3E));
    keyboardComponent.setColour(juce::MidiKeyboardComponent::blackNoteColourId, juce::Colour(0xff151525));
    keyboardComponent.setColour(juce::MidiKeyboardComponent::keySeparatorLineColourId, juce::Colour(0xff444466));
    keyboardComponent.setColour(juce::MidiKeyboardComponent::keyDownOverlayColourId, theme::color::gold.withAlpha(0.35f));
    keyboardComponent.setColour(juce::MidiKeyboardComponent::mouseOverKeyOverlayColourId, theme::color::gold.withAlpha(0.12f));
    keyboardComponent.setColour(juce::MidiKeyboardComponent::textLabelColourId, theme::color::textSecondary.withAlpha(0.6f));
    keyboardComponent.setColour(juce::MidiKeyboardComponent::upDownButtonBackgroundColourId, theme::color::panelSurface);
    keyboardComponent.setColour(juce::MidiKeyboardComponent::upDownButtonArrowColourId, theme::color::gold);
    addAndMakeVisible(keyboardComponent);

    setSize(theme::dim::windowWidth, theme::dim::windowHeight);
    setWantsKeyboardFocus(true);
    startTimerHz(30);
    keyboardComponent.grabKeyboardFocus();
}

VeenaPluginEditor::~VeenaPluginEditor()
{
    stopTimer();
    setLookAndFeel(nullptr);
}

void VeenaPluginEditor::paint(juce::Graphics& g)
{
    g.fillAll(theme::color::background);

    // Header background
    auto headerArea = getLocalBounds().removeFromTop(theme::dim::headerHeight).toFloat();
    g.setColour(theme::color::panelSurface);
    g.fillRect(headerArea);
    g.setColour(theme::color::panelBorder);
    g.drawLine(headerArea.getX(), headerArea.getBottom(),
               headerArea.getRight(), headerArea.getBottom(), 1.0f);

    // Level meter in header (right side)
    float meterW = 80.0f;
    float meterH = 6.0f;
    float meterX = headerArea.getRight() - meterW - 15.0f;
    float meterY = headerArea.getCentreY() - meterH * 0.5f;

    g.setColour(theme::color::knobRing);
    g.fillRoundedRectangle(meterX, meterY, meterW, meterH, 3.0f);

    float meterFill = meterW * juce::jmin(currentPeak, 1.0f);
    juce::Colour meterCol = (currentPeak > 0.8f) ? theme::color::alert
                           : (currentPeak > 0.5f) ? theme::color::meterMid
                           : theme::color::safe;
    g.setColour(meterCol);
    g.fillRoundedRectangle(meterX, meterY, meterFill, meterH, 3.0f);
}

void VeenaPluginEditor::resized()
{
    auto area = getLocalBounds();
    int pad = theme::dim::padding;

    // --- Header ---
    auto header = area.removeFromTop(theme::dim::headerHeight);
    logoLabel->setBounds(header.removeFromLeft(130).reduced(pad, 2));

    // Preset selector on the right
    auto presetArea = header.removeFromRight(200).reduced(4, 6);
    presetPrev.setBounds(presetArea.removeFromLeft(24));
    presetNext.setBounds(presetArea.removeFromRight(24));
    presetCombo.setBounds(presetArea.reduced(2, 0));

    instrumentLabel->setBounds(header.reduced(10, 4));

    // --- Keyboard at bottom ---
    keyboardComponent.setBounds(area.removeFromBottom(theme::dim::keyboardHeight).reduced(pad, 2));

    // --- Visualization ---
    visualization.setBounds(area.removeFromTop(theme::dim::vizHeight).reduced(pad, 4));

    // --- Controls area (3 columns) ---
    auto controls = area.reduced(pad, 2);
    int colWidth = controls.getWidth() / 3;
    auto leftCol = controls.removeFromLeft(colWidth);
    auto midCol = controls.removeFromLeft(colWidth);
    auto rightCol = controls;

    int knobSize = theme::dim::knobSize;
    int knobRow = knobSize + 30;  // knob + label + value

    // --- Left column: STRING knobs ---
    stringGroupLabel->setBounds(leftCol.removeFromTop(18));
    leftCol.removeFromTop(4);

    auto pluckArea = leftCol.removeFromTop(knobRow);
    pluckPositionKnob.setBounds(pluckArea.removeFromTop(knobSize).withSizeKeepingCentre(knobSize, knobSize));
    pluckPosLabel->setBounds(pluckArea.removeFromTop(14));
    pluckPosValue->setBounds(pluckArea.removeFromTop(14));

    auto dampArea = leftCol.removeFromTop(knobRow);
    dampingKnob.setBounds(dampArea.removeFromTop(knobSize).withSizeKeepingCentre(knobSize, knobSize));
    dampingLabel->setBounds(dampArea.removeFromTop(14));
    dampingValue->setBounds(dampArea.removeFromTop(14));

    auto brightArea = leftCol.removeFromTop(knobRow);
    brightnessKnob.setBounds(brightArea.removeFromTop(knobSize).withSizeKeepingCentre(knobSize, knobSize));
    brightnessLabel->setBounds(brightArea.removeFromTop(14));
    brightnessValue->setBounds(brightArea.removeFromTop(14));

    // --- Middle column: EXPRESSION ---
    exprGroupLabel->setBounds(midCol.removeFromTop(18));
    midCol.removeFromTop(4);

    // Pitch bend strip, vibrato knob, expression strip — side by side
    auto exprRow = midCol.removeFromTop(theme::dim::stripHeight + 30);
    int stripW = theme::dim::stripWidth;
    int exprColW = exprRow.getWidth() / 3;

    auto bendCol = exprRow.removeFromLeft(exprColW);
    pitchBendStrip.setBounds(bendCol.removeFromTop(theme::dim::stripHeight).withSizeKeepingCentre(stripW, theme::dim::stripHeight));
    pitchBendLabel->setBounds(bendCol.removeFromTop(14));
    pitchBendValue->setBounds(bendCol.removeFromTop(14));

    auto vibCol = exprRow.removeFromLeft(exprColW);
    vibratoKnob.setBounds(vibCol.removeFromTop(knobSize).withSizeKeepingCentre(knobSize, knobSize));
    vibCol.removeFromTop(theme::dim::stripHeight - knobSize);  // align with strips
    vibratoLabel->setBounds(vibCol.removeFromTop(14));
    vibratoValue->setBounds(vibCol.removeFromTop(14));

    auto exprCol = exprRow;
    expressionStrip.setBounds(exprCol.removeFromTop(theme::dim::stripHeight).withSizeKeepingCentre(stripW, theme::dim::stripHeight));
    expressionLabel->setBounds(exprCol.removeFromTop(14));
    expressionValue->setBounds(exprCol.removeFromTop(14));

    // --- Right column: RESONANCE knobs + SETTINGS combos ---
    resonGroupLabel->setBounds(rightCol.removeFromTop(18));
    rightCol.removeFromTop(4);

    // 3 knobs in a row
    auto knobRowArea = rightCol.removeFromTop(knobRow);
    int rknobW = knobRowArea.getWidth() / 3;

    auto bodyCol = knobRowArea.removeFromLeft(rknobW);
    bodyMixKnob.setBounds(bodyCol.removeFromTop(knobSize).withSizeKeepingCentre(knobSize, knobSize));
    bodyMixLabel->setBounds(bodyCol.removeFromTop(14));
    bodyMixValue->setBounds(bodyCol.removeFromTop(14));

    auto sympCol = knobRowArea.removeFromLeft(rknobW);
    sympatheticKnob.setBounds(sympCol.removeFromTop(knobSize).withSizeKeepingCentre(knobSize, knobSize));
    sympatheticLabel->setBounds(sympCol.removeFromTop(14));
    sympatheticValue->setBounds(sympCol.removeFromTop(14));

    auto thalCol = knobRowArea;
    thalamKnob.setBounds(thalCol.removeFromTop(knobSize).withSizeKeepingCentre(knobSize, knobSize));
    thalamLabel->setBounds(thalCol.removeFromTop(14));
    thalamValue->setBounds(thalCol.removeFromTop(14));

    rightCol.removeFromTop(6);
    settingsGroupLabel->setBounds(rightCol.removeFromTop(16));
    rightCol.removeFromTop(2);

    // Settings combos: 2 per row
    int comboW = (rightCol.getWidth() - 8) / 2;
    int comboH = theme::dim::comboHeight;

    auto srow1 = rightCol.removeFromTop(comboH + 2);
    bodyModeCombo.setBounds(srow1.removeFromLeft(comboW));
    srow1.removeFromLeft(8);
    ragaCombo.setBounds(srow1.removeFromLeft(comboW));

    rightCol.removeFromTop(3);
    auto srow2 = rightCol.removeFromTop(comboH + 2);
    tuningCombo.setBounds(srow2.removeFromLeft(comboW));
    srow2.removeFromLeft(8);
    bendRangeCombo.setBounds(srow2.removeFromLeft(comboW));

    rightCol.removeFromTop(3);
    auto srow3 = rightCol.removeFromTop(comboH + 2);
    glideCurveCombo.setBounds(srow3.removeFromLeft(comboW));
    srow3.removeFromLeft(8);
    legatoToggle.setBounds(srow3);
}

// --- Slider callbacks ---

void VeenaPluginEditor::sliderValueChanged(juce::Slider* slider)
{
    float val = static_cast<float>(slider->getValue());

    if (slider == &pluckPositionKnob)
    {
        processorRef.uiPluckPosition.store(val, std::memory_order_relaxed);
        pluckPosValue->setText(juce::String(val, 2), juce::dontSendNotification);
    }
    else if (slider == &dampingKnob)
    {
        processorRef.uiDamping.store(val, std::memory_order_relaxed);
        dampingValue->setText(juce::String(val, 2), juce::dontSendNotification);
    }
    else if (slider == &brightnessKnob)
    {
        processorRef.uiBrightness.store(val, std::memory_order_relaxed);
        brightnessValue->setText(juce::String(val, 2), juce::dontSendNotification);
    }
    else if (slider == &pitchBendStrip)
    {
        processorRef.uiPitchBend.store(val, std::memory_order_relaxed);
        float st = val * processorRef.getBendRangeSemitones();
        pitchBendValue->setText((st >= 0 ? "+" : "") + juce::String(st, 1), juce::dontSendNotification);
    }
    else if (slider == &vibratoKnob)
    {
        processorRef.uiVibratoDepth.store(val, std::memory_order_relaxed);
        vibratoValue->setText(juce::String(val, 2) + "st", juce::dontSendNotification);
    }
    else if (slider == &expressionStrip)
    {
        processorRef.uiExpressionGain.store(val, std::memory_order_relaxed);
        expressionValue->setText(juce::String(static_cast<int>(val * 100)) + "%", juce::dontSendNotification);
    }
    else if (slider == &bodyMixKnob)
    {
        processorRef.uiBodyMix.store(val, std::memory_order_relaxed);
        bodyMixValue->setText(juce::String(val, 2), juce::dontSendNotification);
    }
    else if (slider == &sympatheticKnob)
    {
        processorRef.uiSympathetic.store(val, std::memory_order_relaxed);
        sympatheticValue->setText(juce::String(val, 2), juce::dontSendNotification);
    }
    else if (slider == &thalamKnob)
    {
        processorRef.uiThalamVolume.store(val, std::memory_order_relaxed);
        thalamValue->setText(juce::String(val, 2), juce::dontSendNotification);
    }
}

void VeenaPluginEditor::sliderDragEnded(juce::Slider* slider)
{
    if (slider == &pitchBendStrip)
    {
        pitchBendStrip.setValue(0.0, juce::dontSendNotification);
        processorRef.uiPitchBend.store(0.0f, std::memory_order_relaxed);
    }
}

// --- Combo box callbacks ---

void VeenaPluginEditor::comboBoxChanged(juce::ComboBox* combo)
{
    if (combo == &bodyModeCombo)
        processorRef.uiBodyMode.store(bodyModeCombo.getSelectedId() - 1, std::memory_order_relaxed);
    else if (combo == &ragaCombo)
        processorRef.uiRagaPreset.store(ragaCombo.getSelectedId() - 1, std::memory_order_relaxed);
    else if (combo == &tuningCombo)
    {
        int offset = tuningCombo.getSelectedId() - 1;
        processorRef.uiTuningOffset.store(offset, std::memory_order_relaxed);
        keyboardComponent.setAvailableRange(48 + offset, 48 + offset + 48);
    }
    else if (combo == &bendRangeCombo)
        processorRef.uiBendRange.store(static_cast<float>(bendRangeCombo.getSelectedId()), std::memory_order_relaxed);
    else if (combo == &glideCurveCombo)
        processorRef.uiGlideCurve.store(glideCurveCombo.getSelectedId() - 1, std::memory_order_relaxed);
}

// --- Key handling for thalam ---

bool VeenaPluginEditor::keyPressed(const juce::KeyPress& key)
{
    char c = static_cast<char>(key.getTextCharacter());
    int thalamIndex = -1;

    if (c == 'z' || c == 'Z') thalamIndex = 0;
    else if (c == 'x' || c == 'X') thalamIndex = 1;
    else if (c == 'c' || c == 'C') thalamIndex = 2;

    if (thalamIndex >= 0 && !thalamKeyState[thalamIndex])
    {
        thalamKeyState[thalamIndex] = true;
        processorRef.keyboardState.noteOn(1, 36 + thalamIndex * 2, 0.8f);
        visualization.setThalamFlash(thalamIndex, 1.0f);
        return true;
    }
    return false;
}

bool VeenaPluginEditor::keyStateChanged(bool /*isKeyDown*/)
{
    const char keys[] = { 'Z', 'X', 'C' };
    const int notes[] = { 36, 38, 40 };
    for (int i = 0; i < 3; ++i)
    {
        if (thalamKeyState[i] && !juce::KeyPress::isKeyCurrentlyDown(keys[i]))
        {
            thalamKeyState[i] = false;
            processorRef.keyboardState.noteOff(1, notes[i], 0.0f);
        }
    }
    return false;
}

// --- Timer: update visualization and level meter ---

void VeenaPluginEditor::timerCallback()
{
    float peak = processorRef.peakLevel.load(std::memory_order_relaxed);
    currentPeak = std::max(peak, currentPeak * 0.85f);

    // Update visualization state from processor.
    visualization.setPeakLevel(currentPeak);
    visualization.setStringAmplitude(0, currentPeak);
    visualization.setStringAmplitude(1, currentPeak * 0.5f);

    // Update pitch bend label (for spring-back visual sync).
    float bendVal = static_cast<float>(pitchBendStrip.getValue());
    float bendSt = bendVal * processorRef.getBendRangeSemitones();
    pitchBendValue->setText((bendSt >= 0 ? "+" : "") + juce::String(bendSt, 1), juce::dontSendNotification);

    repaint(getLocalBounds().removeFromTop(theme::dim::headerHeight));
}

// --- Label factory ---

juce::Label* VeenaPluginEditor::makeLabel(const juce::String& text, float fontSize,
                                           juce::Colour col, juce::Justification just)
{
    auto* label = new juce::Label();
    label->setText(text, juce::dontSendNotification);
    label->setFont(juce::FontOptions(fontSize));
    label->setColour(juce::Label::textColourId, col);
    label->setJustificationType(just);
    ownedLabels.add(label);
    return label;
}
