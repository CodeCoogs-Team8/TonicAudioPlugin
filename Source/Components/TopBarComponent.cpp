#include "TopBarComponent.h"

TopBarComponent::TopBarComponent()
{
    addAndMakeVisible(levelMeter);

    // Set up the title font
    titleFont.setStyleFlags(juce::Font::bold);
}

TopBarComponent::~TopBarComponent()
{
    // Cancel any pending updates
    cancelPendingUpdate();
}

void TopBarComponent::paint(juce::Graphics &g)
{
    g.fillAll(juce::Colour(0xff1a1a1a));

    // Draw the title text
    g.setFont(titleFont);
    g.setColour(juce::Colours::white);

    // Calculate text bounds to ensure it doesn't overlap with the level meter
    auto bounds = getLocalBounds();
    auto meterBounds = levelMeter.getBounds();
    auto textBounds = bounds.withTrimmedRight(meterBounds.getWidth() + 20); // Extra padding between text and meter

    g.drawText(title, textBounds.withTrimmedLeft(leftPadding),
               juce::Justification::centredLeft, true);
}

void TopBarComponent::resized()
{
    auto bounds = getLocalBounds();

    // Calculate level meter dimensions
    const int meterHeight = getHeight() * 0.12;         // 12% of height
    const int meterWidth = getWidth() * 0.2;            // 20% of width
    const int meterY = (getHeight() - meterHeight) / 2; // Center vertically
    const int rightPadding = 10;                        // Padding from right edge

    // Position level meter in top right with padding
    levelMeter.setBounds(bounds.removeFromRight(meterWidth + rightPadding)
                             .withTrimmedRight(rightPadding)
                             .withHeight(meterHeight)
                             .withY(meterY));
}

void TopBarComponent::setLevels(float leftLevel, float rightLevel)
{
    // Store the levels
    pendingLeftLevel = leftLevel;
    pendingRightLevel = rightLevel;

    // Trigger an async update if one isn't already pending
    triggerAsyncUpdate();
}

void TopBarComponent::handleAsyncUpdate()
{
    // This will be called on the message thread
    levelMeter.setLevels(pendingLeftLevel, pendingRightLevel);
}
