/*
  ==============================================================================

    Chorus.cpp
    Created: 15 Apr 2025 7:21:18pm
    Author:  Evan Fraustro

  ==============================================================================
*/

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

  // Safety check for initialization
  if (!isInitialized || !rateParam || !depthParam || !delayParam || !mixParam || !phaseParam)
  {
    buffer.clear();
    return;
  }

  const int numChannels = buffer.getNumChannels();
  const int numSamples = buffer.getNumSamples();

  // Get current parameter values with bounds checking
  const float rate = juce::jlimit(minRate, maxRate, getRate());
  const float depth = juce::jlimit(minDepth, maxDepth, getDepth());
  const float centerDelay = juce::jlimit(minDelay, maxDelay, getDelay());
  const float mix = juce::jlimit(minMix, maxMix, getMix());
  const float phaseOffset = juce::jlimit(minPhase, maxPhase, getPhase()) *
                            juce::MathConstants<float>::pi / 180.0f;

  // Calculate phase increment for LFO
  const float phaseInc = rate * juce::MathConstants<float>::twoPi / currentSampleRate;

  for (int sample = 0; sample < numSamples; ++sample)
  {
    // Calculate LFO values for left and right channels
    const float lfoLeft = std::sin(lfoPhase);
    const float lfoRight = std::sin(lfoPhase + phaseOffset);

    // Update LFO phase with wrapping
    lfoPhase = std::fmod(lfoPhase + phaseInc, juce::MathConstants<float>::twoPi);

    for (int channel = 0; channel < numChannels; ++channel)
    {
      auto *channelData = buffer.getWritePointer(channel);
      const float in = channelData[sample];

      // Calculate delay time modulation
      const float lfo = (channel == 0) ? lfoLeft : lfoRight;
      const float delayTime = centerDelay + (depth * 10.0f * lfo); // +/- 10ms modulation

      // Process through delay line
      const float delayedSample = processChannel(in, channel, delayTime);

      // Mix dry and wet signals with safety bounds
      channelData[sample] = in * (1.0f - mix) + delayedSample * mix;
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
