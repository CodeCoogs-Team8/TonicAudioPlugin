#pragma once

#include <JuceHeader.h>
#include "EffectParameterComponent.h"

class EffectParametersContainer : public juce::Component
{
public:
    EffectParametersContainer();
    ~EffectParametersContainer() override;

    void paint(juce::Graphics &g) override;
    void resized() override;

    // Add a new effect parameter component
    void addEffectParameters(const juce::String &effectName, juce::AudioProcessor *processor);

    // Remove an effect parameter component
    void removeEffectParameters(const juce::String &effectName);

    // Enable/disable an effect
    void setEffectEnabled(const juce::String &effectName, bool shouldBeEnabled);

    // Reorder effect components
    void reorderEffects(int oldPosition, int newPosition);

    // Get the minimum height needed to display all components
    int getMinimumHeight() const;

    // Helper to find component index by name (moved to public)
    int findComponentIndex(const juce::String &effectName) const;

private:
    void updateComponentBounds();

    // Store components with their positions
    struct EffectComponentInfo
    {
        std::unique_ptr<EffectParameterComponent> component;
        int position;
    };

    // Helper to update positions after reordering
    void updatePositions();

    std::vector<EffectComponentInfo> effectComponents;
    std::map<juce::String, int> effectPositions; // Track positions separately

    const float padding = 20.0f;
    const float componentSpacing = 15.0f;
    const float componentHeight = 200.0f;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(EffectParametersContainer)
};