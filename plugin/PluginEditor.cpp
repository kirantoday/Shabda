#include "PluginEditor.h"
#include "PluginProcessor.h"

namespace {
    void setupGroupLabel(juce::Label& label, const juce::String& text)
    {
        label.setText(text, juce::dontSendNotification);
        label.setFont(juce::FontOptions(14.0f, juce::Font::bold));
        label.setJustificationType(juce::Justification::centredLeft);
        label.setColour(juce::Label::textColourId, juce::Colour(0xffcccccc));
    }

    void setupValueLabel(juce::Label& label, const juce::String& text)
    {
        label.setText(text, juce::dontSendNotification);
        label.setFont(juce::FontOptions(12.0f));
        label.setJustificationType(juce::Justification::centredLeft);
        label.setColour(juce::Label::textColourId, juce::Colour(0xff999999));
    }

    void setupSlider(juce::Slider& slider, double min, double max, double defaultVal,
                     double step = 0.01)
    {
        slider.setSliderStyle(juce::Slider::LinearHorizontal);
        slider.setRange(min, max, step);
        slider.setValue(defaultVal, juce::dontSendNotification);
        slider.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
    }
}

VeenaPluginEditor::VeenaPluginEditor(VeenaPluginProcessor& p)
    : AudioProcessorEditor(&p),
      processorRef(p),
      keyboardComponent(p.keyboardState, juce::MidiKeyboardComponent::horizontalKeyboard)
{
    // Title
    titleLabel.setText("Veena", juce::dontSendNotification);
    titleLabel.setFont(juce::FontOptions(26.0f, juce::Font::bold));
    titleLabel.setJustificationType(juce::Justification::centred);
    titleLabel.setColour(juce::Label::textColourId, juce::Colours::white);
    addAndMakeVisible(titleLabel);

    // --- String controls ---
    setupGroupLabel(stringGroupLabel, "String");
    addAndMakeVisible(stringGroupLabel);

    setupSlider(pluckPositionSlider, 0.05, 0.5, 0.15);
    pluckPositionSlider.addListener(this);
    addAndMakeVisible(pluckPositionSlider);
    setupValueLabel(pluckPositionLabel, "Pluck Pos: 0.15");
    addAndMakeVisible(pluckPositionLabel);

    setupSlider(dampingSlider, 0.0, 0.95, 0.4);
    dampingSlider.addListener(this);
    addAndMakeVisible(dampingSlider);
    setupValueLabel(dampingLabel, "Damping: 0.40");
    addAndMakeVisible(dampingLabel);

    setupSlider(brightnessSlider, 0.0, 1.0, 0.65);
    brightnessSlider.addListener(this);
    addAndMakeVisible(brightnessSlider);
    setupValueLabel(brightnessLabel, "Brightness: 0.65");
    addAndMakeVisible(brightnessLabel);

    // --- Body & resonance ---
    setupGroupLabel(bodyGroupLabel, "Body & Resonance");
    addAndMakeVisible(bodyGroupLabel);

    setupSlider(bodyMixSlider, 0.0, 1.0, 0.5);
    bodyMixSlider.addListener(this);
    addAndMakeVisible(bodyMixSlider);
    setupValueLabel(bodyMixLabel, "Body Mix: 0.50");
    addAndMakeVisible(bodyMixLabel);

    setupSlider(sympatheticSlider, 0.0, 0.5, 0.15);
    sympatheticSlider.addListener(this);
    addAndMakeVisible(sympatheticSlider);
    setupValueLabel(sympatheticLabel, "Sympathetic: 0.15");
    addAndMakeVisible(sympatheticLabel);

    // --- Expression controls ---
    setupGroupLabel(exprGroupLabel, "Expression");
    addAndMakeVisible(exprGroupLabel);

    setupSlider(pitchBendSlider, -1.0, 1.0, 0.0, 0.001);
    pitchBendSlider.setDoubleClickReturnValue(true, 0.0);
    pitchBendSlider.addListener(this);
    addAndMakeVisible(pitchBendSlider);
    setupValueLabel(pitchBendLabel, "Pitch Bend: 0.0 st");
    addAndMakeVisible(pitchBendLabel);

    setupSlider(vibratoSlider, 0.0, 1.0, 0.0);
    vibratoSlider.addListener(this);
    addAndMakeVisible(vibratoSlider);
    setupValueLabel(vibratoLabel, "Vibrato: 0.00 st");
    addAndMakeVisible(vibratoLabel);

    setupSlider(expressionSlider, 0.0, 1.0, 1.0);
    expressionSlider.addListener(this);
    addAndMakeVisible(expressionSlider);
    setupValueLabel(expressionLabel, "Expression: 100%");
    addAndMakeVisible(expressionLabel);

    // --- Settings ---
    setupGroupLabel(settingsGroupLabel, "Settings");
    addAndMakeVisible(settingsGroupLabel);

    setupValueLabel(bendRangeLabel, "Bend Range:");
    addAndMakeVisible(bendRangeLabel);
    for (int i = 1; i <= 12; ++i)
        bendRangeCombo.addItem(juce::String(i) + " st", i);
    bendRangeCombo.setSelectedId(7, juce::dontSendNotification);
    bendRangeCombo.addListener(this);
    addAndMakeVisible(bendRangeCombo);

    setupValueLabel(tuningLabel, "Tuning (Sa):");
    addAndMakeVisible(tuningLabel);
    const char* noteNames[] = { "C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B" };
    for (int i = 0; i < 12; ++i)
        tuningCombo.addItem(juce::String("Sa = ") + noteNames[i] + "3", i + 1);
    tuningCombo.setSelectedId(1, juce::dontSendNotification);  // C3 default
    tuningCombo.addListener(this);
    addAndMakeVisible(tuningCombo);

    // --- Level meter label ---
    setupValueLabel(levelLabel, "Level: ----");
    addAndMakeVisible(levelLabel);

    // --- Keyboard ---
    keyboardComponent.setKeyPressBaseOctave(5);
    keyboardComponent.setAvailableRange(36, 96);
    addAndMakeVisible(keyboardComponent);

    setSize(780, 530);

    // Start timer for level meter updates (30 fps).
    startTimerHz(30);

    keyboardComponent.grabKeyboardFocus();
}

