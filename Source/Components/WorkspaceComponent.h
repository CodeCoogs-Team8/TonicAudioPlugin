/*
  ==============================================================================

    WorkSpaceComponent.h
    Created: 15 Apr 2025 9:35:57pm
    Author:  Evan Fraustro

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "EffectParametersContainer.h"

class WorkspaceComponent : public juce::Component
{
public:
  WorkspaceComponent();
  ~WorkspaceComponent() override;

  void paint(juce::Graphics &) override;
  void resized() override;
  bool keyPressed(const juce::KeyPress &key) override;

  // Forward these methods to the effectParameters container
  void addEffectParameters(const juce::String &effectName, juce::AudioProcessor *processor);
  void removeEffectParameters(const juce::String &effectName);
  void setEffectEnabled(const juce::String &effectName, bool shouldBeEnabled);

  // Add method to reorder effects
  void reorderEffects(int oldPosition, int newPosition);

  // Get current position of an effect
  int getEffectPosition(const juce::String &effectName) const;

private:
  std::unique_ptr<juce::Viewport> viewport;
  EffectParametersContainer effectParameters;

  // Track effect positions
  std::map<juce::String, int> effectPositions;

  // Helper method to update positions
  void updateEffectPosition(const juce::String &effectName, int newPosition);

  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(WorkspaceComponent)
};
