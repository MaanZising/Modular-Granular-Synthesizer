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

    // create wave selector for oscillator
    if (nodeName == "Oscillator")
    {
        waveSelector.addSectionHeading ("Bipolar");
        waveSelector.addItem ("Sine", 1);
        waveSelector.addItem ("Triangle", 2);
        waveSelector.addItem ("Sawtooth", 3);
        waveSelector.addItem ("Square", 4);

        waveSelector.addSectionHeading ("Unipolar");
        waveSelector.addItem ("Sine", 5);
        waveSelector.addItem ("Triangle", 6);
        waveSelector.addItem ("Sawtooth", 7);
        waveSelector.addItem ("Square", 8);

        waveSelector.setSelectedId (1); // default to Sine Bipolar

        addAndMakeVisible (waveSelector);

        waveSelector.onChange = [this]()
        {
            if (onWaveTypeChanged)
            {
                // JUCE ComboBox items start at 1, but the switch case starts at 0
                int zeroIndexedChoice = waveSelector.getSelectedId() - 1;
                onWaveTypeChanged (zeroIndexedChoice);
            }
        };
    }

    // create number box
    if (nodeName == "Number Box")
    {
        valueSlider.setSliderStyle(juce::Slider::RotaryVerticalDrag);
        valueSlider.setTextBoxStyle(juce::Slider::TextBoxAbove, false, 80, 22);
        valueSlider.setRange(-20000.0f, 20000.0f, 0.001f);
        valueSlider.setValue(0.0f, juce::dontSendNotification);
        valueSlider.setMouseDragSensitivity(40000);

        valueSlider.setColour(juce::Slider::textBoxBackgroundColourId, midGrey);
        valueSlider.setColour(juce::Slider::textBoxOutlineColourId, darkGrey);
        valueSlider.setColour(juce::Slider::textBoxTextColourId, juce::Colours::black);
        valueSlider.setColour(juce::Slider::rotarySliderFillColourId, juce::Colours::transparentBlack);
        valueSlider.setColour(juce::Slider::rotarySliderOutlineColourId, juce::Colours::transparentBlack);
        
        addAndMakeVisible(valueSlider);

        valueSlider.onValueChange = [this]()
        {
            if (onNumberValueChanged)
            {
                onNumberValueChanged((float)valueSlider.getValue());
            }
        };
    }

    if (nodeName == "Granulator")
        offsetY = -25;
}

void NodeComponent::resized()
{
    // layout inputs on left edge
    int spacing = (getHeight() + offsetY) / (inputs.size() + 1);
    for (int i = 0; i < inputs.size(); ++i)
        inputs[i]->setBounds(-5, spacing * (i+1) - 5 - offsetY, 12, 10);

    // layout outputs on right edge
    spacing = (getHeight() + offsetY) / (outputs.size() + 1);
    for (int i = 0; i < outputs.size(); ++i)
        outputs[i]->setBounds(getWidth()-7, spacing * (i+1) - 5 - offsetY, 12, 10);
    
    // wave selector
    if (nodeName == "Oscillator")
    {
        int comboWidth = 100;
        int comboHeight = 20;
        waveSelector.setBounds
        (
            (getWidth() - comboWidth) / 2 + 10, 
            (getHeight() - comboHeight) / 2, 
            comboWidth, 
            comboHeight
        );
    }

    // number box
    if (nodeName == "Number Box")
    {
        valueSlider.setBounds
        (
            20,
            -1, 
            70, 
            40
        );
    }
}

void NodeComponent::paint(juce::Graphics& g)
{
    g.setColour (darkGrey);
    g.fillRoundedRectangle (getLocalBounds().toFloat(), 5.0f);

    g.setColour (juce::Colours::white);

    std::vector<juce::String> inputPortNames {};
    std::vector<juce::String> outputPortNames {};
    auto nodeNameText = nodeName;

    if (nodeName == "Audio Input")
    {
        outputPortNames.assign ({"L", "R"});
        offsetX = -10;
    }
    else if (nodeName == "Audio Output")
    {
        inputPortNames.assign ({"L", "R"});
        offsetX = 8;
    }
    else if (nodeName == "Granulator")
    {
        inputPortNames.assign ({"In L", "In R", "Rate", "Delay", "Length", "Interval"});
        outputPortNames.assign ({"L", "R"});
        offsetY = -53;
    }
    else if (nodeName == "Oscillator")
    {
        inputPortNames.assign ({"Freq"});
        nodeNameText = "";
    }
    else if (nodeName == "Number Box")
    {
        nodeNameText = "";
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

    g.setFont (15.0f);
    g.drawText (nodeNameText, offsetX, offsetY, getLocalBounds().getWidth(), getLocalBounds().getHeight(), juce::Justification::centred);
}

void NodeComponent::mouseDown(const juce::MouseEvent& e)
{
    if (e.mods.isRightButtonDown())
    {
        juce::PopupMenu menu;
        menu.addItem(1, "Delete Node");

        menu.showMenuAsync(juce::PopupMenu::Options(),
            [this](int result)
            {
                if (result == 0)
                    return; // user cancelled, do nothing
                if (onContextMenu)
                    onContextMenu(this);
            });
        return;
    }
    // only start dragging if not clicking on a connector
    if (!isClickOnConnector(e))
        dragger.startDraggingComponent(this, e);
}

void NodeComponent::mouseDrag(const juce::MouseEvent& e)
{
    if (!isClickOnConnector(e))
    {
        dragger.dragComponent(this, e, nullptr);
        if (onMoved) onMoved(); // notify parent to repaint connections
    }
}

ConnectorComponent* NodeComponent::getInputPort(int index)
{
    return (index >= 0 && index < inputs.size()) ? inputs[index] : nullptr;
}

ConnectorComponent* NodeComponent::getOutputPort(int index)
{
    return (index >= 0 && index < outputs.size()) ? outputs[index] : nullptr;
}

void NodeComponent::setWaveTypeComboBoxId(int id)
{
    waveSelector.setSelectedId(id, juce::dontSendNotification);
}

void NodeComponent::setNumberBoxValue(float value)
{
    valueSlider.setValue(value, juce::dontSendNotification);
}

juce::int64 NodeComponent::getUniqueId() const
{
    return uniqueId;
}

void NodeComponent::setUniqueId(juce::int64 id)
{
    uniqueId = id;
}

bool NodeComponent::isClickOnConnector(const juce::MouseEvent& e)
{
    for (auto* port : inputs)
        if (port->getBounds().contains(e.getPosition())) return true;
    for (auto* port : outputs)
        if (port->getBounds().contains(e.getPosition())) return true;
    return false;
}