VeenaPluginEditor::~VeenaPluginEditor()
{
    stopTimer();
}

void VeenaPluginEditor::paint(juce::Graphics& g)
{
    g.fillAll(juce::Colour(0xff1a1a2e));

    // Draw level meter bar
    auto meterArea = levelLabel.getBounds().translated(0, 18).withHeight(8).reduced(2, 0);
    g.setColour(juce::Colour(0xff333355));
    g.fillRect(meterArea);

    float meterWidth = static_cast<float>(meterArea.getWidth()) * std::min(currentPeak, 1.0f);
    if (currentPeak > 0.8f)
        g.setColour(juce::Colour(0xffff6644));
    else if (currentPeak > 0.5f)
        g.setColour(juce::Colour(0xffffcc44));
    else
        g.setColour(juce::Colour(0xff44cc66));
    g.fillRect(meterArea.withWidth(static_cast<int>(meterWidth)));
}

void VeenaPluginEditor::resized()
{
    auto area = getLocalBounds();

    titleLabel.setBounds(area.removeFromTop(35));

    // Keyboard at bottom
    auto keyboardArea = area.removeFromBottom(100).reduced(8, 4);
    keyboardComponent.setBounds(keyboardArea);

    // Two columns for controls
    auto controlArea = area.reduced(12, 4);
    int colWidth = controlArea.getWidth() / 2;
    auto leftCol = controlArea.removeFromLeft(colWidth).reduced(4, 0);
    auto rightCol = controlArea.reduced(4, 0);

    int rowH = 30;  // slider row height
    int groupH = 22; // group label height

    // --- Left column: String + Body ---

    stringGroupLabel.setBounds(leftCol.removeFromTop(groupH));

    pluckPositionLabel.setBounds(leftCol.removeFromTop(16));
    pluckPositionSlider.setBounds(leftCol.removeFromTop(rowH - 10).reduced(0, 2));
    leftCol.removeFromTop(2);

    dampingLabel.setBounds(leftCol.removeFromTop(16));
    dampingSlider.setBounds(leftCol.removeFromTop(rowH - 10).reduced(0, 2));
    leftCol.removeFromTop(2);

    brightnessLabel.setBounds(leftCol.removeFromTop(16));
    brightnessSlider.setBounds(leftCol.removeFromTop(rowH - 10).reduced(0, 2));
    leftCol.removeFromTop(8);

    bodyGroupLabel.setBounds(leftCol.removeFromTop(groupH));

    bodyMixLabel.setBounds(leftCol.removeFromTop(16));
    bodyMixSlider.setBounds(leftCol.removeFromTop(rowH - 10).reduced(0, 2));
    leftCol.removeFromTop(2);

    sympatheticLabel.setBounds(leftCol.removeFromTop(16));
    sympatheticSlider.setBounds(leftCol.removeFromTop(rowH - 10).reduced(0, 2));

    // --- Right column: Expression + Settings ---

    exprGroupLabel.setBounds(rightCol.removeFromTop(groupH));

    pitchBendLabel.setBounds(rightCol.removeFromTop(16));
    pitchBendSlider.setBounds(rightCol.removeFromTop(rowH - 10).reduced(0, 2));
    rightCol.removeFromTop(2);

    vibratoLabel.setBounds(rightCol.removeFromTop(16));
    vibratoSlider.setBounds(rightCol.removeFromTop(rowH - 10).reduced(0, 2));
    rightCol.removeFromTop(2);

    expressionLabel.setBounds(rightCol.removeFromTop(16));
    expressionSlider.setBounds(rightCol.removeFromTop(rowH - 10).reduced(0, 2));
    rightCol.removeFromTop(8);

    settingsGroupLabel.setBounds(rightCol.removeFromTop(groupH));

    auto settingsRow = rightCol.removeFromTop(26);
    bendRangeLabel.setBounds(settingsRow.removeFromLeft(80));
    bendRangeCombo.setBounds(settingsRow.removeFromLeft(80));
    settingsRow.removeFromLeft(10);
    tuningLabel.setBounds(settingsRow.removeFromLeft(75));
    tuningCombo.setBounds(settingsRow.removeFromLeft(100));

    rightCol.removeFromTop(8);
    levelLabel.setBounds(rightCol.removeFromTop(16));
}

