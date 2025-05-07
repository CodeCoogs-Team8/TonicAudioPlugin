/*
  ==============================================================================

    Delay.h
    Created: 15 Apr 2025 7:21:01pm
    Author:  Evan Fraustro

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>

class Delay : public juce::AudioProcessor
{
public:
  Delay();
  ~Delay() override;

  void prepareToPlay(double sampleRate, int samplesPerBlock) override;
  void releaseResources() override;
  void processBlock(juce::AudioBuffer<float> &, juce::MidiBuffer &) override;

  juce::AudioProcessorEditor *createEditor() override;
  bool hasEditor() const override { return true; }

  const juce::String getName() const override { return "Delay"; }

  bool acceptsMidi() const override { return false; }
  bool producesMidi() const override { return false; }
  bool isMidiEffect() const override { return false; }
  double getTailLengthSeconds() const override { return 0.0; }

  bool isBypassed() const { return bypassed; }
  void setBypassed(bool shouldBeBypassed) { bypassed = shouldBeBypassed; }

  int getNumPrograms() override { return 1; }
  int getCurrentProgram() override { return 0; }
  void setCurrentProgram(int) override {}
  const juce::String getProgramName(int) override { return {}; }
  void changeProgramName(int, const juce::String &) override {}

  void getStateInformation(juce::MemoryBlock &destData) override;
  void setStateInformation(const void *data, int sizeInBytes) override;
  juce::AudioProcessorValueTreeState parameters;

private:
  std::atomic<float> *delayTimeParam = nullptr;
  std::atomic<float> *feedbackParam = nullptr;
  std::atomic<float> *mixParam = nullptr;

  juce::dsp::DelayLine<float> delayLine;
  double currentSampleRate = 0.0;                     // Initialize to 0 to indicate not set
  static constexpr double MIN_SAMPLE_RATE = 8000.0;   // Minimum valid sample rate
  static constexpr double MAX_SAMPLE_RATE = 192000.0; // Maximum valid sample rate
  std::atomic<bool> bypassed{false};                  // Add bypass state

  bool isSampleRateValid() const { return currentSampleRate >= MIN_SAMPLE_RATE && currentSampleRate <= MAX_SAMPLE_RATE; }

  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(Delay)
};
