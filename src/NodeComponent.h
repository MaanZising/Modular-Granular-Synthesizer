#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include "ConnectorComponent.h"

class NodeComponent : public juce::Component
{
public:
    NodeComponent(const juce::String& name) : nodeName (name)
    {
        addAndMakeVisible (connector);
        connector.setBounds(40, 50, 20, 20);
    }

    void paint(juce::Graphics& g) override
    {
        g.setColour(juce::Colours::lightblue);
        g.fillRoundedRectangle(getLocalBounds().toFloat(), 8.0f);

        g.setColour(juce::Colours::black);
        g.drawText(nodeName, getLocalBounds(), juce::Justification::centred);
    }

    void mouseDown(const juce::MouseEvent& e) override
    {
        // Only start dragging if not clicking on connector
        if (! connector.getBounds().contains(e.getPosition()))
            dragger.startDraggingComponent(this, e);
    }

    void mouseDrag(const juce::MouseEvent& e) override
    {
        if (! connector.getBounds().contains(e.getPosition()))
        {
            dragger.dragComponent(this, e, nullptr);
            if (onMoved) onMoved(); // notify parent to repaint connections
        }
    }

    ConnectorComponent connector;
    std::function<void()> onMoved;

private:
    juce::ComponentDragger dragger;
    juce::String nodeName;
};
