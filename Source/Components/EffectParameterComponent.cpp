#include "EffectParameterComponent.h"
#include "KnobComponent.h"
#include "../Effects/Delay.h"
#include "../Effects/Distortion.h"
#include "../Effects/Reverb.h"
#include "../Effects/Chorus.h"
#include "../Effects/Equalizer.h"

EffectParameterComponent::EffectParameterComponent(const juce::String &effectName, juce::AudioProcessor *processor)
    : name(effectName), audioProcessor(processor)
{
    // Create enable button
    enableButton = std::make_unique<juce::ToggleButton>();
    enableButton->setToggleState(true, juce::dontSendNotification);
    enableButton->onClick = [this]
    {
        setEnabled(enableButton->getToggleState());
    };
    addAndMakeVisible(enableButton.get());

    // Create parameter controls
    createParameterControls();
}

EffectParameterComponent::~EffectParameterComponent()
{
}

void EffectParameterComponent::paint(juce::Graphics &g)
{
    auto bounds = getLocalBounds().toFloat();

    // Draw background with rounded corners
    g.setColour(juce::Colour(45, 45, 45).withAlpha(opacity));
    g.fillRoundedRectangle(bounds, cornerSize);

    // Draw edge highlight
    g.setColour(juce::Colour(70, 70, 70).withAlpha(opacity));
    g.drawRoundedRectangle(bounds.reduced(0.5f), cornerSize, 1.0f);

    // Draw effect name vertically
    g.setFont(20.0f);
    g.setColour(juce::Colours::white.withAlpha(opacity));

    // calculate vertical center for the label
    float labelAreaHeight = bounds.getHeight() - 2 * padding;
    float labelAreaY = padding;

    // Set up the transform for vertical text, centered
    auto transform = juce::AffineTransform()
                         .rotated(-juce::MathConstants<float>::halfPi)
                         .translated(padding - 37.5f, bounds.getHeight() / 2.0f + (nameHeight / 1.5f));

    g.addTransform(transform);
    g.drawText(name, 0, 0, static_cast<int>(labelAreaHeight), static_cast<int>(nameHeight), juce::Justification::centred, true);
    g.resetToDefaultState();
}

bool EffectParameterComponent::shouldUseKnobs() const
{
    return true; // Use knobs for all effects
}

void EffectParameterComponent::resized()
{
    auto bounds = getLocalBounds();

    // Position enable button in top-right corner
    enableButton->setBounds(bounds.getRight() - 40,
                            bounds.getY() + static_cast<int>(padding),
                            30, 30);

    // Update parameter control positions
    updateParameterComponentBounds();
}

void EffectParameterComponent::setEnabled(bool shouldBeEnabled)
{
    enabled = shouldBeEnabled;
    opacity = enabled ? 1.0f : 0.5f;

    for (auto &control : parameterControls)
    {
        control->setEnabled(enabled);
        control->setAlpha(opacity);
    }

    for (auto &label : parameterLabels)
    {
        label->setEnabled(enabled);
        label->setAlpha(opacity);
    }

    for (auto &valueLabel : parameterValueLabels)
    {
        valueLabel->setEnabled(enabled);
        valueLabel->setAlpha(opacity);
    }

    // Update the processor if available
    if (auto *processor = getProcessor())
    {
        // TODO: Implement effect bypass in the processor
    }

    repaint();
}

