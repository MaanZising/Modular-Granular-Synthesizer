#pragma once

#include "PluginProcessor.h"
#include "NodeComponent.h"

struct Connection
{
    ConnectorComponent* start;
    ConnectorComponent* end;
};

class AudioPluginAudioProcessorEditor final : public juce::AudioProcessorEditor
{
public:
    explicit AudioPluginAudioProcessorEditor (ModularGranularSynthesizer&);
    ~AudioPluginAudioProcessorEditor() override;

    void paint (juce::Graphics&) override;
    void resized() override;

private:
    ModularGranularSynthesizer& processorRef;

    juce::OwnedArray<NodeComponent> nodes;
    std::vector<Connection> connections;

    ConnectorComponent* draggingConnector = nullptr;
    juce::Point<int> dragPoint;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AudioPluginAudioProcessorEditor)
};
