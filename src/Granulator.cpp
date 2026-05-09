#include "Granulator.h"

GranulatorProcessor::GranulatorProcessor()
        : ProcessorBase
        (
            BusesProperties()
            .withInput("Input", juce::AudioChannelSet::stereo(), true)
            .withInput("Input", juce::AudioChannelSet::discreteChannels (4), true)
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
    int delayBufferSize { static_cast<int>(sampleRate * maxDelayTime) };
    delayBuffer.setSize (getTotalNumInputChannels(), delayBufferSize);
    for (auto channel = 0; channel < getTotalNumOutputChannels(); ++channel)
        delayBuffer.clear (channel, 0, delayBuffer.getNumSamples());

    //
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

    for (int channel = 0; channel < 2; ++channel)
    {
        fillDelayBuffer (channel, buffer);
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

void GranulatorProcessor::triggerGrainBuffer (int channel, juce::AudioBuffer<float>& buffer)
{
    auto bufferSize = buffer.getNumSamples();
    auto sampleRate = getSampleRate();

    for (int sample = 0; sample < bufferSize; ++sample)
    {
        DBG (buffer.getReadPointer(2)[sample]);
        // get parameters

        // rate, channel 2, (0.0, inf)
        rate = buffer.getReadPointer(2)[sample];
        if (rate < 0.0)
            rate = 0.0;

        rate = 1.0;

        // delay time, channel 3, (0.001, maximum delay time)
        delayTime = static_cast<int> (buffer.getReadPointer(3)[sample] * sampleRate);
        if (delayTime >= static_cast<int> (sampleRate * maxDelayTime))
            delayTime = static_cast<int> (sampleRate * maxDelayTime) - 1;
        if (delayTime < static_cast<int> (0.001 * sampleRate))
            delayTime = static_cast<int> (0.001 * sampleRate);

        // grain length, channel 4, (0.001, delay time)
        grainLength = static_cast<int> (buffer.getReadPointer(4)[sample] * sampleRate);
        if (grainLength > delayTime)
            grainLength = delayTime;
        if (grainLength < static_cast<int> (0.001 * sampleRate))
            grainLength = static_cast<int> (0.001 * sampleRate);

        // interval, channel 5, (0.001, inf)
        interval = buffer.getReadPointer(5)[sample];
        if (interval < 0.001)
            interval = 0.001;
        interval = interval * sampleRate;

        double startPosition = static_cast<double> (delayBufferWritePosition - delayTime);
        if (startPosition < 0)
            startPosition += static_cast<double> (delayBuffer.getNumSamples());

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

const juce::String GranulatorProcessor::getName() const
{
    return "Granulator";;
}