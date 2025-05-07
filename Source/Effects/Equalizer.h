/*
  ==============================================================================

    Equalizer.h
    Created: 15 Apr 2025 7:21:31pm
    Author:  Evan Fraustro

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>

class Equalizer : public juce::AudioProcessor
{
public:
  Equalizer();
  ~Equalizer() override;

  void prepareToPlay(double sampleRate, int samplesPerBlock) override;
  void releaseResources() override;
  void reset();

  void processBlock(juce::AudioBuffer<float> &buffer, juce::MidiBuffer &midiMessages) override;

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
  float getLowGain() const { return lowGainParam != nullptr ? lowGainParam->load() : defaultLowGain; }
  float getLowFreq() const { return lowFreqParam != nullptr ? lowFreqParam->load() : defaultLowFreq; }
  float getMidGain() const { return midGainParam != nullptr ? midGainParam->load() : defaultMidGain; }
  float getMidFreq() const { return midFreqParam != nullptr ? midFreqParam->load() : defaultMidFreq; }
  float getMidQ() const { return midQParam != nullptr ? midQParam->load() : defaultMidQ; }
  float getHighGain() const { return highGainParam != nullptr ? highGainParam->load() : defaultHighGain; }
  float getHighFreq() const { return highFreqParam != nullptr ? highFreqParam->load() : defaultHighFreq; }

  // Audio processor value tree
  juce::AudioProcessorValueTreeState parameters;

private:
  // Parameter pointers
  std::atomic<float> *lowGainParam = nullptr;  // Low shelf gain
  std::atomic<float> *lowFreqParam = nullptr;  // Low shelf frequency
  std::atomic<float> *midGainParam = nullptr;  // Mid peak gain
  std::atomic<float> *midFreqParam = nullptr;  // Mid peak frequency
  std::atomic<float> *midQParam = nullptr;     // Mid peak Q
  std::atomic<float> *highGainParam = nullptr; // High shelf gain
  std::atomic<float> *highFreqParam = nullptr; // High shelf frequency

  // Filter processors
  juce::dsp::ProcessorDuplicator<juce::dsp::IIR::Filter<float>, juce::dsp::IIR::Coefficients<float>> lowShelf;
  juce::dsp::ProcessorDuplicator<juce::dsp::IIR::Filter<float>, juce::dsp::IIR::Coefficients<float>> midPeak;
  juce::dsp::ProcessorDuplicator<juce::dsp::IIR::Filter<float>, juce::dsp::IIR::Coefficients<float>> highShelf;

  // Processing state
  double currentSampleRate = 0.0; // Initialize to 0 to indicate not set
  bool isInitialized = false;
  bool filtersNeedUpdate = true;

  // Sample rate validation
  static constexpr double MIN_SAMPLE_RATE = 8000.0;   // Minimum valid sample rate
  static constexpr double MAX_SAMPLE_RATE = 192000.0; // Maximum valid sample rate
  bool isSampleRateValid() const { return currentSampleRate >= MIN_SAMPLE_RATE && currentSampleRate <= MAX_SAMPLE_RATE; }

  // Parameter ranges and defaults
  static constexpr float minGain = -24.0f;
  static constexpr float maxGain = 24.0f;
  static constexpr float defaultLowGain = 0.0f;
  static constexpr float defaultMidGain = 0.0f;
  static constexpr float defaultHighGain = 0.0f;

  static constexpr float minLowFreq = 20.0f;
  static constexpr float maxLowFreq = 1000.0f;
  static constexpr float defaultLowFreq = 100.0f;

  static constexpr float minMidFreq = 200.0f;
  static constexpr float maxMidFreq = 5000.0f;
  static constexpr float defaultMidFreq = 1000.0f;

  static constexpr float minHighFreq = 1000.0f;
  static constexpr float maxHighFreq = 20000.0f;
  static constexpr float defaultHighFreq = 10000.0f;

  static constexpr float minQ = 0.1f;
  static constexpr float maxQ = 10.0f;
  static constexpr float defaultMidQ = 1.0f;

  std::atomic<bool> bypassed{false}; // Add bypass state

  void updateFilters();
  void resetFilters();

  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(Equalizer)
};
