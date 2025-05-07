#include "TopBarComponent.h"

TopBarComponent::TopBarComponent()
{
    addAndMakeVisible(levelMeter);

    // Set up the title font
    titleFont.setStyleFlags(juce::Font::bold);

    // Create gain controls
    createGainControls();
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

    // Calculate gain controls section dimensions
    const int knobWidth = 45;                                 // Width of each knob (matching RotaryKnob default)
    const int knobHeight = 57;                                // Height of each knob (matching RotaryKnob default)
    const int knobSpacing = 20;                               // Space between knobs
    const int totalGainWidth = (knobWidth * 2) + knobSpacing; // Total width needed for knobs
    const int gainControlsPadding = 20;                       // Padding between knobs and meter

    // Position level meter first
    auto meterBounds = bounds.removeFromRight(meterWidth + rightPadding)
                           .withTrimmedRight(rightPadding)
                           .withHeight(meterHeight)
                           .withY(meterY);
    levelMeter.setBounds(meterBounds);

    // Position gain controls to the left of the meter
    auto gainControlsBounds = bounds.removeFromRight(totalGainWidth + gainControlsPadding)
                                  .withTrimmedRight(gainControlsPadding)
                                  .withHeight(getHeight())
                                  .withY(0);

    // Center the knobs within the gain controls section
    auto knobArea = gainControlsBounds.withSizeKeepingCentre(totalGainWidth, getHeight());

    // Position input gain knob
    auto inputBounds = knobArea.removeFromLeft(knobWidth);
    inputGainKnob->setBounds(inputBounds);

    // Add spacing
    knobArea.removeFromLeft(knobSpacing);

    // Position output gain knob
    auto outputBounds = knobArea.removeFromLeft(knobWidth);
    outputGainKnob->setBounds(outputBounds);
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

void TopBarComponent::createGainControls()
{
    // Create input gain knob
    inputGainKnob = std::make_unique<RotaryKnob>(
        "Input",
        inputGainProcessor.getAPVTS(),
        juce::ParameterID("input_gain", 1),
        false, // drawFromMiddle
        45,    // width - using default from RotaryKnob
        57);   // height - using default from RotaryKnob
    addAndMakeVisible(inputGainKnob.get());

    // Create output gain knob
    outputGainKnob = std::make_unique<RotaryKnob>(
        "Output",
        outputGainProcessor.getAPVTS(),
        juce::ParameterID("output_gain", 1),
        false, // drawFromMiddle
        45,    // width - using default from RotaryKnob
        57);   // height - using default from RotaryKnob
    addAndMakeVisible(outputGainKnob.get());
}
