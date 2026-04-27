#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_dsp/juce_dsp.h>

//#include "ProcessorBase.h"
#include "Granulator.h"
#include "Oscillator.h"

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

    ////////////////

    juce::StringArray processorChoices { "Empty", "Granulator", "Oscillator"};
    
    std::unique_ptr<juce::AudioProcessorGraph> mainProcessor;

    juce::AudioParameterBool* muteInput;
    juce::AudioParameterChoice* processorSlot1;
    juce::AudioParameterChoice* processorSlot2;
    juce::AudioParameterBool* bypassSlot1;
    juce::AudioParameterBool* bypassSlot2;

    Node::Ptr audioInputNode;
    Node::Ptr audioOutputNode;
    Node::Ptr slot1Node;
    Node::Ptr slot2Node;

    void initialiseGraph()
    {
        mainProcessor->clear();
        audioInputNode = mainProcessor->addNode (std::make_unique<AudioGraphIOProcessor> (AudioGraphIOProcessor::audioInputNode));
        audioOutputNode = mainProcessor->addNode (std::make_unique<AudioGraphIOProcessor> (AudioGraphIOProcessor::audioOutputNode));
        //connectAudioNodes();
    }

#if 1
    void connectAudioNodes()
    {
        for (int channel = 0; channel < 2; ++channel)
            mainProcessor->addConnection ({ { audioInputNode->nodeID, channel },
                { audioOutputNode->nodeID, channel } });
    }

    void updateGraph()
    {
        bool hasChanged = false;
        juce::Array<juce::AudioParameterChoice*> choices
        {
            processorSlot1,
            processorSlot2
        };

        juce::Array<juce::AudioParameterBool*> bypasses
        {
            bypassSlot1,
            bypassSlot2
        };

        juce::ReferenceCountedArray<Node> slots;
        slots.add (slot1Node);
        slots.add (slot2Node);

        for (int i = 0; i < 2; ++i)
        {
            auto& choice = choices.getReference (i);
            auto slot = slots.getUnchecked (i);
            if (choice->getIndex() == 0)
            {
                if (slot != nullptr)
                {
                    mainProcessor->removeNode (slot.get());
                    slots.set (i, nullptr);
                    hasChanged = true;
                }
            }
            else if (choice->getIndex() == 1)
            {
                if (slot != nullptr)
                {
                    if (slot->getProcessor()->getName() == "Granulator")
                        continue;
                    mainProcessor->removeNode (slot.get());
                }
                slots.set (i, mainProcessor->addNode (std::make_unique<GranulatorProcessor>()));
                hasChanged = true;
            }
            else if (choice->getIndex() == 2)
            {
                if (slot != nullptr)
                {
                    if (slot->getProcessor()->getName() == "Oscillator")
                        continue;
                    mainProcessor->removeNode (slot.get());
                }
                slots.set (i, mainProcessor->addNode (std::make_unique<OscillatorProcessor>()));
                hasChanged = true;
            }
        }

        ////
        if (hasChanged)
        {
            for (auto connection : mainProcessor->getConnections())
                mainProcessor->removeConnection (connection);
            juce::ReferenceCountedArray<Node> activeSlots;
            for (auto slot : slots)
            {
                if (slot != nullptr)
                {
                    activeSlots.add (slot);
                    slot->getProcessor()->setPlayConfigDetails (getMainBusNumInputChannels(),
                        getMainBusNumOutputChannels(),
                        getSampleRate(),
                        getBlockSize());
                }
            }
            if (activeSlots.isEmpty())
            {
                connectAudioNodes();
            }
            else
            {
                for (int i = 0; i < activeSlots.size() - 1; ++i)
                {
                    for (int channel = 0; channel < 2; ++channel)
                        mainProcessor->addConnection ({ { activeSlots.getUnchecked (i)->nodeID, channel },
                            { activeSlots.getUnchecked (i + 1)->nodeID, channel } });
                }
                for (int channel = 0; channel < 2; ++channel)
                {
                    mainProcessor->addConnection ({ { audioInputNode->nodeID, channel },
                        { activeSlots.getFirst()->nodeID, channel } });
                    mainProcessor->addConnection ({ { activeSlots.getLast()->nodeID, channel },
                        { audioOutputNode->nodeID, channel } });
                }
            }
            for (auto node : mainProcessor->getNodes())
                node->getProcessor()->enableAllBuses();
        }

        ////
        for (int i = 0; i < 2; ++i)
        {
            auto slot = slots.getUnchecked (i);
            auto& bypass = bypasses.getReference (i);
            if (slot != nullptr)
                slot->setBypassed (bypass->get());
        }
        audioInputNode->setBypassed (muteInput->get());
        slot1Node = slots.getUnchecked (0);
        slot2Node = slots.getUnchecked (1);
    }
#endif

    //==============================================================================
    // JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ModularGranularSynthesizer)
    //JUCE_DECLARE_NON_COPYABLE (ModularGranularSynthesizer)
};
