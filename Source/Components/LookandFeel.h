/*
  ==============================================================================

    LookandFeel.h
    Created: 5 May 2025 8:19:49pm
    Author:  Evan Fraustro

  ==============================================================================
*/

#pragma once


#include <JuceHeader.h>

namespace Colors
{
    const juce::Colour background { 245, 240, 235 };
    const juce::Colour header { 40, 40, 40 };

    namespace Knob
    {
        const juce::Colour trackBackground { 100, 100, 100 };
        const juce::Colour trackActive { 148, 101, 211 };
        const juce::Colour outline { 45, 50, 50 };
        const juce::Colour gradientTop { 35, 35, 35 };
        const juce::Colour gradientBottom { 40, 40, 40 };
        const juce::Colour dial { 100, 100, 100 };
        const juce::Colour dropShadow { 30, 30, 30 };
        const juce::Colour label { 80, 80, 80 };
        const juce::Colour textBoxBackground { 80, 80, 80 };
        const juce::Colour value { 240, 240, 240 };
        const juce::Colour caret { 10, 15, 10 };
    }

    namespace Group
    {
        const juce::Colour label { 160, 155, 150 };
        const juce::Colour outline { 235, 230, 225 };
    }
}

class Fonts
{
public:
    Fonts() = delete;

    static juce::Font getFont(float height = 16.0f);

private:
    static const juce::Typeface::Ptr typeface;
};

class RotaryKnobLookAndFeel : public juce::LookAndFeel_V4
{
public:
    RotaryKnobLookAndFeel();

    static RotaryKnobLookAndFeel* get()
    {
        static RotaryKnobLookAndFeel instance;
        return &instance;
    }

    void drawRotarySlider(juce::Graphics& g, int x, int y, int width, int height,
                          float sliderPos, float rotaryStartAngle,
                          float rotaryEndAngle, juce::Slider& slider) override;

    juce::Font getLabelFont(juce::Label&) override;
    juce::Label* createSliderTextBox(juce::Slider&) override;

    void drawTextEditorOutline(juce::Graphics&, int, int, juce::TextEditor&) override { }
    void fillTextEditorBackground(juce::Graphics&, int width, int height, juce::TextEditor&) override;

private:
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(RotaryKnobLookAndFeel)

    juce::DropShadow dropShadow { Colors::Knob::dropShadow, 6, { 0, 3 } };
};

class MainLookAndFeel : public juce::LookAndFeel_V4
{
public:
    MainLookAndFeel();

    juce::Font getLabelFont(juce::Label&) override;

private:
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MainLookAndFeel)
};
