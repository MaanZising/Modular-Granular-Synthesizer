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
    osc.setFrequency (freq);
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

    switch (type)
    {
    // bipolar
    case 0:
        osc.initialise ([] (float phase) { return std::sin(phase); }); // sine
        break;
    case 1:
        osc.initialise ([] (float phase) { return std::abs (phase) / juce::MathConstants<float>::pi * 2.0f - 1.0f; }); // triangle
        break;
    case 2:
        osc.initialise ([] (float phase) { return phase / juce::MathConstants<float>::pi; }); // sawtooth
        break;
    case 3:
        osc.initialise ([] (float phase) { return phase < 0.0f ? 1.0f : -1.0f; }); // square
        break;
    // unipolar
    case 4:
        osc.initialise ([] (float phase) { return std::sin(phase) * 0.5f + 0.5f; }); // sine
        break;
    case 5:
        osc.initialise ([] (float phase) { return std::abs (phase) / juce::MathConstants<float>::pi; }); // triangle
        break;
    case 6:
        osc.initialise ([] (float phase) { return phase / juce::MathConstants<float>::pi * 0.5f + 0.5f; }); // sawtooth
        break;
    case 7:
        osc.initialise ([] (float phase) { return phase < 0.0f ? 1.0f : 0.0f; }); // square
        break;
    }

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