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

    int getLatency();

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

    int delayOffset = 0;
    int dryOffset = 0;
    int pitchCents = 5;

    float pitchLfoFreq = 1.0; // Hz
    int pitchLfoDepth = 10;

private:
    juce::AudioBuffer<float> delayBuffer;
    juce::AudioBuffer<float> dryBuffer;
    juce::AudioBuffer<float> pitchShiftBuffer;    
    int delayWritePosition{ 0 };
    int dryWritePosition{ 0 };

    // LFOs
    const int lfoUpdateRate = 100; // we do not need to update the lfo as frequently, update every sampleRate/lfoUpdateRate samples
    juce::dsp::Oscillator<float> pitchLfo;

    // Realtime processing for plugins, all other options are default
    const int rbsOptions = RubberBand::RubberBandStretcher::Option::OptionProcessRealTime
                        + RubberBand::RubberBandStretcher::Option::OptionPitchHighConsistency;

    const double rbsDefaultTimeRatio = 1.0;
    //const double rbsDefaultPitchScale = pow(2.0, 10/1200.0); // 1.005792941; // TODO: change this to suitable default pitch shift
    const double rbsDefaultPitchScale = 1.0;

    int rbDelay;

    std::unique_ptr<RubberBand::RubberBandStretcher> rbs;    
    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ChorusPluginAudioProcessor)
};
