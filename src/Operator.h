#pragma once

#include "ProcessorBase.h"

class Operator : public ProcessorBase
{
public:
    Operator();
    void processBlock (juce::AudioSampleBuffer&, juce::MidiBuffer&) override;
    const juce::String getName() const override;
    
    int type { 0 };
};