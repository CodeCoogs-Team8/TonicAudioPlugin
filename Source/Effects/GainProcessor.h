/*
  ==============================================================================

    GainProcessor.h
    Created: 3 Apr 2025 2:19:00pm
    Author:  Evan Fraustro

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>

class GainProcessor final : public juce::AudioProcessor
{
public:
  GainProcessor(bool isInput = true); // Default to input gain if not specified

  juce::AudioProcessorValueTreeState &getAPVTS() { return parameters; }

  ~GainProcessor() override;

  //===============================================================
  void prepareToPlay(double sampleRate, int samplesPerBlock) override;
  void releaseResources() override;

#ifndef JucePlugin_PreferredChannelConfigurations
  bool isBusesLayoutSupported(const BusesLayout &layouts) const override;
#endif

  void processBlock(juce::AudioBuffer<float> &, juce::MidiBuffer &) override;

  //===============================================================
  juce::AudioProcessorEditor *createEditor() override;
  bool hasEditor() const override;

  //===============================================================
  const juce::String getName() const override;

  bool acceptsMidi() const override;
  bool producesMidi() const override;
  bool isMidiEffect() const override;
  double getTailLengthSeconds() const override;

  //===============================================================
  int getNumPrograms() override;
  int getCurrentProgram() override;
  void setCurrentProgram(int index) override;
  const juce::String getProgramName(int index) override;
  void changeProgramName(int index, const juce::String &newName) override;

  //===============================================================
  void getStateInformation(juce::MemoryBlock &destData) override;
  void setStateInformation(const void *data, int sizeInBytes) override;

private:
  // Parameter ranges and defaults
  static constexpr float minGain = -24.0f;
  static constexpr float maxGain = 24.0f;
  static constexpr float defaultGain = 0.0f;

  // Sample rate validation
  static constexpr double MIN_SAMPLE_RATE = 8000.0;   // Minimum valid sample rate
  static constexpr double MAX_SAMPLE_RATE = 192000.0; // Maximum valid sample rate
  bool isSampleRateValid() const { return currentSampleRate >= MIN_SAMPLE_RATE && currentSampleRate <= MAX_SAMPLE_RATE; }

  juce::AudioProcessorValueTreeState parameters;
  std::atomic<float> *gainParam = nullptr;
  double currentSampleRate = 0.0; // Initialize to 0 to indicate not set

  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(GainProcessor)
};
