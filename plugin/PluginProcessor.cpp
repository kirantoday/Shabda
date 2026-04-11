#include "PluginProcessor.h"
#include "PluginEditor.h"
#include <algorithm>

VeenaPluginProcessor::VeenaPluginProcessor()
    : AudioProcessor(BusesProperties()
                         .withOutput("Output", juce::AudioChannelSet::stereo(), true))
{
}

VeenaPluginProcessor::~VeenaPluginProcessor() = default;

const juce::String VeenaPluginProcessor::getName() const
{
    return JucePlugin_Name;
}

bool VeenaPluginProcessor::acceptsMidi() const  { return true; }
bool VeenaPluginProcessor::producesMidi() const { return false; }
bool VeenaPluginProcessor::isMidiEffect() const { return false; }
double VeenaPluginProcessor::getTailLengthSeconds() const { return 0.0; }

int VeenaPluginProcessor::getNumPrograms()    { return 1; }
int VeenaPluginProcessor::getCurrentProgram() { return 0; }
void VeenaPluginProcessor::setCurrentProgram(int) {}
const juce::String VeenaPluginProcessor::getProgramName(int) { return {}; }
void VeenaPluginProcessor::changeProgramName(int, const juce::String&) {}

void VeenaPluginProcessor::prepareToPlay(double sampleRate, int samplesPerBlock)
{
    veenaVoice.prepare(static_cast<float>(sampleRate), samplesPerBlock);
}

void VeenaPluginProcessor::releaseResources()
{
}

bool VeenaPluginProcessor::isBusesLayoutSupported(const BusesLayout& layouts) const
{
    // Support mono or stereo output only
    const auto& mainOutput = layouts.getMainOutputChannelSet();
    if (mainOutput != juce::AudioChannelSet::mono()
        && mainOutput != juce::AudioChannelSet::stereo())
        return false;

    return true;
}

void VeenaPluginProcessor::processBlock(juce::AudioBuffer<float>& buffer,
                                         juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;

    // Clear the buffer before rendering.
    buffer.clear();

    int numSamples = buffer.getNumSamples();

    // Inject MIDI events from the on-screen keyboard / QWERTY input.
    // This merges virtual keyboard events into the incoming MIDI buffer.
    keyboardState.processNextMidiBuffer(midiMessages, 0, numSamples, true);

    // --- Process MIDI events ---
    // For Step 2, we process all MIDI events at the start of the block
    // (not sample-accurate). This is fine for note triggers.
    // EXTENSION: sample-accurate MIDI splitting for Step 3 pitch bend.
    for (const auto metadata : midiMessages)
    {
        auto msg = metadata.getMessage();

        if (msg.isNoteOn())
            veenaVoice.noteOn(msg.getNoteNumber(), msg.getFloatVelocity());
        else if (msg.isNoteOff())
            veenaVoice.noteOff(msg.getNoteNumber());
        else if (msg.isPitchWheel())
            veenaVoice.pitchBendMidi(msg.getPitchWheelValue());
        else if (msg.isController())
            veenaVoice.handleCC(msg.getControllerNumber(), msg.getControllerValue());
        else if (msg.isChannelPressure())
            veenaVoice.handleAftertouch(msg.getChannelPressureValue());
    }

    // --- Apply UI controls (written by editor, read on audio thread) ---

    // Expression controls
    veenaVoice.pitchBend(uiPitchBend.load(std::memory_order_relaxed) * veenaVoice.getBendRange());
    veenaVoice.setVibratoDepth(uiVibratoDepth.load(std::memory_order_relaxed));
    veenaVoice.setExpressionGain(uiExpressionGain.load(std::memory_order_relaxed));

    // String controls
    veenaVoice.setPluckPosition(uiPluckPosition.load(std::memory_order_relaxed));
    veenaVoice.setDamping(uiDamping.load(std::memory_order_relaxed));
    veenaVoice.setBrightness(uiBrightness.load(std::memory_order_relaxed));

    // Body & resonance
    veenaVoice.setBodyMix(uiBodyMix.load(std::memory_order_relaxed));
    veenaVoice.setSympatheticGain(uiSympathetic.load(std::memory_order_relaxed));

    // Settings
    veenaVoice.setBendRange(uiBendRange.load(std::memory_order_relaxed));
    veenaVoice.setLegatoEnabled(uiLegatoEnabled.load(std::memory_order_relaxed));
    veenaVoice.setGlideCurve(static_cast<engine::GlideCurve>(uiGlideCurve.load(std::memory_order_relaxed)));

    // --- Render audio ---
    // VeenaVoice renders mono into the left channel.
    auto* outputL = buffer.getWritePointer(0);

    veenaVoice.processBlock(outputL, numSamples);

    // Update peak level for the UI meter.
    float peak = 0.0f;
    for (int i = 0; i < numSamples; ++i)
        peak = std::max(peak, std::abs(outputL[i]));
    peakLevel.store(peak, std::memory_order_relaxed);

    // Copy mono to right channel for stereo output.
    if (buffer.getNumChannels() > 1)
    {
        auto* outputR = buffer.getWritePointer(1);
        std::copy(outputL, outputL + numSamples, outputR);
    }
}

juce::AudioProcessorEditor* VeenaPluginProcessor::createEditor()
{
    return new VeenaPluginEditor(*this);
}

bool VeenaPluginProcessor::hasEditor() const { return true; }

void VeenaPluginProcessor::getStateInformation(juce::MemoryBlock& /*destData*/)
{
    // Future: serialize plugin parameters
}

void VeenaPluginProcessor::setStateInformation(const void* /*data*/, int /*sizeInBytes*/)
{
    // Future: restore plugin parameters
}

float VeenaPluginProcessor::getBendRangeSemitones() const
{
    return veenaVoice.getBendRange();
}

// This creates the factory function that JUCE needs to instantiate the plugin
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new VeenaPluginProcessor();
}
