#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_dsp/juce_dsp.h>

#include "ProcessorBase.h"

using AudioGraphIOProcessor = juce::AudioProcessorGraph::AudioGraphIOProcessor;
using Node = juce::AudioProcessorGraph::Node;

#define JucePlugin_Name "Modular Granular Synthesizer"

//==============================================================================
class ModularGranularSynthesizer final : public juce::AudioProcessor
{
public:
    //==============================================================================
    ModularGranularSynthesizer();
    ~ModularGranularSynthesizer() override;

    //==============================================================================
    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;

    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;
    using AudioProcessor::processBlock;

    //==============================================================================
    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    //==============================================================================
    const juce::String getName() const override;

    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;

    //==============================================================================
    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram (int index) override;
    const juce::String getProgramName (int index) override;
    void changeProgramName (int index, const juce::String& newName) override;

    //==============================================================================
    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;

private:
    // parameters
    int grainLength { 0 };
    int delayTime { 0 };
    double interval { 0. };
    double rate { 1. };
    std::vector<double> spawnPosition;

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

    int numGrain { 800 };
    std::vector<Grain> grainPool { std::vector<Grain>(static_cast<long unsigned int> (numGrain)) };
    int grainIndex { 0 };

    int delayBufferWritePosition { 0 };
    juce::AudioBuffer<float> delayBuffer;
    juce::AudioBuffer<float> grainBuffer;

    void updateWritePosition (juce::AudioBuffer<float>& buffer);
    void triggerGrainBuffer (int channel, juce::AudioBuffer<float>& buffer);
    void fillDelayBuffer (int channel, juce::AudioBuffer<float>& buffer);
    void fillGrainBuffer (int channel, juce::AudioBuffer<float>& delayBuffer);

    ////
    juce::dsp::ProcessSpec spec;

    juce::dsp::Oscillator<float> osc
    {
        [] (float phase)
        {
            // sine
            //return std::sin(phase);

            // square
            //return phase < 0.0f ? 1.0f : -1.0f;

            // sawtooth
            //return phase / juce::MathConstants<float>::pi;

            // triangle
            return std::abs (phase) / juce::MathConstants<float>::pi * 2.0f - 1.0f;
        }
    };

    ////////

    std::unique_ptr<juce::AudioProcessorGraph> mainProcessor;

    Node::Ptr audioInputNode;
    Node::Ptr audioOutputNode;

    void initialiseGraph()
    {
        mainProcessor->clear();
        audioInputNode = mainProcessor->addNode (std::make_unique<AudioGraphIOProcessor> (AudioGraphIOProcessor::audioInputNode));
        audioOutputNode = mainProcessor->addNode (std::make_unique<AudioGraphIOProcessor> (AudioGraphIOProcessor::audioOutputNode));
        connectAudioNodes();
    }

    void connectAudioNodes()
    {
        for (int channel = 0; channel < 2; ++channel)
            mainProcessor->addConnection ({ { audioInputNode->nodeID, channel },
                { audioOutputNode->nodeID, channel } });
    }

    //==============================================================================
    // JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ModularGranularSynthesizer)
    //JUCE_DECLARE_NON_COPYABLE (ModularGranularSynthesizer)
};
