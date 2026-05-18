#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_dsp/juce_dsp.h>
#include <juce_data_structures/juce_data_structures.h>

#include "Granulator.h"
#include "Oscillator.h"

using AudioGraphIOProcessor = juce::AudioProcessorGraph::AudioGraphIOProcessor;
using Node = juce::AudioProcessorGraph::Node;

#define JucePlugin_Name "Modular Granular Synthesizer"

//==============================================================================
class ModularGranularSynthesizer final : public juce::AudioProcessor,
                                         private juce::ValueTree::Listener
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

    //==============================================================================
    juce::ValueTree graphState { "Graph" };
    juce::int64 nextNodeId { 1 }; // start IDs at 1
    juce::int64 generateNodeId()
    {
        return nextNodeId++;
    }

    void valueTreeChildAdded (juce::ValueTree& parentTree, juce::ValueTree& childWhichHasBeenAdded) override;
    void valueTreeChildRemoved (juce::ValueTree& parentTree, juce::ValueTree& childWhichHasBeenRemoved, int index) override;

private:
    ////////////////
    std::unique_ptr<juce::AudioProcessorGraph> mainProcessor;

    void initialiseGraph();

    std::unique_ptr<juce::AudioProcessor> createProcessorForType(const juce::String& name);
    void createAudioNodeFromState(juce::ValueTree child);
    juce::AudioProcessorGraph::NodeID getGraphIdForGuiId(juce::int64 guiId);

    //==============================================================================
    // JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ModularGranularSynthesizer)
    //JUCE_DECLARE_NON_COPYABLE (ModularGranularSynthesizer)
};
