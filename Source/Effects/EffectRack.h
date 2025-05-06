/*
  ==============================================================================

    EffectRack.h
    Created: 21 Apr 2025 8:41:39pm
    Author:  Evan Fraustro

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "Delay.h"
#include "Reverb.h"
#include "Distortion.h"
#include "Chorus.h"
#include "Equalizer.h"

//==============================================================================
class EffectRack : juce::AudioProcessor
{
public:
  EffectRack();
  ~EffectRack();

  // AudioProcessor overrides
  const juce::String getName() const override { return "EffectRack"; }
  bool acceptsMidi() const override { return true; }
  bool producesMidi() const override { return false; }
  bool isMidiEffect() const override { return false; }
  double getTailLengthSeconds() const override { return 0.0; }
  int getNumPrograms() override { return 1; }
  int getCurrentProgram() override { return 0; }
  void setCurrentProgram(int) override {}
  const juce::String getProgramName(int) override { return {}; }
  void changeProgramName(int, const juce::String &) override {}
  void getStateInformation(juce::MemoryBlock &destData) override;
  void setStateInformation(const void *data, int sizeInBytes) override;
  bool hasEditor() const override { return false; }
  juce::AudioProcessorEditor *createEditor() override { return nullptr; }

  // Audio processing methods
  void prepareToPlay(double sampleRate, int samplesPerBlock);
  void processBlock(juce::AudioBuffer<float> &buffer, juce::MidiBuffer &midiMessages);
  void releaseResources();

  // Effect management
  void addEffect(std::unique_ptr<juce::AudioProcessor> effect);
  void removeEffect(int index);
  void moveEffect(int fromIndex, int toIndex);
  int getNumEffects() const;
  juce::AudioProcessor *getEffect(int index) const;
  void clearEffects();

  // Effect state queries
  bool isEffectActive(int index) const;
  void setEffectActive(int index, bool active);
  juce::String getEffectName(int index) const;
  int findEffectPosition(const juce::String &name) const;

  // State management
  bool rebuildConnections();

  // Effect order management
  int getEffectOrder(int index) const;
  void setEffectOrder(int index, int newOrder);
  void updateEffectOrder();
  std::vector<juce::AudioProcessor *> getEffectOrder() const;

  // Graph management
  void updateGraphConnections();
  bool isGraphUpdatePending() const { return graphUpdatePending; }

  // Audio levels structure
  struct AudioLevels
  {
    float inputLevel = 0.0f;
    float outputLevel = 0.0f;
  };

private:
  struct EffectNode
  {
    juce::AudioProcessorGraph::Node::Ptr node;
    bool isActive = true;
    juce::String name;
    int position;
    bool isBeingDeleted = false;
  };

  // Graph management
  void updateGraph();
  void rebuildGraph();
  void createBasicGraph();
  void connectNodes();
  juce::AudioProcessorGraph::Node::Ptr addNodeToGraph(std::unique_ptr<juce::AudioProcessor> processor);

  // The main audio processing graph
  juce::AudioProcessorGraph graph;

  // Input/Output nodes
  juce::AudioProcessorGraph::Node::Ptr inputNode;
  juce::AudioProcessorGraph::Node::Ptr outputNode;

  // Thread safety
  mutable juce::ReadWriteLock effectsLock;
  juce::CriticalSection processLock;
  std::atomic<bool> graphUpdatePending{false};

  // State tracking
  double currentSampleRate = 44100.0;
  int currentBlockSize = 512;
  bool isPrepared = false;

  // Effect storage
  std::vector<EffectNode> effects;
  std::vector<juce::AudioProcessor *> effectOrder;

  // Prevent copying
  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(EffectRack)
};
