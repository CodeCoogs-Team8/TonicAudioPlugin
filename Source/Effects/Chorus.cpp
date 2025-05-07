/*
  ==============================================================================

    Chorus.cpp
    Created: 15 Apr 2025 7:21:18pm
    Author:  Evan Fraustro

  ==============================================================================
*/
#define _USE_MATH_DEFINES
#include <cmath>
#include "Chorus.h"

Chorus::Chorus()
    : AudioProcessor(BusesProperties()
                         .withInput("Input", juce::AudioChannelSet::stereo(), true)
                         .withOutput("Output", juce::AudioChannelSet::stereo(), true)),
      parameters(*this, nullptr, "PARAMETERS",
                 {std::make_unique<juce::AudioParameterFloat>(
                      "rate", "Rate", minRate, maxRate, defaultRate),
                  std::make_unique<juce::AudioParameterFloat>(
                      "depth", "Depth", minDepth, maxDepth, defaultDepth),
                  std::make_unique<juce::AudioParameterFloat>(
                      "delay", "Delay", minDelay, maxDelay, defaultDelay),
                  std::make_unique<juce::AudioParameterFloat>(
                      "mix", "Mix", minMix, maxMix, defaultMix),
                  std::make_unique<juce::AudioParameterFloat>(
                      "phase", "Phase", minPhase, maxPhase, defaultPhase)})
{
  rateParam = parameters.getRawParameterValue("rate");
  depthParam = parameters.getRawParameterValue("depth");
  delayParam = parameters.getRawParameterValue("delay");
  mixParam = parameters.getRawParameterValue("mix");
  phaseParam = parameters.getRawParameterValue("phase");
}

Chorus::~Chorus()
{
  releaseResources();
  rateParam = nullptr;
  depthParam = nullptr;
  delayParam = nullptr;
  mixParam = nullptr;
  phaseParam = nullptr;
}

void Chorus::prepareToPlay(double sampleRate, int samplesPerBlock)
{
  // Store sample rate and initialize processing
  currentSampleRate = sampleRate;

  // Configure delay line
  juce::dsp::ProcessSpec spec{sampleRate, static_cast<juce::uint32>(samplesPerBlock), 2};
  delayLine.prepare(spec);

  // Set maximum delay time (convert from ms to samples)
  const int maxDelaySamples = static_cast<int>(maxDelayTimeMs * sampleRate / 1000.0);
  delayLine.setMaximumDelayInSamples(maxDelaySamples);

  // Reset processing state
  reset();
  isInitialized = true;
}

void Chorus::releaseResources()
{
  reset();
  isInitialized = false;
}

void Chorus::reset()
{
  delayLine.reset();
  lfoPhase = 0.0f;
}

float Chorus::processChannel(float input, int channel, float delayTime)
{
  // Ensure delay time is within bounds
  const float boundedDelayTime = juce::jlimit(minDelay, maxDelay, delayTime);
  const float delayInSamples = boundedDelayTime * (currentSampleRate / 1000.0f);

  // Process through delay line
  delayLine.setDelay(delayInSamples);
  const float delayedSample = delayLine.popSample(channel);
  delayLine.pushSample(channel, input);

  return delayedSample;
}

void Chorus::processBlock(juce::AudioBuffer<float> &buffer, juce::MidiBuffer &midiMessages)
{
  juce::ScopedNoDenormals noDenormals;

  // Check if we have a valid sample rate
  if (!isSampleRateValid())
  {
    buffer.clear();
    return;
  }

  // Safely get parameter values at the start of the block
  const float rate = rateParam != nullptr ? rateParam->load() : defaultRate;
  const float depth = depthParam != nullptr ? depthParam->load() : defaultDepth;
  const float delay = delayParam != nullptr ? delayParam->load() : defaultDelay;
  const float mix = mixParam != nullptr ? mixParam->load() : defaultMix;
  const float phase = phaseParam != nullptr ? phaseParam->load() : defaultPhase;

  const int numChannels = buffer.getNumChannels();
  const int numSamples = buffer.getNumSamples();

  // Process each channel
  for (int channel = 0; channel < numChannels; ++channel)
  {
    auto *channelData = buffer.getWritePointer(channel);
    float phaseOffset = (channel == 1) ? phase : 0.0f; // Apply phase offset to right channel

    for (int sample = 0; sample < numSamples; ++sample)
    {
      const float in = channelData[sample];

      // Calculate LFO value
      float lfoValue = std::sin(2.0f * M_PI * (lfoPhase + phaseOffset / 360.0f));
      float delayTime = delay + (depth * lfoValue);

      // Process the sample
      float processed = processChannel(in, channel, delayTime);

      // Mix dry and wet signals
      channelData[sample] = in * (1.0f - mix) + processed * mix;

      // Update LFO phase
      lfoPhase += rate / currentSampleRate;
      if (lfoPhase >= 1.0f)
        lfoPhase -= 1.0f;
    }
  }
}

juce::AudioProcessorEditor *Chorus::createEditor()
{
  return new juce::GenericAudioProcessorEditor(*this);
}

void Chorus::getStateInformation(juce::MemoryBlock &destData)
{
  auto state = parameters.copyState();
  std::unique_ptr<juce::XmlElement> xml(state.createXml());
  copyXmlToBinary(*xml, destData);
}

void Chorus::setStateInformation(const void *data, int sizeInBytes)
{
  try
  {
    std::unique_ptr<juce::XmlElement> xmlState(getXmlFromBinary(data, sizeInBytes));

    if (xmlState != nullptr && xmlState->hasTagName(parameters.state.getType()))
    {
      parameters.replaceState(juce::ValueTree::fromXml(*xmlState));

      // Reset processing state after parameter changes
      reset();
    }
  }
  catch (...)
  {
    // If state restoration fails, reset to default state
    reset();
  }
}