void EffectParameterComponent::createParameterControls()
{
    if (auto *processor = getProcessor())
    {
        const auto &params = processor->getParameters();
        const bool useKnobs = shouldUseKnobs();

        for (auto *param : params)
        {
            if (auto *parameter = dynamic_cast<juce::AudioProcessorParameter *>(param))
            {
                std::unique_ptr<juce::Component> control;

                if (useKnobs)
                {
                    // Try to get APVTS from the processor
                    auto *apvts = [&]() -> juce::AudioProcessorValueTreeState *
                    {
                        if (auto *delay = dynamic_cast<Delay *>(processor))
                            return &delay->parameters;
                        if (auto *distortion = dynamic_cast<Distortion *>(processor))
                            return &distortion->parameters;
                        if (auto *reverb = dynamic_cast<Reverb *>(processor))
                            return &reverb->parameters;
                        if (auto *chorus = dynamic_cast<Chorus *>(processor))
                            return &chorus->parameters;
                        if (auto *eq = dynamic_cast<Equalizer *>(processor))
                            return &eq->parameters;
                        // Add similar lines for other effect types with APVTS
                        return nullptr;
                    }();

                    if (apvts)
                    {
                        if (auto *paramWithID = dynamic_cast<juce::AudioProcessorParameterWithID *>(parameter))
                        {
                            auto rotaryKnob = std::make_unique<RotaryKnob>(
                                paramWithID->getName(100),
                                *apvts,
                                paramWithID->paramID,
                                false);
                            addAndMakeVisible(rotaryKnob.get());
                            control = std::move(rotaryKnob);
                        }
                    }
                }
                else
                {
                    auto slider = std::make_unique<juce::Slider>();
                    slider->setSliderStyle(juce::Slider::LinearVertical);
                    slider->setTextBoxStyle(juce::Slider::NoTextBox, true, 0, 0);
                    slider->setRange(0.0f, 1.0f, 0.01f);
                    slider->setValue(parameter->getValue(), juce::dontSendNotification);

                    // Create value label
                    auto valueLabel = std::make_unique<juce::Label>();
                    valueLabel->setJustificationType(juce::Justification::centred);
                    valueLabel->setFont(juce::Font(12.0f));
                    valueLabel->setColour(juce::Label::textColourId, juce::Colours::lightblue);
                    valueLabel->setVisible(true);
                    auto *valueLabelPtr = valueLabel.get();

                    slider->onValueChange = [parameter, valueLabelPtr, slider = slider.get()]
                    {
                        parameter->setValueNotifyingHost(static_cast<float>(slider->getValue()));
                        valueLabelPtr->setText(parameter->getCurrentValueAsText(), juce::dontSendNotification);
                    };

                    parameterValueLabels.push_back(std::move(valueLabel));
                    control = std::move(slider);
                }

                // Remove creation of parameter name label (handled by RotaryKnob)
                auto *controlPtr = control.get();
                addAndMakeVisible(controlPtr);
                if (!useKnobs) // Only add value label for sliders
                    addAndMakeVisible(parameterValueLabels.back().get());

                parameterControls.push_back(std::move(control));
                // Do not push label to parameterLabels
            }
        }
    }
}

void EffectParameterComponent::updateParameterComponentBounds()
{
    auto bounds = getLocalBounds();

    // Reserve space for the name on the left and padding
    bounds.removeFromLeft(static_cast<int>(padding * 3.0f + 20.0f));
    bounds.removeFromRight(static_cast<int>(padding));
    bounds.removeFromTop(static_cast<int>(padding));
    bounds.removeFromBottom(static_cast<int>(padding));

    if (parameterControls.empty())
        return;

    // Calculate width for each control
    const int numControls = static_cast<int>(parameterControls.size());
    const int totalSpacing = static_cast<int>(controlSpacing) * (numControls - 1);
    const int controlWidth = juce::jmin(100, (bounds.getWidth() - totalSpacing) / numControls);

    // Only use knobHeight for layout, since RotaryKnob draws its own label and value
    const int knobHeight = controlWidth; // Knob is square
    const int textHeight = 24;
    const int groupHeight = knobHeight + textHeight + 4;

    // Vertically center the group
    int y = bounds.getY() + (bounds.getHeight() - groupHeight) / 2;
    int x = bounds.getX();

    for (size_t i = 0; i < parameterControls.size(); ++i)
    {
        // Only set bounds for the control (knob/slider)
        auto controlBounds = juce::Rectangle<int>(x, y, controlWidth, groupHeight);
        parameterControls[i]->setBounds(controlBounds);
        x += controlWidth + controlSpacing;
    }
}

void EffectParameterComponent::parameterValueChanged(int, float newValue)
{
    for (auto &control : parameterControls)
    {
        if (auto *slider = dynamic_cast<juce::Slider *>(control.get()))
        {
            if (slider->getValue() == newValue)
            {
                slider->setValue(newValue, juce::dontSendNotification);
                break;
            }
        }
        else if (auto *knob = dynamic_cast<RotaryKnob *>(control.get()))
        {
            if (knob->getValue() == newValue)
            {
                knob->setValue(newValue);
                break;
            }
        }
    }
}

void EffectParameterComponent::parameterGestureChanged(int, bool)
{
    // Not needed for this implementation
}
