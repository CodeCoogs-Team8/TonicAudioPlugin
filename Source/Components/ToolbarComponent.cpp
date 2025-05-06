/*
  ==============================================================================

    ToolbarComponent.cpp
    Created: 15 Apr 2025 7:19:28pm
    Author:  Evan Fraustro

  ==============================================================================
*/

#include "ToolbarComponent.h"
#include "../Effects/Reverb.h"
#include "../Effects/Delay.h"
#include "../Effects/Distortion.h"
#include "../Effects/Chorus.h"
#include "../Effects/Equalizer.h"

ToolbarComponent::ToolbarComponent(EffectRack &rack)
    : effectRack(rack)
{
  // Create and add delay button
  auto delayButton = std::make_unique<EffectButton>("Delay");
  delayButton->onStateChange = [this](bool isActive)
  {
    if (isActive)
    {
      auto delay = std::make_unique<Delay>();
      auto *delayPtr = delay.get();
      int position = this->findNextAvailablePosition();

      this->effectRack.addEffect(std::move(delay));
      this->saveEffectState("Delay", delayPtr, position);

      if (this->onEffectVisibilityChanged)
        this->onEffectVisibilityChanged("Delay", delayPtr, true);
    }
    else
    {
      auto it = this->effectStates.find("Delay");
      if (it != this->effectStates.end())
      {
          int index = this ->effectRack.findEffectPosition("Delay");
          if (index >= 0)
              this->effectRack.setEffectActive(index, false);
        if (this->onEffectVisibilityChanged)
          this->onEffectVisibilityChanged("Delay", it->second.processor, false);

        it->second.isEnabled = false;
      }
    }
  };
  addAndMakeVisible(delayButton.get());
  effectButtons.push_back(std::move(delayButton));

  // Create and add distortion button
  auto distortionButton = std::make_unique<EffectButton>("Distortion");
  distortionButton->onStateChange = [this](bool isActive)
  {
    if (isActive)
    {
      // Create a new Distortion effect at the next available position
      auto distortion = std::make_unique<Distortion>();
      auto *distortionPtr = distortion.get();
      int position = findNextAvailablePosition();

      // Add to effect rack and save state
      effectRack.addEffect(std::move(distortion));
      saveEffectState("Distortion", distortionPtr, position);

      if (onEffectVisibilityChanged)
        onEffectVisibilityChanged("Distortion", distortionPtr, true);
    }
    else
    {
      auto it = effectStates.find("Distortion");
      if (it != effectStates.end())
      {
        if (onEffectVisibilityChanged)
          onEffectVisibilityChanged("Distortion", it->second.processor, false);

        it->second.isEnabled = false;
      }
    }
  };
  addAndMakeVisible(distortionButton.get());
  effectButtons.push_back(std::move(distortionButton));

  // Create and add reverb button
  auto reverbButton = std::make_unique<EffectButton>("Reverb");
  reverbButton->onStateChange = [this](bool isActive)
  {
    if (isActive)
    {
      // Check if we have a saved state
      auto it = effectStates.find("Reverb");
      if (it != effectStates.end() && !it->second.isEnabled)
      {
        // Restore the existing reverb at its previous position
        it->second.isEnabled = true;
        if (onEffectVisibilityChanged)
          onEffectVisibilityChanged("Reverb", it->second.processor, true);
      }
      else
      {
        // Create a new Reverb effect at the next available position
        auto reverb = std::make_unique<Reverb>();
        auto *reverbPtr = reverb.get();
        int position = findNextAvailablePosition();

        // Add to effect rack and save state
        effectRack.addEffect(std::move(reverb));
        saveEffectState("Reverb", reverbPtr, position);

        if (onEffectVisibilityChanged)
          onEffectVisibilityChanged("Reverb", reverbPtr, true);
      }
    }
    else
    {
      auto it = effectStates.find("Reverb");
      if (it != effectStates.end())
      {
        if (onEffectVisibilityChanged)
          onEffectVisibilityChanged("Reverb", it->second.processor, false);

        it->second.isEnabled = false;
      }
    }
  };
  addAndMakeVisible(reverbButton.get());
  effectButtons.push_back(std::move(reverbButton));

  // Add Chorus button
  auto chorusButton = std::make_unique<EffectButton>("Chorus");
  chorusButton->onStateChange = [this](bool isActive)
  {
    if (isActive)
    {
      auto it = effectStates.find("Chorus");
      if (it != effectStates.end() && !it->second.isEnabled)
      {
        // Restore existing chorus
        it->second.isEnabled = true;
        if (onEffectVisibilityChanged)
          onEffectVisibilityChanged("Chorus", it->second.processor, true);
      }
      else
      {
        // Create new chorus
        auto chorus = std::make_unique<Chorus>();
        auto *chorusPtr = chorus.get();
        int position = findNextAvailablePosition();

        effectRack.addEffect(std::move(chorus));
        saveEffectState("Chorus", chorusPtr, position);

        if (onEffectVisibilityChanged)
          onEffectVisibilityChanged("Chorus", chorusPtr, true);
      }
    }
    else
    {
      auto it = effectStates.find("Chorus");
      if (it != effectStates.end())
      {
        if (onEffectVisibilityChanged)
          onEffectVisibilityChanged("Chorus", it->second.processor, false);
        it->second.isEnabled = false;
      }
    }
  };
  addAndMakeVisible(chorusButton.get());
  effectButtons.push_back(std::move(chorusButton));

  // Add EQ button
  auto eqButton = std::make_unique<EffectButton>("EQ");
  eqButton->onStateChange = [this](bool isActive)
  {
    if (isActive)
    {
      auto it = effectStates.find("EQ");
      if (it != effectStates.end() && !it->second.isEnabled)
      {
        // Restore existing EQ
        it->second.isEnabled = true;
        if (onEffectVisibilityChanged)
          onEffectVisibilityChanged("EQ", it->second.processor, true);
      }
      else
      {
        // Create new EQ
        auto eq = std::make_unique<Equalizer>();
        auto *eqPtr = eq.get();
        int position = findNextAvailablePosition();

        effectRack.addEffect(std::move(eq));
        saveEffectState("EQ", eqPtr, position);

        if (onEffectVisibilityChanged)
          onEffectVisibilityChanged("EQ", eqPtr, true);
      }
    }
    else
    {
      auto it = effectStates.find("EQ");
      if (it != effectStates.end())
      {
        if (onEffectVisibilityChanged)
          onEffectVisibilityChanged("EQ", it->second.processor, false);
        it->second.isEnabled = false;
      }
    }
  };
  addAndMakeVisible(eqButton.get());
  effectButtons.push_back(std::move(eqButton));

  // Set initial size - this will be overridden by the parent's resized() call
  setSize(200, 600);

  // Call resized to set up initial button positions
  resized();
}

