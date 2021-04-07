/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
ChorusPluginAudioProcessor::ChorusPluginAudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
     : AudioProcessor (BusesProperties()
                     #if ! JucePlugin_IsMidiEffect
                      #if ! JucePlugin_IsSynth
                       .withInput  ("Input",  juce::AudioChannelSet::mono(), true)
                      #endif
                       .withOutput ("Output", juce::AudioChannelSet::stereo(), true)
                     #endif
                       )
#endif
{
}

ChorusPluginAudioProcessor::~ChorusPluginAudioProcessor()
{
}

//==============================================================================
const juce::String ChorusPluginAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool ChorusPluginAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool ChorusPluginAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool ChorusPluginAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double ChorusPluginAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int ChorusPluginAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int ChorusPluginAudioProcessor::getCurrentProgram()
{
    return 0;
}

void ChorusPluginAudioProcessor::setCurrentProgram (int index)
{
}

const juce::String ChorusPluginAudioProcessor::getProgramName (int index)
{
    return {};
}

void ChorusPluginAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
}

//==============================================================================
void ChorusPluginAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    delayBuffer.setSize(1,sampleRate*samplesPerBlock); // one channel, 1 second of buffer
    pitchShiftBuffer.setSize(1,sampleRate*samplesPerBlock);

    // create rubberbandstretcher object
    // Uses hard coded number of output channels to be 1 as we only affect one buffer
    rbs = std::make_unique<RubberBand::RubberBandStretcher>(sampleRate, 1, rbsOptions, rbsDefaultTimeRatio, rbsDefaultPitchScale);
    // rbs = std::make_unique<RubberBand::RubberBandStretcher>(sampleRate, 1);

    // clear internal buffers for rbs object
    rbs->reset();
}

void ChorusPluginAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool ChorusPluginAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
  #if JucePlugin_IsMidiEffect
    juce::ignoreUnused (layouts);
    return true;
  #else
    // This is the place where you check if the layout is supported.
    // In this template code we only support mono or stereo.
    // Some plugin hosts, such as certain GarageBand versions, will only
    // load plugins that support stereo bus layouts.
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono()
     && layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;

    return true;
  #endif
}
#endif

void ChorusPluginAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
    auto totalNumInputChannels  = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();

    // In case we have more outputs than inputs, this code clears any output
    // channels that didn't contain input data, (because these aren't
    // guaranteed to be empty - they may contain garbage).
    // This is here to avoid people getting screaming feedback
    // when they first compile a plugin, but obviously you don't need to keep
    // this code if your algorithm always overwrites all the output channels.
    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear (i, 0, buffer.getNumSamples());

    // processing
    // only process input channel 0 because mono is assumed
    int bufferLength = buffer.getNumSamples();
    int delayBufferLength = delayBuffer.getNumSamples();

    auto* inputData = buffer.getReadPointer(0);
    auto* outputDataR = buffer.getWritePointer(1);
    auto* outputDataL = buffer.getWritePointer(0);

    auto* delayInputData = delayBuffer.getReadPointer(0);
    auto* pitchShiftInputData = pitchShiftBuffer.getWritePointer(0); // rbs will write to here
    auto* pitchShiftOutputData = pitchShiftBuffer.getReadPointer(0); // then plugin will retrieve from here

    if (delayBufferLength > bufferLength + delayWritePosition) {
        delayBuffer.copyFromWithRamp(0, delayWritePosition, inputData, bufferLength, 1, 1);
    }
    else {
        int remaining = delayBufferLength - delayWritePosition;
        delayBuffer.copyFromWithRamp(0, delayWritePosition, inputData, remaining, 1, 1);
        delayBuffer.copyFromWithRamp(0, 0, inputData, bufferLength - remaining, 1, 1);
    }

    // now output the delayed signal

    int delayOffset = 0;
    int sampleRate = getSampleRate();
    int readPosition = (delayBufferLength + delayWritePosition - (sampleRate*delayOffset/1000)) % delayBufferLength;

    auto* offsetDelayInputData = delayInputData + readPosition;

    if (delayBufferLength > bufferLength + readPosition) {
        // send data from delay buffer to rbs to process
        rbs->process(&offsetDelayInputData, bufferLength, false);
        
        // retrieve pitch shifted samples into pitchShiftBuffer
        size_t numSamplesStretched = rbs->retrieve(&pitchShiftInputData, bufferLength);

        // output samples from pitchShiftBuffer
        buffer.addFrom(1, 0, pitchShiftOutputData, numSamplesStretched);
    }
    else {
        int remaining = delayBufferLength - readPosition;

        rbs->process(&offsetDelayInputData, remaining, false);
        size_t numSamplesStretched = rbs->retrieve(&pitchShiftInputData, remaining);
        buffer.addFrom(1, 0, pitchShiftOutputData, numSamplesStretched);

        auto remainingDelayInputData = offsetDelayInputData + remaining;

        rbs->process(&remainingDelayInputData, bufferLength - remaining, false);
        numSamplesStretched = rbs->retrieve(&pitchShiftInputData, bufferLength - remaining);
        buffer.addFrom(1, remaining, pitchShiftOutputData, numSamplesStretched);
    }


    // ----------------------------------
    delayWritePosition += bufferLength;
    delayWritePosition %= delayBufferLength;
}

//==============================================================================
bool ChorusPluginAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* ChorusPluginAudioProcessor::createEditor()
{
    return new ChorusPluginAudioProcessorEditor (*this);
}

//==============================================================================
void ChorusPluginAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    // You should use this method to store your parameters in the memory block.
    // You could do that either as raw data, or use the XML or ValueTree classes
    // as intermediaries to make it easy to save and load complex data.
}

void ChorusPluginAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    // You should use this method to restore your parameters from this memory block,
    // whose contents will have been created by the getStateInformation() call.
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new ChorusPluginAudioProcessor();
}
