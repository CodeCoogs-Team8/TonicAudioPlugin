/*
  ==============================================================================

    ComboBoxComponent.cpp
    Created: 15 Apr 2025 7:18:45pm
    Author:  Evan Fraustro

  ==============================================================================
*/

#include "ComboBoxComponent.h"

ComboBoxComponent::ComboBoxComponent()
{
  comboBox.setLookAndFeel(&customLookAndFeel);
  addAndMakeVisible(comboBox);
}

ComboBoxComponent::~ComboBoxComponent()
{
  comboBox.setLookAndFeel(nullptr);
}

void ComboBoxComponent::paint(juce::Graphics &g)
{
  // Optionally draw a background or border for the component
}

void ComboBoxComponent::resized()
{
  comboBox.setBounds(getLocalBounds());
}

void ComboBoxComponent::CustomComboBoxLookAndFeel::drawComboBox(juce::Graphics &g, int width, int height, bool isButtonDown,
                                                                int buttonX, int buttonY, int buttonW, int buttonH, juce::ComboBox &box)
{
  // Simple custom look: dark background, purple border
  g.setColour(juce::Colour(40, 40, 40));
  g.fillRoundedRectangle(0, 0, width, height, 6.0f);
  g.setColour(juce::Colour(148, 0, 211));
  g.drawRoundedRectangle(0, 0, width, height, 6.0f, 2.0f);
}

juce::Font ComboBoxComponent::CustomComboBoxLookAndFeel::getComboBoxFont(juce::ComboBox &)
{
  return juce::Font(16.0f, juce::Font::bold);
}
