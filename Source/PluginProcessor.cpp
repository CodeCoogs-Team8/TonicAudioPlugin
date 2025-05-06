/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
DelayAudioProcessor::DelayAudioProcessor()
    : AudioProcessor(BusesProperties()
                         .withInput("Input", juce::AudioChannelSet::stereo(), true)
                         .withOutput("Output", juce::AudioChannelSet::stereo(), true))
{
}

DelayAudioProcessor::~DelayAudioProcessor()
{
    // First, set isPrepared to false to prevent any new processing
    isPrepared = false;

    // Then release resources with proper locking
    const juce::ScopedLock sl(effectRackLock);
    effectRack.releaseResources();

    // Clear all effects before destruction
    effectRack.clearEffects();
}

//==============================================================================
const juce::String DelayAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool DelayAudioProcessor::acceptsMidi() const
{
    return false;
}

bool DelayAudioProcessor::producesMidi() const
{
    return false;
}

bool DelayAudioProcessor::isMidiEffect() const
{
    return false;
}

double DelayAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int DelayAudioProcessor::getNumPrograms()
{
    return 1;
}

int DelayAudioProcessor::getCurrentProgram()
{
    return 0;
}

void DelayAudioProcessor::setCurrentProgram(int) {}

const juce::String DelayAudioProcessor::getProgramName(int) { return {}; }

void DelayAudioProcessor::changeProgramName(int, const juce::String &) {}

//==============================================================================
void DelayAudioProcessor::prepareToPlay(double sampleRate, int samplesPerBlock)
{
    // Prepare the effect rack first
    {
        const juce::ScopedLock sl(effectRackLock);
        effectRack.prepareToPlay(sampleRate, samplesPerBlock);
    }

    isPrepared = true;
}

void DelayAudioProcessor::releaseResources()
{
    isPrepared = false;
    const juce::ScopedLock sl(effectRackLock);
    effectRack.releaseResources();
}

bool DelayAudioProcessor::isBusesLayoutSupported(const BusesLayout &layouts) const
{
    // Only supports stereo
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;

    // Input layout must match output layout
    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
        return false;

    return true;
}

void DelayAudioProcessor::processBlock(juce::AudioBuffer<float> &buffer, juce::MidiBuffer &midiMessages)
{
    if (!isPrepared)
        return;

    juce::ScopedNoDenormals noDenormals;

    // Clear any unused output channels
    for (auto i = getTotalNumInputChannels(); i < getTotalNumOutputChannels(); ++i)
        buffer.clear(i, 0, buffer.getNumSamples());

    // Process the effect rack
    {
        const juce::ScopedLock sl(effectRackLock);
        effectRack.processBlock(buffer, midiMessages);
    }

    // Update output levels
    AudioLevels levels;

    if (buffer.getNumChannels() >= 1)
    {
        const float *leftData = buffer.getReadPointer(0);
        float sumSquared = 0.0f;
        for (int sample = 0; sample < buffer.getNumSamples(); ++sample)
        {
            sumSquared += leftData[sample] * leftData[sample];
        }
        float rmsLevel = std::sqrt(sumSquared / buffer.getNumSamples());
        levels.leftLevel = juce::jlimit(0.0f, 1.0f,
                                        (juce::Decibels::gainToDecibels(rmsLevel) + 60.0f) / 60.0f);
    }

    if (buffer.getNumChannels() >= 2)
    {
        const float *rightData = buffer.getReadPointer(1);
        float sumSquared = 0.0f;
        for (int sample = 0; sample < buffer.getNumSamples(); ++sample)
        {
            sumSquared += rightData[sample] * rightData[sample];
        }
        float rmsLevel = std::sqrt(sumSquared / buffer.getNumSamples());
        levels.rightLevel = juce::jlimit(0.0f, 1.0f,
                                         (juce::Decibels::gainToDecibels(rmsLevel) + 60.0f) / 60.0f);
    }

    // Update levels with thread safety
    {
        const juce::ScopedLock sl(levelsLock);
        currentLevels = levels;
    }
}

//==============================================================================
bool DelayAudioProcessor::hasEditor() const
{
    return true;
}

juce::AudioProcessorEditor *DelayAudioProcessor::createEditor()
{
    return new DelayAudioProcessorEditor(*this);
}

//==============================================================================
void DelayAudioProcessor::getStateInformation(juce::MemoryBlock &destData)
{
    const juce::ScopedLock sl(effectRackLock);
    effectRack.getStateInformation(destData);
}

void DelayAudioProcessor::setStateInformation(const void *data, int sizeInBytes)
{
    const juce::ScopedLock sl(effectRackLock);
    effectRack.setStateInformation(data, sizeInBytes);
}

//==============================================================================
juce::AudioProcessor *JUCE_CALLTYPE createPluginFilter()
{
    return new DelayAudioProcessor();
}
