#pragma once

#include <JuceHeader.h>
#include "LookandFeel.h"

class RotaryKnob : public juce::Component
{
public:
  /** Creates a rotary knob with customizable dimensions.
   * @param text          The label text for the knob
   * @param apvts         The AudioProcessorValueTreeState to attach to
   * @param parameterID   The parameter ID for this knob
   * @param drawFromMiddle Whether to draw the indicator from the middle
   * @param width         The width of the knob (default: 70)
   * @param height        The total height including label (default: 86)
   */
  RotaryKnob(const juce::String &text,
             juce::AudioProcessorValueTreeState &apvts,
             const juce::ParameterID &parameterID,
             bool drawFromMiddle = false,
             int width = 70,
             int height = 86);

  ~RotaryKnob() override;

  void resized() override;
  void paint(juce::Graphics &g) override;

  float getValue() const
  {
    return slider.getValue();
  }
  void setValue(float value)
  {
    slider.setValue(value);
  }

  /** Sets new dimensions for the knob and updates the layout */
  void setKnobSize(int newWidth, int newHeight);

  /** Gets the current width of the knob */
  int getKnobWidth() const { return knobWidth; }

  /** Gets the current height of the knob */
  int getKnobHeight() const { return knobHeight; }

private:
  void updateKnobLayout();

  juce::Slider slider;
  juce::Label label;
  juce::AudioProcessorValueTreeState::SliderAttachment attachment;
  bool drawFromMiddle;
  int knobWidth;
  int knobHeight;

  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(RotaryKnob)
};
