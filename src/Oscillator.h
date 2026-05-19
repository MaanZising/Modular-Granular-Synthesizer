#pragma once

#include <juce_dsp/juce_dsp.h>

#include "ProcessorBase.h"

class OscillatorProcessor : public ProcessorBase
{
public:
    OscillatorProcessor();
    void prepareToPlay (double, int) override;
    void processBlock (juce::AudioSampleBuffer&, juce::MidiBuffer&) override;
    const juce::String getName() const override;
    void setWaveType (int newType) { type = newType; }

private:
    float freq { 220.0f };
    int type { 0 };
    juce::dsp::ProcessSpec spec;

    juce::dsp::Oscillator<float> osc;
};