void ToolbarComponent::saveEffectState(const juce::String &effectName, juce::AudioProcessor *processor, int position)
{
  // Check if effect already exists and is enabled
  auto it = effectStates.find(effectName);
  if (it != effectStates.end() && it->second.isEnabled)
  {
    // Effect already exists and is enabled, don't create duplicate
    return;
  }

  ToolbarEffectState state;
  state.processor = processor;
  state.isEnabled = true;
  state.position = position;
  effectStates[effectName] = std::move(state);

  // Update the effect order in the rack using the single source of truth
  int effectIndex = effectRack.findEffectPosition(effectName);
  if (effectIndex >= 0)
  {
    effectRack.setEffectOrder(effectIndex, position);
    effectRack.updateEffectOrder();
  }

  // Notify about the reorder to ensure workspace sync
  if (onEffectReordered)
  {
    onEffectReordered(-1, position); // -1 indicates new effect
  }
}

void ToolbarComponent::restoreEffectState(const juce::String &effectName)
{
  auto it = effectStates.find(effectName);
  if (it != effectStates.end())
  {
    it->second.isEnabled = true;

    // First notify about visibility
    if (onEffectVisibilityChanged)
      onEffectVisibilityChanged(effectName, it->second.processor, true);

    // Update the effect order in the rack using the single source of truth
    int effectIndex = effectRack.findEffectPosition(effectName);
    if (effectIndex >= 0)
    {
      effectRack.setEffectOrder(effectIndex, it->second.position);
      effectRack.updateEffectOrder();
    }

    // Notify about the position using the single source of truth
    if (onEffectReordered)
    {
      onEffectReordered(-1, it->second.position); // -1 indicates restored effect
    }
  }
}

