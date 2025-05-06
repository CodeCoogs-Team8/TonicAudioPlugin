/*
  ==============================================================================

    ComboBoxComponent.h
    Created: 15 Apr 2025 7:18:45pm
    Author:  Evan Fraustro

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>

class ComboBoxComponent : public juce::Component
{
public:
  ComboBoxComponent();
  ~ComboBoxComponent() override;

  void paint(juce::Graphics &) override;
  void resized() override;

  void addItem(const juce::String &itemText, int itemId) { comboBox.addItem(itemText, itemId); }
  void setSelectedId(int itemId, juce::NotificationType nt = juce::sendNotification) { comboBox.setSelectedId(itemId, nt); }
  juce::ComboBox *getComboBox() { return &comboBox; }

  // Custom LookAndFeel for ComboBox
  class CustomComboBoxLookAndFeel : public juce::LookAndFeel_V4
  {
  public:
    void drawComboBox(juce::Graphics &g, int width, int height, bool isButtonDown,
                      int buttonX, int buttonY, int buttonW, int buttonH, juce::ComboBox &box) override;
    juce::Font getComboBoxFont(juce::ComboBox &) override;
  };

private:
  juce::ComboBox comboBox;
  CustomComboBoxLookAndFeel customLookAndFeel;
};
