#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
ModularGranularSynthesizer::ModularGranularSynthesizer()
     : AudioProcessor (BusesProperties()
                     #if ! JucePlugin_IsMidiEffect
                      #if ! JucePlugin_IsSynth
                       .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                      #endif
                       .withOutput ("Output", juce::AudioChannelSet::stereo(), true)
                     #endif
                       ),
        
        mainProcessor (new juce::AudioProcessorGraph()) // initiate the main AudioProcessorGraph
{
}

ModularGranularSynthesizer::~ModularGranularSynthesizer()
{
}

//==============================================================================
const juce::String ModularGranularSynthesizer::getName() const
{
    return JucePlugin_Name;
}

bool ModularGranularSynthesizer::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool ModularGranularSynthesizer::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool ModularGranularSynthesizer::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double ModularGranularSynthesizer::getTailLengthSeconds() const
{
    return 0.0;
}

int ModularGranularSynthesizer::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int ModularGranularSynthesizer::getCurrentProgram()
{
    return 0;
}

void ModularGranularSynthesizer::setCurrentProgram (int index)
{
    juce::ignoreUnused (index);
}

const juce::String ModularGranularSynthesizer::getProgramName (int index)
{
    juce::ignoreUnused (index);
    return {};
}

void ModularGranularSynthesizer::changeProgramName (int index, const juce::String& newName)
{
    juce::ignoreUnused (index, newName);
}

//==============================================================================
void ModularGranularSynthesizer::prepareToPlay (double sampleRate, int samplesPerBlock)
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

    ////////
    spec.sampleRate = sampleRate;
    spec.maximumBlockSize = (juce::uint32) samplesPerBlock;
    spec.numChannels = (juce::uint32) getTotalNumOutputChannels();

    // initialize oscillator
    osc.prepare (spec);
    osc.setFrequency (440.0f);

    ////////
    mainProcessor->setPlayConfigDetails (getMainBusNumInputChannels(),
        getMainBusNumOutputChannels(),
        sampleRate,
        samplesPerBlock);
    mainProcessor->prepareToPlay (sampleRate, samplesPerBlock);
    //initialiseGraph();
}

void ModularGranularSynthesizer::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
    mainProcessor->releaseResources();
}

bool ModularGranularSynthesizer::isBusesLayoutSupported (const BusesLayout& layouts) const
{
  #if JucePlugin_IsMidiEffect
    juce::ignoreUnused (layouts);
    return true;
  #else
    // This is the place where you check if the layout is supported.
    // In this template code we only support mono or stereo.
    // Some plugin hosts, such as certain GarageBand versions, will only
    // load plugins that support stereo bus layouts.
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono()
     && layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;

    // This checks if the input layout matches the output layout
   #if ! JucePlugin_IsSynth
    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
        return false;
   #endif

    return true;
  #endif
}

void ModularGranularSynthesizer::processBlock (juce::AudioBuffer<float>& buffer,
                                              juce::MidiBuffer& midiMessages)
{
    juce::ignoreUnused (midiMessages);

    juce::ScopedNoDenormals noDenormals;
    auto totalNumInputChannels  = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();

    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear (i, 0, buffer.getNumSamples());

    //updateGraph();
    mainProcessor->processBlock (buffer, midiMessages);

#if 0
    for (int channel = 0; channel < totalNumInputChannels; ++channel)
    {
        //auto* channelData = buffer.getWritePointer (channel);

        fillDelayBuffer (channel, buffer);
        fillGrainBuffer (channel, delayBuffer);
        triggerGrainBuffer (channel, buffer);
    }

    updateWritePosition (buffer);
#endif

#if 0
    buffer.clear();

    // 1. Create an AudioBlock from the buffer
    juce::dsp::AudioBlock<float> block (buffer);
    
    // 2. Create a ProcessContext that "replaces" the data in the block
    juce::dsp::ProcessContextReplacing<float> context (block);

    // 3. Let the oscillator do the work
    osc.process (context);
#endif
}

void ModularGranularSynthesizer::updateWritePosition (juce::AudioBuffer<float>& buffer)
{
    auto bufferSize = buffer.getNumSamples();
    auto delayBufferSize = delayBuffer.getNumSamples();
    delayBufferWritePosition += bufferSize;
    delayBufferWritePosition %= delayBufferSize;
}

void ModularGranularSynthesizer::triggerGrainBuffer (int channel, juce::AudioBuffer<float>& buffer)
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

void ModularGranularSynthesizer::fillDelayBuffer (int channel, juce::AudioBuffer<float>& buffer)
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

void ModularGranularSynthesizer::fillGrainBuffer (int channel, juce::AudioBuffer<float>& delayBuffer)
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

//==============================================================================
bool ModularGranularSynthesizer::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* ModularGranularSynthesizer::createEditor()
{
    return new AudioPluginAudioProcessorEditor (*this);
}

//==============================================================================
void ModularGranularSynthesizer::getStateInformation (juce::MemoryBlock& destData)
{
    // You should use this method to store your parameters in the memory block.
    // You could do that either as raw data, or use the XML or ValueTree classes
    // as intermediaries to make it easy to save and load complex data.
    juce::ignoreUnused (destData);
}

void ModularGranularSynthesizer::setStateInformation (const void* data, int sizeInBytes)
{
    // You should use this method to restore your parameters from this memory block,
    // whose contents will have been created by the getStateInformation() call.
    juce::ignoreUnused (data, sizeInBytes);
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new ModularGranularSynthesizer();
}