int ToolbarComponent::findNextAvailablePosition() const
{
  // First, find the highest position currently in use
  int maxPosition = -1;
  for (const auto &pair : effectStates)
  {
    if (pair.second.isEnabled && pair.second.position > maxPosition)
      maxPosition = pair.second.position;
  }

  // If we have no enabled effects, start at 0
  if (maxPosition == -1)
    return 0;

  // Check for gaps in the sequence
  std::vector<bool> positions(maxPosition + 1, false);
  for (const auto &pair : effectStates)
  {
    if (pair.second.isEnabled && pair.second.position >= 0)
      positions[pair.second.position] = true;
  }

  // Find the first gap
  for (int i = 0; i < static_cast<int>(positions.size()); ++i)
  {
    if (!positions[i])
      return i;
  }

  // If no gaps found, return the next position
  return maxPosition + 1;
}

ToolbarComponent::~ToolbarComponent()
{
}

void ToolbarComponent::paint(juce::Graphics &g)
{
  // Fill the background with a dark color
  g.fillAll(juce::Colour(40, 40, 40));

  // Debug: Draw insertion points
  if (!effectButtons.empty())
  {
    const float heightRatio = 0.15f; // Match the height ratio used elsewhere
    int buttonHeight = juce::jmax(60, static_cast<int>(getWidth() * heightRatio));
    int spacing = static_cast<int>(buttonHeight * 0.2f);

    g.setColour(juce::Colour(60, 60, 60));
    for (int i = 0; i <= effectButtons.size(); ++i)
    {
      int y = buttonMargin + i * (buttonHeight + spacing);
      g.drawHorizontalLine(y - spacing / 2, 0.0f, static_cast<float>(getWidth()));
    }
  }
}

int ToolbarComponent::getButtonInsertIndex(int yPosition) const
{
  // Get button dimensions
  const float heightRatio = 0.15f; // Match the new height ratio
  int buttonHeight = juce::jmax(60, static_cast<int>(getWidth() * heightRatio));
  int spacing = static_cast<int>(buttonHeight * 0.2f);

  // Calculate which position in the list the button should be inserted
  int index = (yPosition - buttonMargin + spacing / 2) / (buttonHeight + spacing);
  return juce::jlimit(0, static_cast<int>(effectButtons.size()), index);
}

