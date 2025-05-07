#pragma once

#include <JuceHeader.h>

#include "HorizontalLevelMeter.h"
#include "KnobComponent.h"
#include "../Effects/GainProcessor.h"

class TopBarComponent : public juce::Component,
                        private juce::AsyncUpdater
{
public:
    TopBarComponent();
    ~TopBarComponent() override;

    void paint(juce::Graphics &) override;
    void resized() override;

    void setLevel(float newLevel) { setLevels(newLevel, newLevel); }
    void setLevels(float leftLevel, float rightLevel);

private:
    void handleAsyncUpdate() override;
    void createGainControls();

    HorizontalLevelMeter levelMeter;
    float pendingLeftLevel = 0.0f;
    float pendingRightLevel = 0.0f;

    juce::Font titleFont{24.0f}; // Font for the title
    const juce::String title{"Tonic"};
    const int leftPadding = 15; // Padding from left edge

    // Gain controls
    std::unique_ptr<RotaryKnob> inputGainKnob;
    std::unique_ptr<RotaryKnob> outputGainKnob;

    // Gain processors
    GainProcessor inputGainProcessor{true};   // true for input gain
    GainProcessor outputGainProcessor{false}; // false for output gain

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(TopBarComponent)
};
