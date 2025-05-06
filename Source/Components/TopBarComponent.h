#pragma once

#include <JuceHeader.h>
#include "HorizontalLevelMeter.h"

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

    HorizontalLevelMeter levelMeter;
    float pendingLeftLevel = 0.0f;
    float pendingRightLevel = 0.0f;

    juce::Font titleFont{24.0f}; // Font for the title
    const juce::String title{"Tonic"};
    const int leftPadding = 15; // Padding from left edge

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(TopBarComponent)
};