#pragma once

#include "ProcessorBase.h"

class NumberBox : public ProcessorBase
{
public:
    NumberBox();
    void prepareToPlay (double, int) override;
    void processBlock (juce::AudioSampleBuffer&, juce::MidiBuffer&) override;
    const juce::String getName() const override;

private:
};