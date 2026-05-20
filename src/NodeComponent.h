#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include "ConnectorComponent.h"

class ConnectorComponent;
class AudioPluginAudioProcessorEditor;

class NodeComponent : public juce::Component
{
public:
    NodeComponent(const juce::String& name, int numInputs, int numOutputs);

    void resized() override;
    void paint(juce::Graphics& g) override;

    void mouseDown(const juce::MouseEvent& e) override;
    void mouseDrag(const juce::MouseEvent& e) override;

    ConnectorComponent* getInputPort(int index);
    ConnectorComponent* getOutputPort(int index);

    void setWaveTypeComboBoxId(int id);
    void setNumberBoxValue(float value);
    juce::int64 getUniqueId() const;
    void setUniqueId(juce::int64 id);

    juce::OwnedArray<ConnectorComponent> inputs;
    juce::OwnedArray<ConnectorComponent> outputs;
    std::function<void()> onMoved;
    std::function<void(NodeComponent*)> onContextMenu;
    std::function<void(int)> onWaveTypeChanged;
    std::function<void(float)> onNumberValueChanged;

private:
    juce::Colour lightGrey {juce::Colour (200, 200, 200)};
    juce::Colour midGrey {juce::Colour (140, 140, 140)};
    juce::Colour darkGrey {juce::Colour (60, 60, 60)};
    juce::Colour orange {juce::Colour (225, 145, 10)};
    juce::Colour blue {juce::Colour (80, 125, 185)};

    juce::ComponentDragger dragger;
    juce::String nodeName;
    juce::int64 uniqueId;
    juce::ComboBox waveSelector;
    juce::Slider valueSlider;

    int offsetX { 0 };
    int offsetY { 0 };

    bool isClickOnConnector(const juce::MouseEvent& e);
};
