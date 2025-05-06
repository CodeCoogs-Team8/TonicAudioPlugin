/*
  ==============================================================================

    EffectRack.cpp
    Created: 21 Apr 2025 8:41:39pm
    Author:  Evan Fraustro

  ==============================================================================
*/

#include "EffectRack.h"
#include <JuceHeader.h>

EffectRack::EffectRack()
{
  // Initialize the graph first
  graph.enableAllBuses();
  createBasicGraph();
}

EffectRack::~EffectRack()
{
  const juce::ScopedWriteLock sl(effectsLock);
  const juce::ScopedLock pl(processLock);

  // First, mark all effects as being deleted to prevent any further processing
  for (auto &effect : effects)
  {
    effect.isBeingDeleted = true;
  }

  // Release resources for all processors
  for (auto &effect : effects)
  {
    if (effect.node != nullptr && effect.node->getProcessor() != nullptr)
    {
      effect.node->getProcessor()->releaseResources();
    }
  }

  // Clear the graph first to disconnect all nodes
  graph.clear();

  // Clear the effects vector
  effects.clear();

  // Reset nodes
  inputNode = nullptr;
  outputNode = nullptr;
}

void EffectRack::prepareToPlay(double sampleRate, int samplesPerBlock)
{
  const juce::ScopedLock pl(processLock);

  currentSampleRate = sampleRate;
  currentBlockSize = samplesPerBlock;

  // Configure and prepare the graph
  graph.setPlayConfigDetails(2, 2, sampleRate, samplesPerBlock);
  graph.prepareToPlay(sampleRate, samplesPerBlock);

  updateGraph();
  isPrepared = true;
}

void EffectRack::releaseResources()
{
  const juce::ScopedLock pl(processLock);
  isPrepared = false;
  graph.releaseResources();
}

void EffectRack::processBlock(juce::AudioBuffer<float> &buffer, juce::MidiBuffer &midiMessages)
{
  if (!isPrepared)
    return;

  juce::ScopedNoDenormals noDenormals;

  // Check if we need to update the graph
  if (graphUpdatePending)
  {
    const juce::ScopedLock pl(processLock);
    rebuildConnections();
    graphUpdatePending = false;
  }

  // Clear any unused output channels
  for (auto i = getTotalNumInputChannels(); i < getTotalNumOutputChannels(); ++i)
    buffer.clear(i, 0, buffer.getNumSamples());

  // Process the effect rack
  {
    const juce::ScopedLock sl(processLock);
    graph.processBlock(buffer, midiMessages);
  }

  // Update output levels
  AudioLevels levels;
  // ... rest of the processBlock implementation ...
}

void EffectRack::createBasicGraph()
{
  const juce::ScopedLock pl(processLock);

  // Clear everything first
  graph.clear();
  inputNode = nullptr;
  outputNode = nullptr;

  // Create input node
  auto audioInputProcessor = std::make_unique<juce::AudioProcessorGraph::AudioGraphIOProcessor>(
      juce::AudioProcessorGraph::AudioGraphIOProcessor::audioInputNode);
  inputNode = graph.addNode(std::move(audioInputProcessor));

  // Create output node
  auto audioOutputProcessor = std::make_unique<juce::AudioProcessorGraph::AudioGraphIOProcessor>(
      juce::AudioProcessorGraph::AudioGraphIOProcessor::audioOutputNode);
  outputNode = graph.addNode(std::move(audioOutputProcessor));

  if (inputNode != nullptr && outputNode != nullptr)
  {
    // Create initial input to output connection
    for (int channel = 0; channel < 2; ++channel)
    {
      auto sourceChannelIndex = channel;
      auto destChannelIndex = channel;

      graph.addConnection({{inputNode->nodeID, sourceChannelIndex},
                           {outputNode->nodeID, destChannelIndex}});
    }
  }
}

void EffectRack::connectNodes()
{
  const juce::ScopedLock pl(processLock);

  if (!rebuildConnections())
  {
    // If rebuilding fails, fall back to basic graph
    createBasicGraph();
  }
}

void EffectRack::updateGraph()
{
  const juce::ScopedLock pl(processLock);

  if (!isPrepared)
    return;

  try
  {
    // Ensure we have basic nodes
    if (inputNode == nullptr || outputNode == nullptr)
    {
      createBasicGraph();
      if (inputNode == nullptr || outputNode == nullptr)
        return;
    }

    // Configure the graph
    graph.setPlayConfigDetails(2, 2, currentSampleRate, currentBlockSize);

    // Prepare all effect nodes
    for (auto &effect : effects)
    {
      if (effect.node != nullptr && effect.node->getProcessor() != nullptr)
      {
        auto *processor = effect.node->getProcessor();
        processor->setPlayConfigDetails(2, 2, currentSampleRate, currentBlockSize);
        processor->prepareToPlay(currentSampleRate, currentBlockSize);
      }
    }

    // Update connections
    connectNodes();
  }
  catch (...)
  {
    // If anything goes wrong, ensure we have a valid audio path
    createBasicGraph();
  }
}

