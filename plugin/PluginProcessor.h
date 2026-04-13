#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_audio_utils/juce_audio_utils.h>
#include "instruments/veena/VeenaVoice.h"
#include <atomic>

class VeenaPluginProcessor : public juce::AudioProcessor
{
public:
    VeenaPluginProcessor();
    ~VeenaPluginProcessor() override;

    void prepareToPlay(double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

    bool isBusesLayoutSupported(const BusesLayout& layouts) const override;

    void processBlock(juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    const juce::String getName() const override;

    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;

    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram(int index) override;
    const juce::String getProgramName(int index) override;
    void changeProgramName(int index, const juce::String& newName) override;

    void getStateInformation(juce::MemoryBlock& destData) override;
    void setStateInformation(const void* data, int sizeInBytes) override;

    // --- Shared state (editor ↔ audio thread) ---

    juce::MidiKeyboardState keyboardState;

    // Expression controls (from Step 6 sliders)
    std::atomic<float> uiPitchBend { 0.0f };       // -1..+1 normalized
    std::atomic<float> uiVibratoDepth { 0.0f };     // 0..1 semitones
    std::atomic<float> uiExpressionGain { 1.0f };   // 0..1

    // String controls
    std::atomic<float> uiPluckPosition { 0.15f };   // 0.05..0.5
    std::atomic<float> uiDamping { 0.4f };           // 0..0.95
    std::atomic<float> uiBrightness { 0.65f };       // 0..1

    // Body & resonance
    std::atomic<float> uiBodyMix { 0.5f };           // 0..1
    std::atomic<float> uiSympathetic { 0.15f };      // 0..0.5
    std::atomic<float> uiThalamVolume { 0.6f };      // 0..1

    // Settings
    std::atomic<float> uiBendRange { 7.0f };         // 1..12 semitones
    std::atomic<int>   uiTuningOffset { 0 };         // semitones from C3 (0=C, 2=D, etc.)
    std::atomic<bool>  uiLegatoEnabled { true };     // legato glide on/off
    std::atomic<int>   uiGlideCurve { 1 };           // 0=Linear, 1=Exponential, 2=SCurve, 3=Late
    std::atomic<int>   uiRagaPreset { 0 };           // index into RAGA_PRESETS (0=Free)

    // Peak level for the meter (written by audio thread, read by editor)
    std::atomic<float> peakLevel { 0.0f };

    float getBendRangeSemitones() const;

private:
    veena::VeenaVoice veenaVoice;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(VeenaPluginProcessor)
};