void VeenaPluginEditor::sliderValueChanged(juce::Slider* slider)
{
    float val = static_cast<float>(slider->getValue());

    if (slider == &pluckPositionSlider)
    {
        processorRef.uiPluckPosition.store(val, std::memory_order_relaxed);
        pluckPositionLabel.setText("Pluck Pos: " + juce::String(val, 2), juce::dontSendNotification);
    }
    else if (slider == &dampingSlider)
    {
        processorRef.uiDamping.store(val, std::memory_order_relaxed);
        dampingLabel.setText("Damping: " + juce::String(val, 2), juce::dontSendNotification);
    }
    else if (slider == &brightnessSlider)
    {
        processorRef.uiBrightness.store(val, std::memory_order_relaxed);
        brightnessLabel.setText("Brightness: " + juce::String(val, 2), juce::dontSendNotification);
    }
    else if (slider == &bodyMixSlider)
    {
        processorRef.uiBodyMix.store(val, std::memory_order_relaxed);
        bodyMixLabel.setText("Body Mix: " + juce::String(val, 2), juce::dontSendNotification);
    }
    else if (slider == &sympatheticSlider)
    {
        processorRef.uiSympathetic.store(val, std::memory_order_relaxed);
        sympatheticLabel.setText("Sympathetic: " + juce::String(val, 2), juce::dontSendNotification);
    }
    else if (slider == &pitchBendSlider)
    {
        processorRef.uiPitchBend.store(val, std::memory_order_relaxed);
        float semitones = val * processorRef.getBendRangeSemitones();
        juce::String text = "Pitch Bend: ";
        if (semitones >= 0.0f) text += "+";
        text += juce::String(semitones, 1) + " st";
        pitchBendLabel.setText(text, juce::dontSendNotification);
    }
    else if (slider == &vibratoSlider)
    {
        processorRef.uiVibratoDepth.store(val, std::memory_order_relaxed);
        vibratoLabel.setText("Vibrato: " + juce::String(val, 2) + " st", juce::dontSendNotification);
    }
    else if (slider == &expressionSlider)
    {
        processorRef.uiExpressionGain.store(val, std::memory_order_relaxed);
        expressionLabel.setText("Expression: " + juce::String(static_cast<int>(val * 100)) + "%",
                                juce::dontSendNotification);
    }
}

void VeenaPluginEditor::sliderDragEnded(juce::Slider* slider)
{
    if (slider == &pitchBendSlider)
        pitchBendSlider.setValue(0.0, juce::sendNotificationSync);
}

void VeenaPluginEditor::comboBoxChanged(juce::ComboBox* combo)
{
    if (combo == &bendRangeCombo)
    {
        float range = static_cast<float>(bendRangeCombo.getSelectedId());
        processorRef.uiBendRange.store(range, std::memory_order_relaxed);
    }
    else if (combo == &tuningCombo)
    {
        int offset = tuningCombo.getSelectedId() - 1;  // 0=C, 1=C#, 2=D, ...
        processorRef.uiTuningOffset.store(offset, std::memory_order_relaxed);
        // Shift the keyboard display to start from the selected Sa note.
        int baseNote = 48 + offset;  // C3 + offset
        keyboardComponent.setAvailableRange(baseNote, baseNote + 48);
    }
}

void VeenaPluginEditor::timerCallback()
{
    // Update level meter with peak from audio thread.
    float peak = processorRef.peakLevel.load(std::memory_order_relaxed);
    // Smooth decay for visual appeal.
    currentPeak = std::max(peak, currentPeak * 0.85f);

    int db = (currentPeak > 0.0001f)
        ? static_cast<int>(20.0f * std::log10(currentPeak))
        : -60;
    levelLabel.setText("Level: " + juce::String(db) + " dB", juce::dontSendNotification);
    repaint();  // Trigger meter bar redraw
}
