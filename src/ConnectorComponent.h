#pragma once

#include <juce_audio_processors/juce_audio_processors.h>

class NodeComponent;

enum class ConnectorType { Input, Output};

class ConnectorComponent : public juce::Component
{
public:
    ConnectorComponent(ConnectorType type, NodeComponent* parent, int idx);

    void paint(juce::Graphics& g) override;

    void mouseDown(const juce::MouseEvent& e) override
    {
        if (e.mods.isRightButtonDown())
        {
            if (onRightClick) onRightClick(this);
        }
        else
        {
            if (onStartDrag) onStartDrag(this, e);
        }
    }

    void mouseDrag(const juce::MouseEvent& e) override
    {
        if (onDrag) onDrag(this, e);
    }

    void mouseUp(const juce::MouseEvent& e) override
    {
        if (onFinishDrag) onFinishDrag(this, e);
    }

    NodeComponent* getParentNode() const { return parentNode; }
    int getIndex() const { return index; }
    ConnectorType getType() const { return connectorType; }

    std::function<void (ConnectorComponent*, const juce::MouseEvent&)> onStartDrag;
    std::function<void (ConnectorComponent*, const juce::MouseEvent&)> onDrag;
    std::function<void (ConnectorComponent*, const juce::MouseEvent&)> onFinishDrag;
    std::function<void(ConnectorComponent*)> onRightClick;

private:
    juce::Colour lightGrey {juce::Colour (200, 200, 200)};
    juce::Colour midGrey {juce::Colour (140, 140, 140)};
    juce::Colour darkGrey {juce::Colour (60, 60, 60)};
    juce::Colour orange {juce::Colour (225, 145, 10)};
    juce::Colour blue {juce::Colour (80, 125, 185)};

    ConnectorType connectorType;
    NodeComponent* parentNode;
    int index;
};
