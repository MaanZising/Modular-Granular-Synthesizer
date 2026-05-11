# include "NodeComponent.h"

NodeComponent::NodeComponent(const juce::String& name, int numInputs, int numOutputs)
    : nodeName(name)
{
    // create input ports
    for (int i = 0; i < numInputs; ++i)
    {
        auto* port = new ConnectorComponent(ConnectorType::Input, this, i);
        inputs.add(port);
        addAndMakeVisible(port);
    }

    // create output ports
    for (int i = 0; i < numOutputs; ++i)
    {
        auto* port = new ConnectorComponent(ConnectorType::Output, this, i);
        outputs.add(port);
        addAndMakeVisible(port);
    }
}
