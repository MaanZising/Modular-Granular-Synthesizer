#include "NoiseGenerator.h"

NoiseGeneratorProcessor::NoiseGeneratorProcessor()
        : ProcessorBase
        (
            BusesProperties()
            .withInput("Input", juce::AudioChannelSet::mono(), true)
            .withOutput("Output", juce::AudioChannelSet::mono(), true)
        )
{
}

void NoiseGeneratorProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
}

void NoiseGeneratorProcessor::processBlock (juce::AudioBuffer<float>& buffer,
                                              juce::MidiBuffer& midiMessages)
{
    juce::ignoreUnused (midiMessages);

    juce::ScopedNoDenormals noDenormals;
}

const juce::String NoiseGeneratorProcessor::getName() const
{
    return "Noise Generator";
}