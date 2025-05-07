/*
  ==============================================================================

    GainProcessor.cpp
    Created: 3 Apr 2025 2:19:00pm
    Author:  Evan Fraustro

  ==============================================================================
*/

#include "GainProcessor.h"

GainProcessor::GainProcessor(bool isInput)
    : AudioProcessor(BusesProperties()
                         .withInput("Input", juce::AudioChannelSet::stereo(), true)
                         .withOutput("Output", juce::AudioChannelSet::stereo(), true)),
      parameters(*this, nullptr, "Parameters",
                 {std::make_unique<juce::AudioParameterFloat>(
                     juce::ParameterID(isInput ? "input_gain" : "output_gain", 1), // ParameterID with version
                     isInput ? "Input Gain" : "Output Gain",                       // Parameter name
                     juce::NormalisableRange<float>(minGain, maxGain, 0.1f),
                     defaultGain,
                     juce::String(), // Label
                     juce::AudioProcessorParameter::genericParameter,
                     [](float value, int)
                     { return juce::String(value, 1) + " dB"; })})
{
    gainParam = parameters.getRawParameterValue(isInput ? "input_gain" : "output_gain");
}

GainProcessor::~GainProcessor()
{
    gainParam = nullptr;
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
    currentSampleRate = sampleRate;
}

void GainProcessor::releaseResources()
{
    currentSampleRate = 0.0;
}

bool GainProcessor::isBusesLayoutSupported(const BusesLayout &layouts) const
{
    // Only supports stereo
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;

    // Input layout must match output layout
    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
        return false;

    return true;
}

void GainProcessor::processBlock(juce::AudioBuffer<float> &buffer, juce::MidiBuffer &)
{
    juce::ScopedNoDenormals noDenormals;

    // Check if we have a valid sample rate
    if (!isSampleRateValid())
    {
        buffer.clear();
        return;
    }

    // Get the current gain value in dB
    float gainDB = gainParam != nullptr ? gainParam->load() : defaultGain;

    // Convert dB to linear gain
    float linearGain = juce::Decibels::decibelsToGain(gainDB);

    // Apply gain to all channels
    for (int channel = 0; channel < buffer.getNumChannels(); ++channel)
    {
        float *channelData = buffer.getWritePointer(channel);
        for (int sample = 0; sample < buffer.getNumSamples(); ++sample)
        {
            channelData[sample] *= linearGain;
        }
    }
}

bool GainProcessor::hasEditor() const
{
    return true;
}

juce::AudioProcessorEditor *GainProcessor::createEditor()
{
    return new juce::GenericAudioProcessorEditor(*this);
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
