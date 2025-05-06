/*
  ==============================================================================

    Distortion.h
    Created: 15 Apr 2025 7:21:10pm
    Author:  Evan Fraustro

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>

class Distortion : public juce::AudioProcessor
{
public:
  Distortion();
  ~Distortion() override;

  void prepareToPlay(double sampleRate, int samplesPerBlock) override;
  void releaseResources() override;
  void reset();

  void processBlock(juce::AudioBuffer<float> &buffer, juce::MidiBuffer &midiMessages) override;
  float processSample(float sample, float drive, float range);

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
  float getDrive() const { return driveParam != nullptr ? driveParam->load() : defaultDrive; }
  float getRange() const { return rangeParam != nullptr ? rangeParam->load() : defaultRange; }
  float getMix() const { return mixParam != nullptr ? mixParam->load() : defaultMix; }
  float getOutputGain() const { return outputGainParam != nullptr ? outputGainParam->load() : defaultGain; }

  // Audio processor value tree
  juce::AudioProcessorValueTreeState parameters;

private:
  // Parameter pointers
  std::atomic<float> *driveParam = nullptr;
  std::atomic<float> *rangeParam = nullptr;
  std::atomic<float> *mixParam = nullptr;
  std::atomic<float> *outputGainParam = nullptr;

  // Processing state
  double currentSampleRate = 44100.0;
  double lastSampleRate = 0.0;
  bool isInitialized = false;

  // Parameter ranges and defaults
  static constexpr float minDrive = 1.0f;
  static constexpr float maxDrive = 25.0f;
  static constexpr float defaultDrive = 1.0f;

  static constexpr float minRange = 0.0f;
  static constexpr float maxRange = 1.0f;
  static constexpr float defaultRange = 0.8f;

  static constexpr float minMix = 0.0f;
  static constexpr float maxMix = 1.0f;
  static constexpr float defaultMix = 1.0f;

  static constexpr float minGain = 0.0f;
  static constexpr float maxOutputGain = 2.0f;
  static constexpr float defaultGain = 1.0f;

  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(Distortion)
};
