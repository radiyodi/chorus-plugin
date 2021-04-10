/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
ChorusPluginAudioProcessorEditor::ChorusPluginAudioProcessorEditor (ChorusPluginAudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p)
{
    // Make sure that before the constructor has finished, you've set the
    // editor's size to whatever you need it to be.
    setSize (400, 150);

    addAndMakeVisible(delaySlider);
    delaySlider.setRange(0, 100, 1);
    delaySlider.setTextValueSuffix(" ms");
    delaySlider.addListener(this);
    delaySlider.setValue(0);

    addAndMakeVisible(delayLabel);
    delayLabel.setText("Delay",juce::NotificationType::sendNotification);
    delayLabel.attachToComponent(&delaySlider, true);

    addAndMakeVisible(pitchSlider);
    pitchSlider.setRange(-25, 25, 1);
    pitchSlider.setTextValueSuffix(" cents");
    pitchSlider.addListener(this);
    pitchSlider.setValue(5);

    addAndMakeVisible(pitchLabel);
    pitchLabel.setText("Pitch", juce::NotificationType::sendNotification);
    pitchLabel.attachToComponent(&pitchSlider, true);
}

ChorusPluginAudioProcessorEditor::~ChorusPluginAudioProcessorEditor()
{
}

//==============================================================================
void ChorusPluginAudioProcessorEditor::paint (juce::Graphics& g)
{
    /*
    // (Our component is opaque, so we must completely fill the background with a solid colour)
    g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));

    g.setColour (juce::Colours::white);
    g.setFont (15.0f);
    g.drawFittedText("Chorus", getLocalBounds(), juce::Justification::centredTop, 1);
    */
}

void ChorusPluginAudioProcessorEditor::resized()
{
    // This is generally where you'll want to lay out the positions of any
    // subcomponents in your editor..
    delaySlider.setBounds(75,25, 300,50);
    pitchSlider.setBounds(75,62, 300, 50);
}

void ChorusPluginAudioProcessorEditor::sliderValueChanged(juce::Slider* slider)
{
    if (slider == &delaySlider) {
        double delayValue = delaySlider.getValue();
        //DBG(delayValue);
        audioProcessor.delayOffset = delayValue;
    }
    else if (slider == &pitchSlider) {
        double pitchValue = pitchSlider.getValue();
        //DBG(pitchValue);
        audioProcessor.pitchCents = pitchValue;
    }
}