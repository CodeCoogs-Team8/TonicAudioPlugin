/*
  ==============================================================================

    Reverb.h
    Created: 15 Apr 2025 7:21:23pm
    Author:  Evan Fraustro

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>

class Reverb : public juce::AudioProcessor
{
public:
  Reverb();
  ~Reverb() override;

  // AudioProcessor methods
  void prepareToPlay(double sampleRate, int samplesPerBlock) override;
  void releaseResources() override;
  void processBlock(juce::AudioBuffer<float> &, juce::MidiBuffer &) override;

  // Plugin methods
  const juce::String getName() const override { return "Reverb"; }
  bool acceptsMidi() const override { return false; }
  bool producesMidi() const override { return false; }
  bool isMidiEffect() const override { return false; }
  double getTailLengthSeconds() const override { return 2.0; }

  // Editor methods
  bool hasEditor() const override { return true; }
  juce::AudioProcessorEditor *createEditor() override;

  // Program methods
  int getNumPrograms() override { return 1; }
  int getCurrentProgram() override { return 0; }
  void setCurrentProgram(int) override {}
  const juce::String getProgramName(int) override { return {}; }
  void changeProgramName(int, const juce::String &) override {}

  // Bus layout methods
  bool isBusesLayoutSupported(const BusesLayout &layouts) const override
  {
    // Only support stereo
    return layouts.getMainOutputChannelSet() == juce::AudioChannelSet::stereo() &&
           layouts.getMainInputChannelSet() == juce::AudioChannelSet::stereo();
  }

  // State management
  void getStateInformation(juce::MemoryBlock &destData) override;
  void setStateInformation(const void *data, int sizeInBytes) override;

  juce::AudioProcessorValueTreeState parameters;

private:
  juce::Reverb reverb;

  std::atomic<float> *roomSizeParam = nullptr;
  std::atomic<float> *dampingParam = nullptr;
  std::atomic<float> *wetLevelParam = nullptr;
  std::atomic<float> *dryLevelParam = nullptr;
  std::atomic<float> *widthParam = nullptr;
  std::atomic<float> *freezeModeParam = nullptr;

  // Processing state
  double currentSampleRate = 0.0; // Initialize to 0 to indicate not set

  // Sample rate validation
  static constexpr double MIN_SAMPLE_RATE = 8000.0;   // Minimum valid sample rate
  static constexpr double MAX_SAMPLE_RATE = 192000.0; // Maximum valid sample rate
  bool isSampleRateValid() const { return currentSampleRate >= MIN_SAMPLE_RATE && currentSampleRate <= MAX_SAMPLE_RATE; }

  void updateReverbParameters();

  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(Reverb)
};