void EffectRack::addEffect(std::unique_ptr<juce::AudioProcessor> effect)
{
  const juce::ScopedWriteLock sl(effectsLock);
  const juce::ScopedLock pl(processLock);

  if (effect != nullptr)
  {
    EffectNode node;
    node.node = graph.addNode(std::move(effect));
    node.name = node.node->getProcessor()->getName();
    node.position = static_cast<int>(effects.size()); // Add at the end
    node.isActive = true;

    effects.push_back(std::move(node));
    updateEffectOrder();
    graphUpdatePending = true;
  }
}

void EffectRack::removeEffect(int index)
{
  const juce::ScopedWriteLock sl(effectsLock);
  const juce::ScopedLock pl(processLock);

  if (index >= 0 && index < static_cast<int>(effects.size()))
  {
    // Set the deletion flag before removing
    effects[index].isBeingDeleted = true;

    if (effects[index].node != nullptr)
    {
      effects[index].node->getProcessor()->releaseResources();
      graph.removeNode(effects[index].node->nodeID);
    }

    effects.erase(effects.begin() + index);

    // Update positions
    for (size_t i = 0; i < effects.size(); ++i)
    {
      effects[i].position = static_cast<int>(i);
    }

    connectNodes();
  }
}

void EffectRack::moveEffect(int fromIndex, int toIndex)
{
  // Get both locks to prevent audio processing during rearrangement
  const juce::ScopedWriteLock sl(effectsLock);
  const juce::ScopedLock pl(processLock);

  if (fromIndex >= 0 && fromIndex < static_cast<int>(effects.size()) &&
      toIndex >= 0 && toIndex < static_cast<int>(effects.size()) &&
      fromIndex != toIndex)
  {
    try
    {
      // Move the effect
      auto effect = std::move(effects[fromIndex]);
      effects.erase(effects.begin() + fromIndex);
      effects.insert(effects.begin() + toIndex, std::move(effect));

      // Update positions
      for (size_t i = 0; i < effects.size(); ++i)
      {
        effects[i].position = static_cast<int>(i);
      }

      // Rebuild connections
      connectNodes();
    }
    catch (...)
    {
      // If something goes wrong, try to recover to a basic state
      createBasicGraph();
    }
  }
}

int EffectRack::getNumEffects() const
{
  const juce::ScopedReadLock sl(effectsLock);
  return static_cast<int>(effects.size());
}

juce::AudioProcessor *EffectRack::getEffect(int index) const
{
  const juce::ScopedReadLock sl(effectsLock);
  if (index >= 0 && index < static_cast<int>(effects.size()))
  {
    if (effects[index].node != nullptr)
    {
      return effects[index].node->getProcessor();
    }
  }
  return nullptr;
}

bool EffectRack::isEffectActive(int index) const
{
  const juce::ScopedReadLock sl(effectsLock);
  if (index >= 0 && index < static_cast<int>(effects.size()))
  {
    return effects[index].isActive;
  }
  return false;
}

void EffectRack::setEffectActive(int index, bool active)
{
  // Get both locks to ensure thread safety during state change
  const juce::ScopedWriteLock sl(effectsLock);
  const juce::ScopedLock pl(processLock);

  if (index >= 0 && index < static_cast<int>(effects.size()))
  {
    try
    {
      // Store old state in case we need to revert
      bool oldState = effects[index].isActive;

      // Update state
      effects[index].isActive = active;

      // If we're enabling an effect, make sure it's prepared
      if (active && isPrepared && effects[index].node != nullptr && effects[index].node->getProcessor() != nullptr)
      {
        auto *processor = effects[index].node->getProcessor();
        processor->setPlayConfigDetails(2, 2, currentSampleRate, currentBlockSize);
        processor->prepareToPlay(currentSampleRate, currentBlockSize);
      }

      // Try to rebuild connections with new state
      if (!rebuildConnections())
      {
        // If rebuilding fails, revert to old state
        effects[index].isActive = oldState;
        rebuildConnections();
      }
    }
    catch (...)
    {
      // If anything goes wrong, try to restore to a working state
      createBasicGraph();
      if (isPrepared)
      {
        graph.setPlayConfigDetails(2, 2, currentSampleRate, currentBlockSize);
        graph.prepareToPlay(currentSampleRate, currentBlockSize);
      }
    }
  }
}

