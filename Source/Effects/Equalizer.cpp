/*
  ==============================================================================

    Equalizer.cpp
    Created: 15 Apr 2025 7:21:31pm
    Author:  Evan Fraustro

  ==============================================================================
*/

#include "Equalizer.h"

Equalizer::Equalizer()
    : AudioProcessor(BusesProperties()
                         .withInput("Input", juce::AudioChannelSet::stereo(), true)
                         .withOutput("Output", juce::AudioChannelSet::stereo(), true)),
      parameters(*this, nullptr, "PARAMETERS",
                 {std::make_unique<juce::AudioParameterFloat>(
                      "lowGain", "Low Gain",
                      juce::NormalisableRange<float>(minGain, maxGain),
                      defaultLowGain),
                  std::make_unique<juce::AudioParameterFloat>(
                      "lowFreq", "Low Freq",
                      juce::NormalisableRange<float>(minLowFreq, maxLowFreq),
                      defaultLowFreq),
                  std::make_unique<juce::AudioParameterFloat>(
                      "midGain", "Mid Gain",
                      juce::NormalisableRange<float>(minGain, maxGain),
                      defaultMidGain),
                  std::make_unique<juce::AudioParameterFloat>(
                      "midFreq", "Mid Freq",
                      juce::NormalisableRange<float>(minMidFreq, maxMidFreq),
                      defaultMidFreq),
                  std::make_unique<juce::AudioParameterFloat>(
                      "midQ", "Mid Q",
                      juce::NormalisableRange<float>(minQ, maxQ),
                      defaultMidQ),
                  std::make_unique<juce::AudioParameterFloat>(
                      "highGain", "High Gain",
                      juce::NormalisableRange<float>(minGain, maxGain),
                      defaultHighGain),
                  std::make_unique<juce::AudioParameterFloat>(
                      "highFreq", "High Freq",
                      juce::NormalisableRange<float>(minHighFreq, maxHighFreq),
                      defaultHighFreq)})
{
  lowGainParam = parameters.getRawParameterValue("lowGain");
  lowFreqParam = parameters.getRawParameterValue("lowFreq");
  midGainParam = parameters.getRawParameterValue("midGain");
  midFreqParam = parameters.getRawParameterValue("midFreq");
  midQParam = parameters.getRawParameterValue("midQ");
  highGainParam = parameters.getRawParameterValue("highGain");
  highFreqParam = parameters.getRawParameterValue("highFreq");
}

Equalizer::~Equalizer()
{
  releaseResources();
  lowGainParam = nullptr;
  lowFreqParam = nullptr;
  midGainParam = nullptr;
  midFreqParam = nullptr;
  midQParam = nullptr;
  highGainParam = nullptr;
  highFreqParam = nullptr;
}

void Equalizer::prepareToPlay(double sampleRate, int samplesPerBlock)
{
  // Store sample rate and initialize processing
  currentSampleRate = sampleRate;

  // Configure filters
  juce::dsp::ProcessSpec spec;
  spec.sampleRate = sampleRate;
  spec.maximumBlockSize = static_cast<juce::uint32>(samplesPerBlock);
  spec.numChannels = 2;

  lowShelf.prepare(spec);
  midPeak.prepare(spec);
  highShelf.prepare(spec);

  // Reset state and update filters
  reset();
  isInitialized = true;
}

void Equalizer::releaseResources()
{
  reset();
  isInitialized = false;
}

void Equalizer::reset()
{
  resetFilters();
  filtersNeedUpdate = true;
}

void Equalizer::resetFilters()
{
  lowShelf.reset();
  midPeak.reset();
  highShelf.reset();
}

void Equalizer::updateFilters()
{
  if (!isInitialized)
    return;

  const float sampleRate = static_cast<float>(currentSampleRate);

  // Get parameter values with bounds checking
  const float lowGain = juce::jlimit(minGain, maxGain, getLowGain());
  const float lowFreq = juce::jlimit(minLowFreq, maxLowFreq, getLowFreq());
  const float midGain = juce::jlimit(minGain, maxGain, getMidGain());
  const float midFreq = juce::jlimit(minMidFreq, maxMidFreq, getMidFreq());
  const float midQ = juce::jlimit(minQ, maxQ, getMidQ());
  const float highGain = juce::jlimit(minGain, maxGain, getHighGain());
  const float highFreq = juce::jlimit(minHighFreq, maxHighFreq, getHighFreq());

  // Update low shelf filter
  *lowShelf.state = *juce::dsp::IIR::Coefficients<float>::makeLowShelf(
      sampleRate, lowFreq, 1.0f, juce::Decibels::decibelsToGain(lowGain));

  // Update mid peak filter
  *midPeak.state = *juce::dsp::IIR::Coefficients<float>::makePeakFilter(
      sampleRate, midFreq, midQ, juce::Decibels::decibelsToGain(midGain));

  // Update high shelf filter
  *highShelf.state = *juce::dsp::IIR::Coefficients<float>::makeHighShelf(
      sampleRate, highFreq, 1.0f, juce::Decibels::decibelsToGain(highGain));

  filtersNeedUpdate = false;
}

void Equalizer::processBlock(juce::AudioBuffer<float> &buffer, juce::MidiBuffer &midiMessages)
{
  juce::ScopedNoDenormals noDenormals;

  // Check if we have a valid sample rate
  if (!isSampleRateValid())
  {
    buffer.clear();
    return;
  }

  // Update filters if needed
  if (filtersNeedUpdate)
  {
    updateFilters();
    filtersNeedUpdate = false;
  }

  // Create an AudioBlock that references the entire buffer
  juce::dsp::AudioBlock<float> block(buffer);

  // Process through each filter
  lowShelf.process(juce::dsp::ProcessContextReplacing<float>(block));
  midPeak.process(juce::dsp::ProcessContextReplacing<float>(block));
  highShelf.process(juce::dsp::ProcessContextReplacing<float>(block));
}

juce::AudioProcessorEditor *Equalizer::createEditor()
{
  return new juce::GenericAudioProcessorEditor(*this);
}

void Equalizer::getStateInformation(juce::MemoryBlock &destData)
{
  auto state = parameters.copyState();
  std::unique_ptr<juce::XmlElement> xml(state.createXml());
  copyXmlToBinary(*xml, destData);
}

void Equalizer::setStateInformation(const void *data, int sizeInBytes)
{
  try
  {
    std::unique_ptr<juce::XmlElement> xmlState(getXmlFromBinary(data, sizeInBytes));

    if (xmlState != nullptr && xmlState->hasTagName(parameters.state.getType()))
    {
      parameters.replaceState(juce::ValueTree::fromXml(*xmlState));

      // Mark filters for update and reset processing state
      filtersNeedUpdate = true;
      reset();
    }
  }
  catch (...)
  {
    // If state restoration fails, reset to default state
    reset();
  }
}
