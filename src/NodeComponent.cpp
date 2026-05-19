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