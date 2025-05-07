#include <JuceHeader.h>
#include "./KnobComponent.h"
#include "LookAndFeel.h"

RotaryKnob::RotaryKnob(const juce::String &text,
                       juce::AudioProcessorValueTreeState &apvts,
                       const juce::ParameterID &parameterID,
                       bool drawFromMiddle,
                       int width,
                       int height)
    : attachment(apvts, parameterID.getParamID(), slider),
      drawFromMiddle(drawFromMiddle),
      knobWidth(width),
      knobHeight(height)
{
  slider.setSliderStyle(juce::Slider::SliderStyle::RotaryHorizontalVerticalDrag);
  slider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, width, 16);

  label.setText(text, juce::NotificationType::dontSendNotification);
  label.setJustificationType(juce::Justification::horizontallyCentred);
  label.setBorderSize(juce::BorderSize<int>{0, 0, 2, 0});
  label.attachToComponent(&slider, false);

  addAndMakeVisible(slider);
  addAndMakeVisible(label);

  setLookAndFeel(RotaryKnobLookAndFeel::get());

  float pi = juce::MathConstants<float>::pi;
  slider.setRotaryParameters(1.25f * pi, 2.75f * pi, true);
  slider.getProperties().set("drawFromMiddle", drawFromMiddle);

  updateKnobLayout();
}

RotaryKnob::~RotaryKnob()
{
}

void RotaryKnob::updateKnobLayout()
{
  // Set bounds for the slider
  slider.setBounds(0, 0, knobWidth, knobHeight);

  // Calculate total height including label (24px for label space)
  const int totalHeight = knobHeight + 24;
  setSize(knobWidth, totalHeight);

  // Position the slider
  slider.setTopLeftPosition(0, 24);
}

void RotaryKnob::setKnobSize(int newWidth, int newHeight)
{
  knobWidth = newWidth;
  knobHeight = newHeight;
  updateKnobLayout();
}

void RotaryKnob::resized()
{
  updateKnobLayout();
}

void RotaryKnob::paint(juce::Graphics &g)
{
  // The actual painting is handled by the LookAndFeel
}
