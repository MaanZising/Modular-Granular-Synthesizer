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

void NodeComponent::resized()
{
    // layout inputs on left edge
    int spacing = getHeight() / (inputs.size() + 1);
    for (int i = 0; i < inputs.size(); ++i)
        inputs[i]->setBounds(-5, spacing * (i+1) - 5, 12, 10);

    // layout outputs on right edge
    spacing = getHeight() / (outputs.size() + 1);
    for (int i = 0; i < outputs.size(); ++i)
        outputs[i]->setBounds(getWidth()-7, spacing * (i+1) - 5, 12, 10);
}

void NodeComponent::paint(juce::Graphics& g)
{
    g.setColour (darkGrey);
    g.fillRoundedRectangle (getLocalBounds().toFloat(), 5.0f);

    g.setColour (juce::Colours::white);
    g.drawText (nodeName, getLocalBounds(), juce::Justification::centred);
}