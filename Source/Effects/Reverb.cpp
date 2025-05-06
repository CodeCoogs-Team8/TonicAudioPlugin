/*
  ==============================================================================

    Reverb.cpp
    Created: 15 Apr 2025 7:21:23pm
    Author:  Evan Fraustro

  ==============================================================================
*/

#include "Reverb.h"

Reverb::Reverb()
    : AudioProcessor(BusesProperties()
                         .withInput("Input", juce::AudioChannelSet::stereo(), true)
                         .withOutput("Output", juce::AudioChannelSet::stereo(), true)),
      parameters(*this, nullptr, juce::Identifier("ReverbParameters"),
                 {std::make_unique<juce::AudioParameterFloat>(
                      "roomSize",  // parameterID
                      "Room Size", // parameter name
                      0.0f,        // minimum value
                      1.0f,        // maximum value
                      0.5f         // default value
                      ),
                  std::make_unique<juce::AudioParameterFloat>(
                      "damping",        // parameterID
                      "Damping Amount", // parameter name
                      0.0f,             // minimum value
                      1.0f,             // maximum value
                      0.5f              // default value
                      ),
                  std::make_unique<juce::AudioParameterFloat>(
                      "wetLevel",  // parameterID
                      "Wet Level", // parameter name
                      0.0f,        // minimum value
                      1.0f,        // maximum value
                      0.33f        // default value
                      ),
                  std::make_unique<juce::AudioParameterFloat>(
                      "dryLevel",  // parameterID
                      "Dry Level", // parameter name
                      0.0f,        // minimum value
                      1.0f,        // maximum value
                      0.4f         // default value
                      ),
                  std::make_unique<juce::AudioParameterFloat>(
                      "width",        // parameterID
                      "Stereo Width", // parameter name
                      0.0f,           // minimum value
                      1.0f,           // maximum value
                      1.0f            // default value
                      ),
                  std::make_unique<juce::AudioParameterBool>(
                      "freezeMode",  // parameterID
                      "Freeze Mode", // parameter name
                      false          // default value
                      )})
{
  roomSizeParam = parameters.getRawParameterValue("roomSize");
  dampingParam = parameters.getRawParameterValue("damping");
  wetLevelParam = parameters.getRawParameterValue("wetLevel");
  dryLevelParam = parameters.getRawParameterValue("dryLevel");
  widthParam = parameters.getRawParameterValue("width");
  freezeModeParam = parameters.getRawParameterValue("freezeMode");
}

Reverb::~Reverb()
{
  releaseResources();
  roomSizeParam = nullptr;
  dampingParam = nullptr;
  wetLevelParam = nullptr;
  dryLevelParam = nullptr;
  widthParam = nullptr;
  freezeModeParam = nullptr;
}

void Reverb::prepareToPlay(double sampleRate, int samplesPerBlock)
{
  reverb.setSampleRate(sampleRate);
  updateReverbParameters();
}

void Reverb::releaseResources()
{
  // Reset the reverb state
  reverb.reset();
}

void Reverb::processBlock(juce::AudioBuffer<float> &buffer, juce::MidiBuffer &midiMessages)
{
  juce::ScopedNoDenormals noDenormals;

  // Check if we have a valid sample rate
  if (!isSampleRateValid())
  {
    buffer.clear();
    return;
  }

  // Update reverb parameters
  updateReverbParameters();

  // Process the reverb
  reverb.processStereo(buffer.getWritePointer(0), buffer.getWritePointer(1), buffer.getNumSamples());
}

juce::AudioProcessorEditor *Reverb::createEditor()
{
  return new juce::GenericAudioProcessorEditor(*this);
}

void Reverb::getStateInformation(juce::MemoryBlock &destData)
{
  auto state = parameters.copyState();
  std::unique_ptr<juce::XmlElement> xml(state.createXml());
  copyXmlToBinary(*xml, destData);
}

void Reverb::setStateInformation(const void *data, int sizeInBytes)
{
  try
  {
    std::unique_ptr<juce::XmlElement> xmlState(getXmlFromBinary(data, sizeInBytes));

    if (xmlState != nullptr && xmlState->hasTagName(parameters.state.getType()))
    {
      // Update parameters
      parameters.replaceState(juce::ValueTree::fromXml(*xmlState));

      // Update reverb parameters
      updateReverbParameters();

      // Reset reverb state if needed
      if (getSampleRate() > 0.0)
      {
        reverb.reset();
      }
    }
  }
  catch (...)
  {
    // If state restoration fails, reset to a safe state
    if (getSampleRate() > 0.0)
    {
      reverb.reset();
      updateReverbParameters();
    }
  }
}

void Reverb::updateReverbParameters()
{
  juce::Reverb::Parameters params;
  params.roomSize = roomSizeParam != nullptr ? roomSizeParam->load() : 0.5f;
  params.damping = dampingParam != nullptr ? dampingParam->load() : 0.5f;
  params.wetLevel = wetLevelParam != nullptr ? wetLevelParam->load() : 0.33f;
  params.dryLevel = dryLevelParam != nullptr ? dryLevelParam->load() : 0.4f;
  params.width = widthParam != nullptr ? widthParam->load() : 1.0f;
  params.freezeMode = freezeModeParam != nullptr ? freezeModeParam->load() > 0.5f : false;

  reverb.setParameters(params);
}
