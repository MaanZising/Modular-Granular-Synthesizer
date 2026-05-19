#pragma once

#include "ProcessorBase.h"

class NumberBox : public ProcessorBase
{
public:
    NumberBox();
    void processBlock (juce::AudioSampleBuffer&, juce::MidiBuffer&) override;
    const juce::String getName() const override;

    float value { 0.0f };

private:
};