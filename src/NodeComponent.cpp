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

    std::vector<juce::String> inputPortNames {};
    std::vector<juce::String> outputPortNames {};
    int offsetX {};

    if (nodeName == "Audio Input")
    {
        outputPortNames.assign ({"L", "R"});
        offsetX = -10;
    }
    else if (nodeName == "Audio Output")
    {
        inputPortNames.assign ({"L", "R"});
        offsetX = 10;
    }

    g.setFont (11.0f);

    for (int i = 0; i < outputs.size(); ++i)
    {
        auto portBounds = outputs[i]->getBounds();
        
        juce::Rectangle<int> labelArea
        (
            portBounds.getX() - 44,
            portBounds.getY() - 5,
            40,
            portBounds.getHeight() + 10
        );

        juce::String labelText = (i < outputPortNames.size()) ? outputPortNames[i] : "";
        
        g.drawText (labelText, labelArea, juce::Justification::centredRight, true);
    }

    for (int i = 0; i < inputs.size(); ++i)
    {
        auto portBounds = inputs[i]->getBounds();
        
        juce::Rectangle<int> labelArea
        (
            portBounds.getRight() + 4,
            portBounds.getY() - 5,
            40,
            portBounds.getHeight() + 10
        );

        juce::String labelText = (i < inputPortNames.size()) ? inputPortNames[i] : "";
        
        g.drawText (labelText, labelArea, juce::Justification::centredLeft, true);
    }

    g.setFont (14.0f);
    g.drawText (nodeName, offsetX, 0, getLocalBounds().getWidth(), getLocalBounds().getHeight(), juce::Justification::centred);
}