bool EffectRack::rebuildConnections()
{
  if (inputNode == nullptr || outputNode == nullptr)
    return false;

  try
  {
    // Clear all existing connections
    graph.getConnections().clear();

    // Start with input node
    juce::AudioProcessorGraph::Node::Ptr previousNode = inputNode;
    bool anyConnectionFailed = false;

    // Connect through active effects
    for (auto &effect : effects)
    {
      if (effect.isActive && effect.node != nullptr && effect.node->getProcessor() != nullptr)
      {
        bool allConnectionsSucceeded = true;

        // Connect all channels
        for (int channel = 0; channel < 2; ++channel)
        {
          bool success = graph.addConnection({{previousNode->nodeID, channel},
                                              {effect.node->nodeID, channel}});

          if (!success)
          {
            allConnectionsSucceeded = false;
            anyConnectionFailed = true;
            break;
          }
        }

        if (allConnectionsSucceeded)
        {
          previousNode = effect.node;
        }
      }
    }

    // Always connect the last node to the output
    if (previousNode != nullptr)
    {
      for (int channel = 0; channel < 2; ++channel)
      {
        if (!graph.addConnection({{previousNode->nodeID, channel},
                                  {outputNode->nodeID, channel}}))
        {
          anyConnectionFailed = true;
          break;
        }
      }
    }
    else
    {
      // If no effects are active, connect input directly to output
      for (int channel = 0; channel < 2; ++channel)
      {
        if (!graph.addConnection({{inputNode->nodeID, channel},
                                  {outputNode->nodeID, channel}}))
        {
          anyConnectionFailed = true;
          break;
        }
      }
    }

    return !anyConnectionFailed;
  }
  catch (...)
  {
    return false;
  }
}

juce::String EffectRack::getEffectName(int index) const
{
  const juce::ScopedReadLock sl(effectsLock);
  if (index >= 0 && index < static_cast<int>(effects.size()))
  {
    return effects[index].name;
  }
  return {};
}

int EffectRack::findEffectPosition(const juce::String &name) const
{
  const juce::ScopedReadLock sl(effectsLock);
  for (size_t i = 0; i < effects.size(); ++i)
  {
    if (effects[i].name == name)
    {
      return static_cast<int>(i);
    }
  }
  return -1;
}

void EffectRack::clearEffects()
{
  const juce::ScopedWriteLock sl(effectsLock);
  const juce::ScopedLock pl(processLock);

  // Mark all effects as being deleted
  for (auto &effect : effects)
  {
    effect.isBeingDeleted = true;
  }

  // First release all processors
  for (auto &effect : effects)
  {
    if (effect.node != nullptr && effect.node->getProcessor() != nullptr)
    {
      effect.node->getProcessor()->releaseResources();
    }
  }

  // Clear the graph
  graph.clear();
  effects.clear();

  // Reset nodes
  inputNode = nullptr;
  outputNode = nullptr;

  // Recreate basic structure
  createBasicGraph();
}

void EffectRack::getStateInformation(juce::MemoryBlock &destData)
{
  const juce::ScopedReadLock sl(effectsLock);

  juce::ValueTree state("EFFECTRACK");
  state.setProperty("numEffects", static_cast<int>(effects.size()), nullptr);
  state.setProperty("sampleRate", currentSampleRate, nullptr);
  state.setProperty("blockSize", currentBlockSize, nullptr);

  for (int i = 0; i < static_cast<int>(effects.size()); ++i)
  {
    // Add additional null checks
    if (effects[i].node != nullptr &&
        effects[i].node->getProcessor() != nullptr &&
        !effects[i].isBeingDeleted) // Use the flag instead of isBeingDeleted()
    {
      juce::ValueTree effectState("EFFECT" + juce::String(i));
      effectState.setProperty("name", effects[i].name, nullptr);
      effectState.setProperty("active", effects[i].isActive, nullptr);
      effectState.setProperty("position", effects[i].position, nullptr);

      // Get processor state safely with try-catch
      try
      {
        juce::MemoryBlock processorData;
        effects[i].node->getProcessor()->getStateInformation(processorData);
        if (processorData.getSize() > 0)
        {
          effectState.setProperty("processorState", processorData.toBase64Encoding(), nullptr);
        }
        state.addChild(effectState, -1, nullptr);
      }
      catch (...)
      {
        // Skip this effect if we can't get its state
        continue;
      }
    }
  }

  std::unique_ptr<juce::XmlElement> xml(state.createXml());
  juce::AudioProcessor::copyXmlToBinary(*xml, destData);
}

