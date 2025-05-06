#pragma once

#include <JuceHeader.h>

class HorizontalLevelMeter : public juce::Component,
                             public juce::Timer
{
public:
    HorizontalLevelMeter();
    ~HorizontalLevelMeter() override;

    void paint(juce::Graphics &) override;
    void resized() override;
    void timerCallback() override;

    void setLevels(float leftLevel, float rightLevel);
    void setMeterColour(juce::Colour newColour);

private:
    void drawMeterBar(juce::Graphics &g, const juce::Rectangle<float> &bounds, float level, float peakLevel);

    float leftLevel = 0.0f;
    float rightLevel = 0.0f;
    float leftPeakLevel = 0.0f;
    float rightPeakLevel = 0.0f;

    juce::Colour meterColour = juce::Colour(0xff00ffff);      // Light blue
    juce::Colour peakColour = juce::Colour(0xffff0000);       // Red
    juce::Colour backgroundColour = juce::Colour(0xff333333); // Dark gray

    const float cornerSize = 2.0f;
    const float meterGap = 2.0f; // Gap between left and right meters

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(HorizontalLevelMeter)
};