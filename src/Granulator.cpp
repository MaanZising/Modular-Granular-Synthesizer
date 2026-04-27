#include "Granulator.h"

GranulatorProcessor::GranulatorProcessor()
        : ProcessorBase
        (
            BusesProperties()
            .withInput("Input", juce::AudioChannelSet::discreteChannels (8), true)
            .withOutput("Output", juce::AudioChannelSet::stereo(), true)
        )
{
}

void GranulatorProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    // Use this method as the place to do any pre-playback
    // initialisation that you need..
    juce::ignoreUnused (sampleRate, samplesPerBlock);

    // initialize delay buffer
    int delayBufferSize { static_cast<int>(sampleRate * 2.0) };
    delayBuffer.setSize (getTotalNumInputChannels(), delayBufferSize);
    for (auto channel = 0; channel < getTotalNumOutputChannels(); ++channel)
        delayBuffer.clear (channel, 0, delayBuffer.getNumSamples());

    // initialize grain buffer
    int grainBufferSize { static_cast<int>(sampleRate * 2.0) };
    grainBuffer.setSize (getTotalNumInputChannels(), grainBufferSize);
    for (auto channel = 0; channel < getTotalNumOutputChannels(); ++channel)
        grainBuffer.clear (channel, 0, grainBuffer.getNumSamples());

    for (auto channel = 0; channel < getTotalNumOutputChannels(); ++channel)
        spawnPosition.push_back (0.);

    grainLength = static_cast<int> (sampleRate * 0.2);
    delayTime = static_cast<int> (sampleRate);
    interval = sampleRate * 0.05;
    rate = 0.8;
}

void GranulatorProcessor::processBlock (juce::AudioBuffer<float>& buffer,
                                              juce::MidiBuffer& midiMessages)
{
    juce::ignoreUnused (midiMessages);

    juce::ScopedNoDenormals noDenormals;
    auto totalNumInputChannels  = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();

    for (int channel = 0; channel < totalNumInputChannels; ++channel)
    {
        //auto* channelData = buffer.getWritePointer (channel);

        fillDelayBuffer (channel, buffer);
        fillGrainBuffer (channel, delayBuffer);
        triggerGrainBuffer (channel, buffer);
    }

    updateWritePosition (buffer);
}

void GranulatorProcessor::updateWritePosition (juce::AudioBuffer<float>& buffer)
{
    auto bufferSize = buffer.getNumSamples();
    auto delayBufferSize = delayBuffer.getNumSamples();
    delayBufferWritePosition += bufferSize;
    delayBufferWritePosition %= delayBufferSize;
}

void GranulatorProcessor::triggerGrainBuffer (int channel, juce::AudioBuffer<float>& buffer)
{
    auto bufferSize = buffer.getNumSamples();
    double startPosition = static_cast<double> (delayBufferWritePosition - delayTime);
    if (startPosition < 0)
        startPosition += static_cast<double> (delayBuffer.getNumSamples());

    for (int sample = 0; sample < bufferSize; ++sample)
    {
        // check if it's time to spawn a new grain
        if (spawnPosition[channel] >= interval)
        {
            //buffer.setSample (channel, sample, 1.0); // click
            grainPool[grainIndex].initiate (channel, startPosition, rate, grainLength); // spawn a new grain
            grainIndex = (grainIndex + 1) % numGrain;
            while (spawnPosition[channel] > 0.)
                spawnPosition[channel] -= interval; // reset timer
        }

        float outputSample { 0.f };

        // sum all active grains
        for (auto& grain : grainPool)
        {
            if (grain.active and grain.currentChannel == channel)
                outputSample += grain.getNextSample (channel, delayBuffer);
        }
        buffer.setSample (channel, sample, outputSample);

        ++spawnPosition[channel];
    }
}

void GranulatorProcessor::fillDelayBuffer (int channel, juce::AudioBuffer<float>& buffer)
{
    auto bufferSize = buffer.getNumSamples();
    auto delayBufferSize = delayBuffer.getNumSamples();

    // check if main buffer copies to delay buffer without needing to wrap
    if (delayBufferSize >= bufferSize + delayBufferWritePosition)
    {
        delayBuffer.copyFrom (channel, delayBufferWritePosition, buffer.getWritePointer (channel), bufferSize);
    }
    else
    {
        // determine how much space is left at the end of the delay buffer
        auto numSamplesToEnd = delayBufferSize - delayBufferWritePosition;
        delayBuffer.copyFrom (channel, delayBufferWritePosition, buffer.getWritePointer (channel), numSamplesToEnd);

        // calculate the amount of remaining samples to copy
        auto numSamplesAtStart = bufferSize - numSamplesToEnd;
        delayBuffer.copyFrom (channel, 0, buffer.getWritePointer (channel, numSamplesToEnd), numSamplesAtStart);
    }
}

void GranulatorProcessor::fillGrainBuffer (int channel, juce::AudioBuffer<float>& delayBuffer)
{
    auto delayBufferSize = delayBuffer.getNumSamples();
    auto delayBufferReadPosition = delayBufferWritePosition - delayTime; // grain start position updates at control rate

    if (delayBufferReadPosition < 0)
    {
        delayBufferReadPosition += delayBufferSize;
    }
    auto delayBufferNumFramesToEnd = delayBufferSize - delayBufferReadPosition;

    if (grainLength > delayBufferNumFramesToEnd)
    {
        auto delayBufferNumFramesAtStart = grainLength - delayBufferNumFramesToEnd;
        grainBuffer.copyFrom (channel, 0, delayBuffer, channel, delayBufferReadPosition, delayBufferNumFramesToEnd);
        grainBuffer.copyFrom (channel, delayBufferNumFramesToEnd, delayBuffer, channel, 0, delayBufferNumFramesAtStart);
    }
    else
    {
        grainBuffer.copyFrom (channel, 0, delayBuffer, channel, delayBufferReadPosition, grainLength);
    }
}

const juce::String GranulatorProcessor::getName() const
{
    return "Granulator";;
}