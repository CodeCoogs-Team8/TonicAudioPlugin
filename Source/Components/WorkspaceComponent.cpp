/*
  ==============================================================================

    WorkspaceComponent.cpp
    Created: 15 Apr 2025 9:35:57pm
    Author:  Evan Fraustro

  ==============================================================================
*/

#include "WorkspaceComponent.h"

WorkspaceComponent::WorkspaceComponent()
{
  setOpaque(true);

  // Create and set up the viewport
  viewport = std::make_unique<juce::Viewport>();
  viewport->setViewedComponent(&effectParameters, false);
  viewport->setScrollBarsShown(true, false);
  viewport->setScrollOnDragEnabled(true);
  viewport->setSingleStepSizes(20, 20);
  viewport->setScrollBarThickness(12);

  addAndMakeVisible(viewport.get());
  addAndMakeVisible(effectParameters);
}

WorkspaceComponent::~WorkspaceComponent()
{
  viewport->setViewedComponent(nullptr, false);
}

void WorkspaceComponent::paint(juce::Graphics &g)
{
  g.fillAll(juce::Colour(30, 30, 30));
}

void WorkspaceComponent::resized()
{
  // Set viewport to fill the entire component
  viewport->setBounds(getLocalBounds());

  // Calculate the content height based on the number of effects
  const int minHeight = getHeight();
  const int contentHeight = juce::jmax(minHeight, effectParameters.getMinimumHeight());

  // Set the effect parameters container size, accounting for scrollbar width
  effectParameters.setBounds(0, 0,
                             viewport->getWidth() - viewport->getScrollBarThickness(),
                             contentHeight);
}

bool WorkspaceComponent::keyPressed(const juce::KeyPress &key)
{
  // Add keyboard shortcuts for scrolling if needed
  return false;
}

void WorkspaceComponent::addEffectParameters(const juce::String &effectName, juce::AudioProcessor *processor)
{
  // If we have a stored position, use it, otherwise use the next available position
  int position = effectPositions.count(effectName) > 0 ? effectPositions[effectName] : static_cast<int>(effectPositions.size());

  effectParameters.addEffectParameters(effectName, processor);
  updateEffectPosition(effectName, position);
  resized();
}

void WorkspaceComponent::removeEffectParameters(const juce::String &effectName)
{
  // Store the position before removal
  int position = getEffectPosition(effectName);
  if (position >= 0)
  {
    effectPositions[effectName] = position; // Keep position for later restoration
  }

  effectParameters.removeEffectParameters(effectName);
  resized();
}

void WorkspaceComponent::setEffectEnabled(const juce::String &effectName, bool shouldBeEnabled)
{
  effectParameters.setEffectEnabled(effectName, shouldBeEnabled);

  if (!shouldBeEnabled)
  {
    // Store position when disabling
    int position = getEffectPosition(effectName);
    if (position >= 0)
    {
      effectPositions[effectName] = position;
    }
  }

  resized();
}

void WorkspaceComponent::reorderEffects(int oldPosition, int newPosition)
{
  effectParameters.reorderEffects(oldPosition, newPosition);

  // Update positions map after reordering
  for (const auto &pair : effectPositions)
  {
    int currentPos = getEffectPosition(pair.first);
    if (currentPos >= 0)
    {
      effectPositions[pair.first] = currentPos;
    }
  }

  resized();
}

int WorkspaceComponent::getEffectPosition(const juce::String &effectName) const
{
  return effectParameters.findComponentIndex(effectName);
}

void WorkspaceComponent::updateEffectPosition(const juce::String &effectName, int newPosition)
{
  effectPositions[effectName] = newPosition;

  // If the current position differs from the desired position, reorder
  int currentPosition = getEffectPosition(effectName);
  if (currentPosition >= 0 && currentPosition != newPosition)
  {
    effectParameters.reorderEffects(currentPosition, newPosition);
  }
}
