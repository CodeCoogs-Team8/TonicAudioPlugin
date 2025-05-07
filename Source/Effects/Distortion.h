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
  // Add enum for distortion types
  enum class DistortionType
  {
    SoftClip,  // Current tanh-based distortion
    HardClip,  // Hard clipping
    Fold,      // Wave folding
    BitCrush,  // Bit reduction
    SampleRate // Sample rate reduction
  };

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

  bool isBypassed() const { return bypassed; }
  void setBypassed(bool shouldBeBypassed) { bypassed = shouldBeBypassed; }

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
  std::atomic<float> *typeParam = nullptr; // New parameter for distortion type

  // Processing state
  double currentSampleRate = 0.0; // Initialize to 0 to indicate not set
  double lastSampleRate = 0.0;
  bool isInitialized = false;

  // Sample rate validation
  static constexpr double MIN_SAMPLE_RATE = 8000.0;   // Minimum valid sample rate
  static constexpr double MAX_SAMPLE_RATE = 192000.0; // Maximum valid sample rate
  bool isSampleRateValid() const { return currentSampleRate >= MIN_SAMPLE_RATE && currentSampleRate <= MAX_SAMPLE_RATE; }

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
  // Bit crushing parameters
  float bitCrushMaxValue = 255.0f;   // Default to 8-bit (2^8 - 1)
  std::atomic<bool> bypassed{false}; // Add bypass state
  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(Distortion)
};
