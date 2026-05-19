#include "NumberBox.h"

NumberBox::NumberBox()
        : ProcessorBase
        (
            BusesProperties()
            .withOutput("Output", juce::AudioChannelSet::mono(), true)
        )
{
}

void NumberBox::prepareToPlay (double sampleRate, int samplesPerBlock)
{
}

void NumberBox::processBlock (juce::AudioBuffer<float>& buffer,
                                              juce::MidiBuffer& midiMessages)
{
    juce::ignoreUnused (midiMessages);

    juce::ScopedNoDenormals noDenormals;
    
    buffer.clear();
}

const juce::String NumberBox::getName() const
{
    return "Number Box";;
}