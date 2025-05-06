/*
  =============================================================================

    This file contains the basic framework code for a JUCE plugin editor

  =============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
DelayAudioProcessorEditor::DelayAudioProcessorEditor(DelayAudioProcessor &p)
    : AudioProcessorEditor(&p), audioProcessor(p),
      toolbar(p.getEffectRack()),
      workspaceArea(),
      topBar()
{
    setOpaque(true);
    addAndMakeVisible(toolbar);
    addAndMakeVisible(workspaceArea);
    addAndMakeVisible(topBar);

    // Set up the callback for effect visibility changes
    toolbar.onEffectVisibilityChanged = [this](const juce::String &effectName,
                                               juce::AudioProcessor *processor,
                                               bool isVisible)
    {
        if (isVisible)
            workspaceArea.addEffectParameters(effectName, processor);
        else
            workspaceArea.removeEffectParameters(effectName);
    };

    // Set up the callback for effect reordering
    toolbar.onEffectReordered = [this](int oldPosition, int newPosition)
    {
        // Update the workspace component's effect order
        workspaceArea.reorderEffects(oldPosition, newPosition);
    };

    // Start the timer to update level meters
    startTimerHz(30); // Update at 30Hz

    // Make sure our window is big enough to fit our components
    setSize(1200, 800);
}

DelayAudioProcessorEditor::~DelayAudioProcessorEditor()
{
    stopTimer();
}

//==============================================================================
void DelayAudioProcessorEditor::paint(juce::Graphics &g)
{
    g.fillAll(juce::Colour(0xff1a1a1a));
}

void DelayAudioProcessorEditor::resized()
{
    auto bounds = getLocalBounds();

    // Top bar takes 10% of height
    int topBarHeight = static_cast<int>(getHeight() * 0.1);
    topBar.setBounds(bounds.removeFromTop(topBarHeight));

    // Toolbar takes 20% of width
    int toolbarWidth = static_cast<int>(getWidth() * 0.2);
    toolbar.setBounds(bounds.removeFromLeft(toolbarWidth));

    // Workspace takes the remaining space
    workspaceArea.setBounds(bounds);
}

void DelayAudioProcessorEditor::setOutputLevel(float leftLevel, float rightLevel)
{
    topBar.setLevels(leftLevel, rightLevel);
}

void DelayAudioProcessorEditor::timerCallback()
{
    // Get the current levels from the processor
    auto levels = audioProcessor.getCurrentLevels();

    // Update the level meters
    setOutputLevel(levels.leftLevel, levels.rightLevel);
}