void ToolbarComponent::normalizePositions()
{
  // Check if normalization is needed
  bool needsNormalization = false;
  std::vector<std::pair<juce::String, int>> enabledEffects;

  // First pass: collect enabled effects and check for gaps
  int lastPosition = -1;
  for (const auto &pair : effectStates)
  {
    if (pair.second.isEnabled)
    {
      enabledEffects.emplace_back(pair.first, pair.second.position);
      if (pair.second.position != lastPosition + 1)
      {
        needsNormalization = true;
      }
      lastPosition = pair.second.position;
    }
  }

  // Only proceed with normalization if needed
  if (!needsNormalization)
    return;

  // Sort by current position
  std::sort(enabledEffects.begin(), enabledEffects.end(),
            [](const auto &a, const auto &b)
            { return a.second < b.second; });

  // Assign new sequential positions
  for (size_t i = 0; i < enabledEffects.size(); ++i)
  {
    const auto &effectName = enabledEffects[i].first;
    int newPosition = static_cast<int>(i);

    // Only update if position has changed
    if (effectStates[effectName].position != newPosition)
    {
      effectStates[effectName].position = newPosition;

      // Update rack position using the single source of truth
      int effectIndex = effectRack.findEffectPosition(effectName);
      if (effectIndex >= 0)
      {
        effectRack.setEffectOrder(effectIndex, newPosition);
      }
    }
  }

  // Update rack order
  effectRack.updateEffectOrder();

  // Notify workspace about all position changes
  if (onEffectReordered)
  {
    for (size_t i = 0; i < enabledEffects.size(); ++i)
    {
      const auto &effectName = enabledEffects[i].first;
      int oldPosition = effectStates[effectName].position;
      int newPosition = static_cast<int>(i);

      if (oldPosition != newPosition)
      {
        onEffectReordered(oldPosition, newPosition);
      }
    }
  }
}

void ToolbarComponent::reorderButtons(EffectButton *draggedButton, int newPosition)
{
  // Find the current position of the dragged button
  int currentPosition = -1;
  juce::String effectName;
  for (size_t i = 0; i < effectButtons.size(); ++i)
  {
    if (effectButtons[i].get() == draggedButton)
    {
      currentPosition = static_cast<int>(i);
      effectName = draggedButton->getEffectName();
      break;
    }
  }

  if (currentPosition == -1 || currentPosition == newPosition)
    return;

  // Move the button in the vector
  auto button = std::move(effectButtons[currentPosition]);
  effectButtons.erase(effectButtons.begin() + currentPosition);
  effectButtons.insert(effectButtons.begin() + newPosition, std::move(button));

  // Update positions for all buttons
  updateButtonPositions();

  // Update effectStates positions to match the new visual order
  for (size_t i = 0; i < effectButtons.size(); ++i)
  {
    const auto &button = effectButtons[i];
    auto it = effectStates.find(button->getEffectName());
    if (it != effectStates.end())
    {
      it->second.position = static_cast<int>(i);
    }
  }

  // Update the effect order in the rack using the single source of truth
  int effectIndex = effectRack.findEffectPosition(effectName);
  if (effectIndex >= 0)
  {
    effectRack.setEffectOrder(effectIndex, newPosition);
    effectRack.updateEffectOrder();
  }

  // Notify about the reorder to update workspace
  if (onEffectReordered)
  {
    onEffectReordered(currentPosition, newPosition);
  }
}

void ToolbarComponent::updateButtonPositions()
{
  const int buttonHeight = effectButtons[0]->getHeight();
  const int spacing = static_cast<int>(buttonHeight * 0.2f);
  int y = buttonMargin;

  for (auto &button : effectButtons)
  {
    button->setTargetPosition(y);
    y += buttonHeight + spacing;
  }
}

void ToolbarComponent::resized()
{
  // Calculate button dimensions
  const float heightRatio = 0.15f;
  const float widthRatio = 0.9f;
  int buttonHeight = juce::jmax(60, static_cast<int>(getWidth() * heightRatio));
  int buttonWidth = static_cast<int>(getWidth() * widthRatio);
  int spacing = static_cast<int>(buttonHeight * 0.2f);
  int y = buttonMargin;

  // Update each button's size and position
  for (auto &button : effectButtons)
  {
    // Set button bounds directly
    button->setBounds((getWidth() - buttonWidth) / 2, y, buttonWidth, buttonHeight);
    y += buttonHeight + spacing;
  }
}

std::vector<EffectButton *> ToolbarComponent::getButtonOrder() const
{
  std::vector<EffectButton *> order;
  order.reserve(effectButtons.size());
  for (const auto &button : effectButtons)
  {
    order.push_back(button.get());
  }
  return order;
}
