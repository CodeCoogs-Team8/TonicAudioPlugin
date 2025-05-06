/*
  ==============================================================================

    Distortion.cpp
    Created: 15 Apr 2025 7:21:10pm
    Author:  Evan Fraustro

  ==============================================================================
*/

#include "Distortion.h"

Distortion::Distortion()
    : AudioProcessor(BusesProperties()
                         .withInput("Input", juce::AudioChannelSet::stereo(), true)
                         .withOutput("Output", juce::AudioChannelSet::stereo(), true)),
      parameters(*this, nullptr, "PARAMETERS",
                 {std::make_unique<juce::AudioParameterFloat>(
                      "drive",        // parameterID
                      "Drive Amount", // parameter name
                      1.0f,           // minimum value
                      25.0f,          // maximum value
                      1.0f            // default value
                      ),
                  std::make_unique<juce::AudioParameterFloat>(
                      "range",            // parameterID
                      "Distortion Range", // parameter name
                      0.0f,               // minimum value
                      1.0f,               // maximum value
                      0.8f                // default value
                      ),
                  std::make_unique<juce::AudioParameterFloat>(
                      "mix",         // parameterID
                      "Dry/Wet Mix", // parameter name
                      0.0f,          // minimum value
                      1.0f,          // maximum value
                      1.0f           // default value
                      ),
                  std::make_unique<juce::AudioParameterFloat>(
                      "outputGain",   // parameterID
                      "Output Level", // parameter name
                      0.0f,           // minimum value
                      2.0f,           // maximum value
                      1.0f            // default value
                      )})
{
  driveParam = parameters.getRawParameterValue("drive");
  rangeParam = parameters.getRawParameterValue("range");
  mixParam = parameters.getRawParameterValue("mix");
  outputGainParam = parameters.getRawParameterValue("outputGain");
}

Distortion::~Distortion()
{
  releaseResources();
  driveParam = nullptr;
  rangeParam = nullptr;
  mixParam = nullptr;
  outputGainParam = nullptr;
}

void Distortion::prepareToPlay(double sampleRate, int samplesPerBlock)
{
  // Store sample rate for potential future use
  currentSampleRate = sampleRate;

  // Reset any processing state if needed
  reset();
}

void Distortion::releaseResources()
{
  // Reset any processing state
  reset();
}

void Distortion::reset()
{
  // Clear any processing state
  lastSampleRate = 0.0;
}

float Distortion::processSample(float sample, float drive, float range)
{
  // Ensure drive and range are within valid bounds
  drive = std::max(1.0f, std::min(drive, 25.0f));
  range = std::max(0.0f, std::min(range, 1.0f));

  float input = sample * drive;

  // Soft clipping using tanh
  float processed = std::tanh(input);

  // Apply range control (blend between distorted and hard clipped)
  if (range < 1.0f)
  {
    float hardClipped = input < -1.0f ? -1.0f : (input > 1.0f ? 1.0f : input);
    processed = processed * range + hardClipped * (1.0f - range);
  }

  return processed;
}

void Distortion::processBlock(juce::AudioBuffer<float> &buffer, juce::MidiBuffer &midiMessages)
{
  juce::ScopedNoDenormals noDenormals;

  // Check if parameters are valid
  if (!driveParam || !rangeParam || !mixParam || !outputGainParam)
    return;

  const int numChannels = buffer.getNumChannels();
  const int numSamples = buffer.getNumSamples();

  // Get current parameter values with safe defaults if null
  const float drive = driveParam ? driveParam->load() : 1.0f;
  const float range = rangeParam ? rangeParam->load() : 0.8f;
  const float mix = mixParam ? mixParam->load() : 1.0f;
  const float outputGain = outputGainParam ? outputGainParam->load() : 1.0f;

  // Process each channel
  for (int channel = 0; channel < numChannels; ++channel)
  {
    auto *channelData = buffer.getWritePointer(channel);

    for (int sample = 0; sample < numSamples; ++sample)
    {
      const float input = channelData[sample];
      const float processed = processSample(input, drive, range);

      // Mix dry and wet signals with safety bounds
      const float wetDry = std::max(0.0f, std::min(mix, 1.0f));
      const float gain = std::max(0.0f, std::min(outputGain, 2.0f));
      channelData[sample] = (input * (1.0f - wetDry) + processed * wetDry) * gain;
    }
  }
}

juce::AudioProcessorEditor *Distortion::createEditor()
{
  return new juce::GenericAudioProcessorEditor(*this);
}

void Distortion::getStateInformation(juce::MemoryBlock &destData)
{
  auto state = parameters.copyState();
  std::unique_ptr<juce::XmlElement> xml(state.createXml());
  copyXmlToBinary(*xml, destData);
}

void Distortion::setStateInformation(const void *data, int sizeInBytes)
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
