/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#pragma once

#include <rubberband/RubberBandStretcher.h>
#include <JuceHeader.h>

//==============================================================================
/**
*/
class ChorusPluginAudioProcessor  : public juce::AudioProcessor
{
public:
    //==============================================================================
    ChorusPluginAudioProcessor();
    ~ChorusPluginAudioProcessor() override;

    //==============================================================================
    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

   #ifndef JucePlugin_PreferredChannelConfigurations
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
   #endif

    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    //==============================================================================
    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    //==============================================================================
    const juce::String getName() const override;

    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;

    //==============================================================================
    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram (int index) override;
    const juce::String getProgramName (int index) override;
    void changeProgramName (int index, const juce::String& newName) override;

    //==============================================================================
    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;

private:
    juce::AudioBuffer<float> delayBuffer;
    juce::AudioBuffer<float> pitchShiftBuffer;
    int delayWritePosition{ 0 };

    // Realtime processing for plugins, all other options are default
    const int rbsOptions = RubberBand::RubberBandStretcher::Option::OptionProcessRealTime;

    const double rbsDefaultTimeRatio = 1.0;
    const double rbsDefaultPitchScale = 1.0116; // 1.005792941; // TODO: change this to suitable default pitch shift

    std::unique_ptr<RubberBand::RubberBandStretcher> rbs;
    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ChorusPluginAudioProcessor)
};
