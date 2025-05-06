/*
  =============================================================================

    This file contains the basic framework code for a JUCE plugin editor

  =============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"
#include "./Components/ToolbarComponent.h"
#include "./Components/WorkspaceComponent.h"
#include "./Components/TopBarComponent.h"

//==============================================================================
class DelayAudioProcessorEditor : public juce::AudioProcessorEditor,
                                  private juce::Timer
{
public:
  explicit DelayAudioProcessorEditor(DelayAudioProcessor &);
  ~DelayAudioProcessorEditor() override;

  //==============================================================================
  void paint(juce::Graphics &) override;
  void resized() override;

  void setOutputLevel(float leftLevel, float rightLevel);

private:
  void timerCallback() override;

  // This reference is provided as a quick way for your editor to
  // access the processor object that created it.
  DelayAudioProcessor &audioProcessor;
  ToolbarComponent toolbar;
  WorkspaceComponent workspaceArea;
  TopBarComponent topBar;

  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(DelayAudioProcessorEditor)
};
