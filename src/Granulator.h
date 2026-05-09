#pragma once

#include "ProcessorBase.h"

struct Grain
{
    double sourcePosition { 0. };
    double speed = { 0. }; // playback rate
    int lengthSamples { 0 };
    int currentSample { 0 }; // how many samples this grain has lived
    int currentChannel { 0 };
    bool active { false };

    void initiate (int channel, double startPos, double rate, int dur)
    {
        currentChannel = channel;
        sourcePosition = startPos;
        speed = rate;
        lengthSamples = dur;
        currentSample = 0;
        active = true;
    }

    // this is called for every audio frame while the grain is active
    float getNextSample (int channel, juce::AudioBuffer<float>& source)
    {
        if (!active)
            return 0.f;

        auto souceBufferSize = source.getNumSamples();

        // get the current read position
        int indexPos = static_cast<int> (sourcePosition); // integer part
        float fraction = static_cast<float> (sourcePosition - indexPos); // fractional part

        // linear interpolation
        float s0 = source.getSample (channel, indexPos % souceBufferSize);
        float s1 = source.getSample (channel, (indexPos + 1) % souceBufferSize);
        float sampleValue = s0 + ((s1 - s0) * fraction);

        // apply hanning window
        float progress { static_cast<float> (currentSample) / static_cast<float> (lengthSamples)};
        float envelope = 0.5f * (1.0f - std::cos(2.0f * juce::MathConstants<float>::pi * progress));
        sampleValue *= envelope;

        //update state
        if (currentChannel == channel)
        {
            sourcePosition += speed;
            ++currentSample;
        }
        if (currentSample >= lengthSamples)
            active = false;

        return sampleValue;
    }
};

class GranulatorProcessor : public ProcessorBase
{
public:
    GranulatorProcessor();
    void prepareToPlay (double, int) override;
    void processBlock (juce::AudioSampleBuffer&, juce::MidiBuffer&) override;
    const juce::String getName() const override;
private:
    // parameters
    int grainLength { 0 };
    int delayTime { 0 };
    double interval { 0. };
    double rate { 1.0 };
    double maxDelayTime { 2.0 };
    std::vector<double> spawnPosition;

    // grain pool
    int numGrain { 800 };
    std::vector<Grain> grainPool { std::vector<Grain>(static_cast<long unsigned int> (numGrain)) };
    int grainIndex { 0 };

    // delay buffer
    int delayBufferWritePosition { 0 };
    juce::AudioBuffer<float> delayBuffer;
    void updateWritePosition (juce::AudioBuffer<float>& buffer);
    void fillDelayBuffer (int channel, juce::AudioBuffer<float>& buffer);

    // play grains
    void triggerGrainBuffer (int channel, juce::AudioBuffer<float>& buffer);
};