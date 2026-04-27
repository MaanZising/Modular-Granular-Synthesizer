#include "ProcessorBase.h"

// ProcessorBase::ProcessorBase(const BusesProperties& busProps)
//         : AudioProcessor (busProps)
// {
// }

ProcessorBase::ProcessorBase(const BusesProperties& busProps)
        : AudioProcessor
        (
            BusesProperties()
            .withInput("Input", juce::AudioChannelSet::stereo(), true)
            .withOutput("Output", juce::AudioChannelSet::stereo(), true)
        )
{
}