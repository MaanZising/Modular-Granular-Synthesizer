#include "Operator.h"

Operator::Operator()
        : ProcessorBase
        (
            BusesProperties()
            .withInput("Input", juce::AudioChannelSet::stereo(), true)
            .withOutput("Output", juce::AudioChannelSet::mono(), true)
        )
{
}

void Operator::processBlock (juce::AudioBuffer<float>& buffer,
                                              juce::MidiBuffer& midiMessages)
{
    juce::ignoreUnused (midiMessages);
    juce::ScopedNoDenormals noDenormals;

    for (int sample = 0; sample < buffer.getNumSamples(); ++sample)
    {
        auto& value1 = buffer.getReadPointer(0)[sample];
        auto& value2 = buffer.getReadPointer(1)[sample];

        switch (type)
        {
        case 0:
            buffer.getWritePointer(0)[sample] = value1 + value2;
            break;
        case 1:
            buffer.getWritePointer(0)[sample] = value1 - value2;
            break;
        case 2:
            buffer.getWritePointer(0)[sample] = value1 * value2;
            break;
        case 3:
            if (value2 == 0.0f)
            {
                buffer.getWritePointer(0)[sample] = 0.0f;
                break;
            }
            buffer.getWritePointer(0)[sample] = value1 / value2;
            break;
        }
    }
}

const juce::String Operator::getName() const
{
    return "Operator";;
}