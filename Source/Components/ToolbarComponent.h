/*
  ===========================================================================

    ToolbarComponen
    Created: 15 Apr 2025 7:19:2
    Author:  Evan Fraus

  ===========================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "EffectButton.h"
#include "../Effects/EffectRack.h"

class ToolbarComponent : public juce::Component
{
public:
  ToolbarComponent(EffectRack &rack);
  ~ToolbarComponent() override;

  void paint(juce::Graphics &) override;
  void resized() override;

  // Callback for effect visibility change
  std::function<void(const juce::String &, juce::AudioProcessor *, bool)> onEffectVisibilityChanged;

  // Callback for effect reordering
  std::function<void(int, int)> onEffectReordered;

  // Public methods for drag-and-drop reorderin
  int getButtonInsertIndex(int yPosition) const;
  void reorderButtons(EffectButton *draggedButton, int newPosition);
  void updateButtonPositions();
  int getNumButtons() const { return static_cast<int>(effectButtons.size()); }
  std::vector<EffectButton *> getButtonOrder() const;

  // Make buttonMargin accessibl
  const int buttonMargin = 20; // Top margin for first butto

  // Helper methods for effect managemen
  void saveEffectState(const juce::String &effectName, juce::AudioProcessor *processor, int position);
  void restoreEffectState(const juce::String &effectName);
  int findNextAvailablePosition() const;
  void normalizePositions();

private:
  struct ToolbarEffectState
  {
    bool isEnabled = false;
    juce::AudioProcessor *processor = nullptr;
    int position = -1;
  };

  std::vector<std::unique_ptr<EffectButton>> effectButtons;
  std::map<juce::String, ToolbarEffectState> effectStates;

  const float toolbarWidthRatio = 0.2f; // Toolbar takes 20% of parent widt
  int buttonSpacing = 10;               // Will be calculated based on button heigh
  float cornerSize = 8.0f;
  EffectRack &effectRack;

  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ToolbarComponent)
};
