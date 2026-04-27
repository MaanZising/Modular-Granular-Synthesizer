#include "Oscillator.h"

OscillatorProcessor::OscillatorProcessor()
        : ProcessorBase
        (
            BusesProperties()
            .withInput("Input", juce::AudioChannelSet::mono(), true)
            .withOutput("Output", juce::AudioChannelSet::mono(), true)
        )
{
}

void OscillatorProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    spec.sampleRate = sampleRate;
    spec.maximumBlockSize = (juce::uint32) samplesPerBlock;
    spec.numChannels = (juce::uint32) getTotalNumOutputChannels();

    // initialize oscillator
    osc.prepare (spec);
    osc.setFrequency (440.0f);
}

void OscillatorProcessor::processBlock (juce::AudioBuffer<float>& buffer,
                                              juce::MidiBuffer& midiMessages)
{
    juce::ignoreUnused (midiMessages);

    juce::ScopedNoDenormals noDenormals;
    auto totalNumInputChannels  = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();

    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear (i, 0, buffer.getNumSamples());

    buffer.clear();

    // 1. Create an AudioBlock from the buffer
    juce::dsp::AudioBlock<float> block (buffer);
    
    // 2. Create a ProcessContext that "replaces" the data in the block
    juce::dsp::ProcessContextReplacing<float> context (block);

    // 3. Let the oscillator do the work
    osc.process (context);
}

const juce::String OscillatorProcessor::getName() const
{
    return "Oscillator";;
}