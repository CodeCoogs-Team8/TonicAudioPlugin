/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "./Effects/EffectRack.h"

//==============================================================================
/**
 */
class DelayAudioProcessor : public juce::AudioProcessor
{
public:
  //==============================================================================
  DelayAudioProcessor();
  ~DelayAudioProcessor() override;

  //==============================================================================
  void prepareToPlay(double sampleRate, int samplesPerBlock) override;
  void releaseResources() override;

  bool isBusesLayoutSupported(const BusesLayout &layouts) const override;
  void processBlock(juce::AudioBuffer<float> &, juce::MidiBuffer &) override;

  //==============================================================================
  juce::AudioProcessorEditor *createEditor() override;
    bool hasEditor() const override;

  //==============================================================================
    const juce::String getName() const override;

    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;

  //==============================================================================
    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram(int) override;
    const juce::String getProgramName(int) override;
    void changeProgramName(int, const juce::String &) override;

  //==============================================================================
  void getStateInformation(juce::MemoryBlock &destData) override;
  void setStateInformation(const void *data, int sizeInBytes) override;

  // Effect rack access with thread safety
  EffectRack &getEffectRack()
  {
    const juce::ScopedLock sl(effectRackLock);
    return effectRack;
  }

  const EffectRack &getEffectRack() const
  {
    const juce::ScopedLock sl(effectRackLock);
    return effectRack;
  }

  // Level monitoring
  struct AudioLevels
  {
    float leftLevel = 0.0f;
    float rightLevel = 0.0f;
  };

  AudioLevels getCurrentLevels() const
  {
    const juce::ScopedLock sl(levelsLock);
    return currentLevels;
  }

private:
  //==============================================================================
  void removeAllGraphConnections();

  juce::AudioProcessorGraph audioGraph;
  juce::AudioProcessorGraph::Node::Ptr inputNode;
  juce::AudioProcessorGraph::Node::Ptr outputNode;
  EffectRack effectRack;

  mutable juce::CriticalSection effectRackLock;

  AudioLevels currentLevels;
  mutable juce::CriticalSection levelsLock;

  std::atomic<bool> isPrepared{false};

  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(DelayAudioProcessor)
};
