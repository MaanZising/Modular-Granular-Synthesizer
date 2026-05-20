#pragma once

#include "ProcessorBase.h"

class NoiseGeneratorProcessor : public ProcessorBase
{
public:
    NoiseGeneratorProcessor();
    void prepareToPlay (double, int) override;
    void processBlock (juce::AudioSampleBuffer&, juce::MidiBuffer&) override;
    const juce::String getName() const override;

private:
    float freq { 220.0f };
};