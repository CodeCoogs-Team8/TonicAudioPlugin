/*
  ==============================================================================

    Chorus.h
    Created: 15 Apr 2025 7:21:18pm
    Author:  Evan Fraustro

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>

class Chorus : public juce::AudioProcessor
{
public:
  Chorus();
  ~Chorus() override;

  void prepareToPlay(double sampleRate, int samplesPerBlock) override;
  void releaseResources() override;
  void reset();

  void processBlock(juce::AudioBuffer<float> &buffer, juce::MidiBuffer &midiMessages) override;
  float processChannel(float input, int channel, float delayTime);

  juce::AudioProcessorEditor *createEditor() override;
  bool hasEditor() const override { return true; }

  const juce::String getName() const override { return JucePlugin_Name; }

  bool acceptsMidi() const override { return false; }
  bool producesMidi() const override { return false; }
  bool isMidiEffect() const override { return false; }
  double getTailLengthSeconds() const override { return 0.0; }

  int getNumPrograms() override { return 1; }
  int getCurrentProgram() override { return 0; }
  void setCurrentProgram(int index) override {}
  const juce::String getProgramName(int index) override { return {}; }
  void changeProgramName(int index, const juce::String &newName) override {}

  void getStateInformation(juce::MemoryBlock &destData) override;
  void setStateInformation(const void *data, int sizeInBytes) override;

  // Parameter access methods with bounds checking
  float getRate() const { return rateParam != nullptr ? rateParam->load() : defaultRate; }
  float getDepth() const { return depthParam != nullptr ? depthParam->load() : defaultDepth; }
  float getDelay() const { return delayParam != nullptr ? delayParam->load() : defaultDelay; }
  float getMix() const { return mixParam != nullptr ? mixParam->load() : defaultMix; }
  float getPhase() const { return phaseParam != nullptr ? phaseParam->load() : defaultPhase; }

  // Audio processor value tree
  juce::AudioProcessorValueTreeState parameters;

private:
  // Parameter pointers
  std::atomic<float> *rateParam = nullptr;  // LFO rate (Hz)
  std::atomic<float> *depthParam = nullptr; // Modulation depth
  std::atomic<float> *delayParam = nullptr; // Center delay time
  std::atomic<float> *mixParam = nullptr;   // Wet/dry mix
  std::atomic<float> *phaseParam = nullptr; // Stereo phase offset

  // Processing state
  juce::dsp::DelayLine<float> delayLine;
  float lfoPhase = 0.0f;
  double currentSampleRate = 44100.0;
  bool isInitialized = false;
  bool filtersNeedUpdate = true;

  // Parameter ranges and defaults
  static constexpr float minRate = 0.1f;
  static constexpr float maxRate = 10.0f;
  static constexpr float defaultRate = 1.0f;

  static constexpr float minDepth = 0.0f;
  static constexpr float maxDepth = 1.0f;
  static constexpr float defaultDepth = 0.5f;

  static constexpr float minDelay = 5.0f;
  static constexpr float maxDelay = 30.0f;
  static constexpr float defaultDelay = 15.0f;

  static constexpr float minMix = 0.0f;
  static constexpr float maxMix = 1.0f;
  static constexpr float defaultMix = 0.5f;

  static constexpr float minPhase = 0.0f;
  static constexpr float maxPhase = 360.0f;
  static constexpr float defaultPhase = 90.0f;

  // Maximum delay time in milliseconds
  static constexpr float maxDelayTimeMs = 50.0f;

  void updateFilters();
  void resetFilters();

  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(Chorus)
};
