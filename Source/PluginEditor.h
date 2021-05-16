/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"

//==============================================================================
/**
*/
class ChorusPluginAudioProcessorEditor  : public juce::AudioProcessorEditor, public juce::Slider::Listener
{
public:
    ChorusPluginAudioProcessorEditor (ChorusPluginAudioProcessor&);
    ~ChorusPluginAudioProcessorEditor() override;

    //==============================================================================
    void paint (juce::Graphics&) override;
    void resized() override;
    void sliderValueChanged(juce::Slider* slider) override;

private:
    // This reference is provided as a quick way for your editor to
    // access the processor object that created it.
    ChorusPluginAudioProcessor& audioProcessor;

    juce::Slider delaySlider;
    juce::Label delayLabel;
    juce::Slider pitchSlider;
    juce::Label pitchLabel;
    juce::Slider pitchLfoFreqSlider;
    juce::Label pitchLfoFreqLabel;
    juce::Slider pitchLfoDepthSlider;
    juce::Label pitchLfoDepthLabel;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ChorusPluginAudioProcessorEditor)
};
