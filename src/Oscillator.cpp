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

    for (int sample = 0; sample < buffer.getNumSamples(); ++sample)
    {
        freq = buffer.getReadPointer(0)[sample];
        if (freq < 0.0f) freq = 0.0f;
        osc.setFrequency (freq);
        buffer.getWritePointer(0)[sample] = osc.processSample (0.0f);
    }

    //juce::dsp::AudioBlock<float> block (buffer);
    //juce::dsp::ProcessContextReplacing<float> context (block);
    //osc.process (context);
}

const juce::String OscillatorProcessor::getName() const
{
    return "Oscillator";
}