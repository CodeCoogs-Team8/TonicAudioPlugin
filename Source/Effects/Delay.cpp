/*
  ==============================================================================

    Delay.cpp
    Created: 15 Apr 2025 7:21:01pm
    Author:  Evan Fraustro

  ==============================================================================
*/

#include "Delay.h"

Delay::Delay()
    : AudioProcessor(BusesProperties()
                         .withInput("Input", juce::AudioChannelSet::stereo(), true)
                         .withOutput("Output", juce::AudioChannelSet::stereo(), true)),
      parameters(*this, nullptr, "PARAMETERS",
                 {std::make_unique<juce::AudioParameterFloat>(
                      "delayTime",  // parameterID
                      "Delay Time", // parameter name
                      0.0f,         // minimum value
                      2.0f,         // maximum value
                      0.5f          // default value
                      ),
                  std::make_unique<juce::AudioParameterFloat>(
                      "feedback",        // parameterID
                      "Feedback Amount", // parameter name
                      0.0f,              // minimum value
                      0.95f,             // maximum value
                      0.4f               // default value
                      ),
                  std::make_unique<juce::AudioParameterFloat>(
                      "mix",         // parameterID
                      "Dry/Wet Mix", // parameter name
                      0.0f,          // minimum value
                      1.0f,          // maximum value
                      0.5f           // default value
                      )})
{
  delayTimeParam = parameters.getRawParameterValue("delayTime");
  feedbackParam = parameters.getRawParameterValue("feedback");
  mixParam = parameters.getRawParameterValue("mix");
}

Delay::~Delay()
{
  releaseResources();
  delayTimeParam = nullptr;
  feedbackParam = nullptr;
  mixParam = nullptr;
}

void Delay::prepareToPlay(double sampleRate, int samplesPerBlock)
{
  currentSampleRate = sampleRate;
  delayLine.prepare({sampleRate, static_cast<juce::uint32>(samplesPerBlock), 2});
  delayLine.setMaximumDelayInSamples(static_cast<int>(sampleRate * 2.0)); // Max 2 seconds delay
}

void Delay::releaseResources()
{
  // Clear the delay line
  delayLine.reset();
}

void Delay::processBlock(juce::AudioBuffer<float> &buffer, juce::MidiBuffer &midiMessages)
{
  juce::ScopedNoDenormals noDenormals;

  // Check if we have a valid sample rate
  if (!isSampleRateValid())
  {
    buffer.clear();
    return;
  }

  // Safely get parameter values at the start of the block
  const float delayTime = delayTimeParam != nullptr ? delayTimeParam->load() : 0.5f;
  const float feedback = feedbackParam != nullptr ? feedbackParam->load() : 0.4f;
  const float mix = mixParam != nullptr ? mixParam->load() : 0.5f;

  const int numChannels = buffer.getNumChannels();
  const int numSamples = buffer.getNumSamples();

  // Update delay time
  const int delaySamples = static_cast<int>(delayTime * currentSampleRate);
  delayLine.setDelay(delaySamples);

  // Process each channel
  for (int channel = 0; channel < numChannels; ++channel)
  {
    auto *channelData = buffer.getWritePointer(channel);

    for (int sample = 0; sample < numSamples; ++sample)
    {
      const float in = channelData[sample];
      float delayedSample = delayLine.popSample(channel);

      // Push the input + feedback to the delay line
      delayLine.pushSample(channel, in + (delayedSample * feedback));

      // Mix the dry and wet signals
      channelData[sample] = in * (1.0f - mix) + delayedSample * mix;
    }
  }
}

juce::AudioProcessorEditor *Delay::createEditor()
{
  return new juce::GenericAudioProcessorEditor(*this);
}

void Delay::getStateInformation(juce::MemoryBlock &destData)
{
  auto state = parameters.copyState();
  std::unique_ptr<juce::XmlElement> xml(state.createXml());
  copyXmlToBinary(*xml, destData);
}

void Delay::setStateInformation(const void *data, int sizeInBytes)
{
  try
  {
    std::unique_ptr<juce::XmlElement> xmlState(getXmlFromBinary(data, sizeInBytes));

    if (xmlState != nullptr && xmlState->hasTagName(parameters.state.getType()))
    {
      // Store the current delay line state
      std::vector<float> delayLineState;
      for (int ch = 0; ch < 2; ++ch)
      {
        // Store delay line samples if needed
      }

      // Update parameters
      parameters.replaceState(juce::ValueTree::fromXml(*xmlState));

      // Reset DSP state if sample rate has changed
      if (currentSampleRate > 0.0)
      {
        delayLine.prepare({currentSampleRate,
                           static_cast<juce::uint32>(getBlockSize()),
                           static_cast<juce::uint32>(getNumInputChannels())});
        delayLine.setMaximumDelayInSamples(static_cast<int>(currentSampleRate * 2.0));
      }
    }
  }
  catch (...)
  {
    // If state restoration fails, ensure we're in a valid state
    if (currentSampleRate > 0.0)
    {
      delayLine.prepare({currentSampleRate,
                         static_cast<juce::uint32>(getBlockSize()),
                         static_cast<juce::uint32>(getNumInputChannels())});
      delayLine.setMaximumDelayInSamples(static_cast<int>(currentSampleRate * 2.0));
    }
  }
}
