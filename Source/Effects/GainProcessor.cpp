/*
  ==============================================================================

    GainProcessor.cpp
    Created: 3 Apr 2025 2:19:00pm
    Author:  Evan Fraustro

  ==============================================================================
*/

#include "GainProcessor.h"

GainProcessor::GainProcessor()
    : AudioProcessor(BusesProperties()
                         .withInput("Input", juce::AudioChannelSet::stereo(), true)
                         .withOutput("Output", juce::AudioChannelSet::stereo(), true)),
      parameters(*this, nullptr, "Parameters", {std::make_unique<juce::AudioParameterFloat>("gain", // ParameterID
                                                                                            "Gain", // Parameter name
                                                                                            juce::NormalisableRange<float>(-48.0f, 0.0f, 0.1f), 0.0f)})

{
    gainParam = parameters.getRawParameterValue("gain");
}

GainProcessor::~GainProcessor()
{
}

//=============================================
const juce::String GainProcessor::getName() const
{
    return "Gain";
}

bool GainProcessor::acceptsMidi() const
{
    return false;
}

bool GainProcessor::producesMidi() const
{
    return false;
}

bool GainProcessor::isMidiEffect() const
{
    return false;
}

double GainProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int GainProcessor::getNumPrograms()
{
    return 1;
}

int GainProcessor::getCurrentProgram()
{
    return 0;
}

void GainProcessor::setCurrentProgram(int index)
{
}

const juce::String GainProcessor::getProgramName(int index)
{
    return {};
}

void GainProcessor::changeProgramName(int index, const juce::String &newName)
{
}

void GainProcessor::prepareToPlay(double sampleRate, int samplesPerBlock)
{
}

juce::AudioProcessorEditor *GainProcessor::createEditor()
{
    return new juce::GenericAudioProcessorEditor(*this);
}

void GainProcessor::releaseResources()
{
}

bool GainProcessor::isBusesLayoutSupported(const BusesLayout &layouts) const
{
    // Simplest stereo support
    return layouts.getMainOutputChannelSet() == juce::AudioChannelSet::stereo() && layouts.getMainInputChannelSet() == juce::AudioChannelSet::stereo();
}

void GainProcessor::processBlock(juce::AudioBuffer<float> &buffer, juce::MidiBuffer &)
{
    juce::ScopedNoDenormals noDenormals;

    float linearGain = 1.0f;

    if (gainParam != nullptr)
    {
        linearGain = std::pow(10.0f, gainParam->load() / 20.0f);
    }

    for (int channel = 0; channel < buffer.getNumChannels(); ++channel)
    {
        buffer.applyGain(channel, 0, buffer.getNumSamples(), linearGain);
    }
}

bool GainProcessor::hasEditor() const
{
    return true;
}

void GainProcessor::getStateInformation(juce::MemoryBlock &destData)
{
    auto state = parameters.copyState();
    std::unique_ptr<juce::XmlElement> xml(state.createXml());
    copyXmlToBinary(*xml, destData);
}

void GainProcessor::setStateInformation(const void *data, int sizeInBytes)
{
    std::unique_ptr<juce::XmlElement> xmlState(getXmlFromBinary(data, sizeInBytes));

    if (xmlState.get() != nullptr)
        if (xmlState->hasTagName(parameters.state.getType()))
            parameters.replaceState(juce::ValueTree::fromXml(*xmlState));
}
