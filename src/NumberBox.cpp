#include "NumberBox.h"

NumberBox::NumberBox()
        : ProcessorBase
        (
            BusesProperties()
            .withOutput("Output", juce::AudioChannelSet::mono(), true)
        )
{
}

void NumberBox::processBlock (juce::AudioBuffer<float>& buffer,
                                              juce::MidiBuffer& midiMessages)
{
    juce::ignoreUnused (midiMessages);
    juce::ScopedNoDenormals noDenormals;
    
    buffer.clear();

    for (int sample = 0; sample < buffer.getNumSamples(); ++sample)
    {
        buffer.getWritePointer(0)[sample] = value;
    }
}

const juce::String NumberBox::getName() const
{
    return "Number Box";;
}