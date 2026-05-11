#pragma once

#include "PluginProcessor.h"
#include "NodeComponent.h"
#include "ConnectorComponent.h"

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

    void mouseDown (const juce::MouseEvent& e);
    void hookUpNode (NodeComponent* node);
    
    // persist
    juce::var serializeGraph();
    void saveToFile(const juce::File& file);
    void loadFromFile(const juce::File& file);
    void updateNodePosition(NodeComponent* node);

private:
    ModularGranularSynthesizer& processorRef;

    juce::Colour lightGrey {juce::Colour (195, 195, 195)};
    juce::Colour midGrey {juce::Colour (120, 120, 120)};
    juce::Colour darkGrey {juce::Colour (60, 60, 60)};
    juce::Colour orange {juce::Colour (225, 145, 10)};
    juce::Colour blue {juce::Colour (80, 125, 185)};

    juce::OwnedArray<NodeComponent> nodes;
    std::vector<Connection> connections;

    ConnectorComponent* draggingConnector = nullptr;
    juce::Point<int> dragPoint;

    void removeConnection (ConnectorComponent* p);
    void addNode(const juce::String& name, int numInputs, int numOutputs, juce::Rectangle<int> bounds);
    void removeNode(NodeComponent* node);

    NodeComponent* findNodeById(juce::int64 id);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AudioPluginAudioProcessorEditor)
};