void EffectRack::setStateInformation(const void *data, int sizeInBytes)
{
  const juce::ScopedWriteLock sl(effectsLock);
  const juce::ScopedLock pl(processLock);

  // Store current state in case restoration fails
  auto oldEffects = effects;

  try
  {
    // Clear existing effects first
    effects.clear();
    graph.clear();
    inputNode = nullptr;
    outputNode = nullptr;

    std::unique_ptr<juce::XmlElement> xml(juce::AudioProcessor::getXmlFromBinary(data, sizeInBytes));
    if (xml != nullptr && xml->hasTagName("EFFECTRACK"))
    {
      juce::ValueTree state = juce::ValueTree::fromXml(*xml);

      currentSampleRate = state.getProperty("sampleRate", 44100.0);
      currentBlockSize = state.getProperty("blockSize", 512);

      // Recreate the basic graph structure
      createBasicGraph();

      // Restore effects
      for (int i = 0; i < state.getNumChildren(); ++i)
      {
        juce::ValueTree effectState = state.getChild(i);
        if (effectState.hasType("EFFECT" + juce::String(i)))
        {
          juce::String name = effectState.getProperty("name");
          bool active = effectState.getProperty("active", true);

          // Create appropriate processor
          std::unique_ptr<juce::AudioProcessor> processor;
          if (name == "Delay")
            processor = std::make_unique<Delay>();
          else if (name == "Reverb")
            processor = std::make_unique<Reverb>();
          else if (name == "Distortion")
            processor = std::make_unique<Distortion>();
          else if (name == "Chorus")
            processor = std::make_unique<Chorus>();
          else if (name == "EQ")
            processor = std::make_unique<Equalizer>();

          if (processor != nullptr)
          {
            // Restore processor state if available
            juce::String processorStateBase64 = effectState.getProperty("processorState");
            if (processorStateBase64.isNotEmpty())
            {
              juce::MemoryBlock processorData;
              processorData.fromBase64Encoding(processorStateBase64);
              processor->setStateInformation(processorData.getData(),
                                             static_cast<int>(processorData.getSize()));
            }

            // Add to rack
            auto node = graph.addNode(std::move(processor));
            if (node != nullptr)
            {
              EffectNode effectNode;
              effectNode.name = name;
              effectNode.isActive = active;
              effectNode.position = effectState.getProperty("position");
              effectNode.node = node;
              effects.push_back(std::move(effectNode));
            }
          }
        }
      }

      // Prepare if needed
      if (isPrepared)
      {
        graph.setPlayConfigDetails(2, 2, currentSampleRate, currentBlockSize);
        graph.prepareToPlay(currentSampleRate, currentBlockSize);
        if (!rebuildConnections())
        {
          throw std::runtime_error("Failed to rebuild connections");
        }
      }
    }
    else
    {
      throw std::runtime_error("Invalid state data");
    }
  }
  catch (...)
  {
    // Restore old state if anything goes wrong
    effects = std::move(oldEffects);

    // Clear and rebuild the graph
    graph.clear();
    createBasicGraph();

    // Restore the nodes to the graph
    for (auto &effect : effects)
    {
      if (effect.node != nullptr && effect.node->getProcessor() != nullptr)
      {
        auto processor = std::unique_ptr<juce::AudioProcessor>(effect.node->getProcessor());
        effect.node = graph.addNode(std::move(processor));
      }
    }

    if (isPrepared)
    {
      graph.setPlayConfigDetails(2, 2, currentSampleRate, currentBlockSize);
      graph.prepareToPlay(currentSampleRate, currentBlockSize);
      rebuildConnections();
    }
  }
}

int EffectRack::getEffectOrder(int index) const
{
  const juce::ScopedReadLock sl(effectsLock);
  if (index >= 0 && index < static_cast<int>(effects.size()))
  {
    return effects[index].position;
  }
  return -1;
}

void EffectRack::setEffectOrder(int index, int newOrder)
{
  const juce::ScopedWriteLock sl(effectsLock);
  if (index >= 0 && index < static_cast<int>(effects.size()))
  {
    effects[index].position = newOrder;
    updateEffectOrder();
    graphUpdatePending = true;
  }
}

void EffectRack::updateEffectOrder()
{
  const juce::ScopedWriteLock sl(effectsLock);

  // Sort effects by their position
  std::sort(effects.begin(), effects.end(),
            [](const EffectNode &a, const EffectNode &b)
            {
              return a.position < b.position;
            });

  // Update positions to be sequential
  for (size_t i = 0; i < effects.size(); ++i)
  {
    effects[i].position = static_cast<int>(i);
  }
}

std::vector<juce::AudioProcessor *> EffectRack::getEffectOrder() const
{
  return effectOrder;
}

void EffectRack::updateGraphConnections()
{
  // This should be called from the message thread
  jassert(juce::MessageManager::getInstance()->isThisTheMessageThread());

  if (graphUpdatePending)
  {
    const juce::ScopedLock pl(processLock);
    rebuildConnections();
    graphUpdatePending = false;
  }
}
