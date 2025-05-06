#pragma once

#include <JuceHeader.h>
#include "KnobComponent.h"
#include <juce_audio_processors/juce_audio_processors.h>

class EffectParameterComponent : public juce::Component,
                                 public juce::AudioProcessorParameter::Listener
{
public:
    EffectParameterComponent(const juce::String &effectName, juce::AudioProcessor *processor);
    ~EffectParameterComponent() override;

    void paint(juce::Graphics &g) override;
    void resized() override;

    // Enable/disable the effect
    void setEnabled(bool shouldBeEnabled);
    bool isEnabled() const { return enabled; }

    // Get the effect name
    const juce::String &getEffectName() const { return name; }

    // Get processor
    juce::AudioProcessor *getProcessor() const { return audioProcessor; }

    // AudioProcessorParameter::Listener methods
    void parameterValueChanged(int parameterIndex, float newValue) override;
    void parameterGestureChanged(int parameterIndex, bool gestureIsStarting) override;

private:
    void createParameterControls();
    void updateParameterComponentBounds();
    bool shouldUseKnobs() const;

    juce::String name;
    juce::AudioProcessor *audioProcessor;
    std::vector<std::unique_ptr<juce::Component>> parameterControls;
    std::vector<std::unique_ptr<juce::Label>> parameterLabels;
    std::vector<std::unique_ptr<juce::Label>> parameterValueLabels;
    std::unique_ptr<juce::ToggleButton> enableButton;

    bool enabled = true;
    float opacity = 1.0f;
    const float cornerSize = 10.0f;
    const float padding = 15.0f;
    const float nameHeight = 120.0f;
    const float controlSpacing = 10.0f;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(EffectParameterComponent)
};