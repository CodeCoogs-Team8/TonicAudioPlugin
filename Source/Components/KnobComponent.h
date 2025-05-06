#pragma once

#include <JuceHeader.h>

class RotaryKnob : public juce::Component
{
public:
  RotaryKnob(const juce::String &text,
             juce::AudioProcessorValueTreeState &apvts,
             const juce::ParameterID &parameterID,
             bool drawFromMiddle = false);

  ~RotaryKnob() override;

  void resized() override;

  juce::Slider slider;
  juce::Label label;

  juce::AudioProcessorValueTreeState::SliderAttachment attachment;
  float getValue() const
  {
    return static_cast<float>(slider.getValue());
  }
  void setValue(float newValue)
  {
    slider.setValue(static_cast<double>(newValue), juce::dontSendNotification);
  }

private:
  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(RotaryKnob)
};
