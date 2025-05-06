#include "HorizontalLevelMeter.h"

HorizontalLevelMeter::HorizontalLevelMeter()
{
    startTimerHz(30); // Update at 30Hz
}

HorizontalLevelMeter::~HorizontalLevelMeter()
{
    stopTimer();
}

void HorizontalLevelMeter::paint(juce::Graphics &g)
{
    auto bounds = getLocalBounds().toFloat();
    float meterHeight = (bounds.getHeight() - meterGap) / 2.0f;

    // Top half for left channel
    juce::Rectangle<float> leftMeterBounds = bounds.removeFromTop(meterHeight);
    drawMeterBar(g, leftMeterBounds, leftLevel, leftPeakLevel);

    // Bottom half for right channel
    juce::Rectangle<float> rightMeterBounds = bounds.withTrimmedTop(meterGap);
    drawMeterBar(g, rightMeterBounds, rightLevel, rightPeakLevel);
}

void HorizontalLevelMeter::drawMeterBar(juce::Graphics &g, const juce::Rectangle<float> &bounds, float level, float peakLevel)
{
    const float cornerSize = 2.0f;

    // Draw background
    g.setColour(backgroundColour);
    g.fillRoundedRectangle(bounds, cornerSize);

    // Draw level meter
    auto levelWidth = bounds.getWidth() * level;
    if (levelWidth > 0)
    {
        g.setColour(meterColour);
        g.fillRoundedRectangle(bounds.withWidth(levelWidth), cornerSize);
    }

    // Draw peak marker
    auto peakX = bounds.getWidth() * peakLevel;
    if (peakX > 0)
    {
        g.setColour(peakColour);
        float peakWidth = 2.0f;
        g.fillRect(bounds.getX() + peakX - peakWidth / 2, bounds.getY(), peakWidth, bounds.getHeight());
    }
}

void HorizontalLevelMeter::resized()
{
    // No specific resizing needed
}

void HorizontalLevelMeter::timerCallback()
{
    // Decay peaks
    const float decayRate = 0.002f;

    if (leftPeakLevel > leftLevel)
        leftPeakLevel = std::max(leftLevel, leftPeakLevel - decayRate);

    if (rightPeakLevel > rightLevel)
        rightPeakLevel = std::max(rightLevel, rightPeakLevel - decayRate);

    repaint();
}

void HorizontalLevelMeter::setLevels(float newLeftLevel, float newRightLevel)
{
    // Ensure levels are between 0 and 1
    leftLevel = juce::jlimit(0.0f, 1.0f, newLeftLevel);
    rightLevel = juce::jlimit(0.0f, 1.0f, newRightLevel);

    // Update peak levels
    leftPeakLevel = std::max(leftPeakLevel, leftLevel);
    rightPeakLevel = std::max(rightPeakLevel, rightLevel);

    repaint();
}

void HorizontalLevelMeter::setMeterColour(juce::Colour newColour)
{
    meterColour = newColour;
    repaint();
}