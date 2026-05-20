#include "ConnectorComponent.h"

ConnectorComponent::ConnectorComponent(ConnectorType type, NodeComponent* parent, int idx)
        : connectorType (type),
          parentNode (parent),
          index (idx)
{
}

void ConnectorComponent::paint(juce::Graphics& g)
{
    g.setColour (orange);
    //if (connectorType == ConnectorType::Input) g.setColour (blue);
    g.fillEllipse (getLocalBounds().toFloat());
    g.fillRoundedRectangle (getLocalBounds().toFloat(), 5.0f);
}

void ConnectorComponent::mouseDown(const juce::MouseEvent& e)
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

void ConnectorComponent::mouseDrag(const juce::MouseEvent& e)
{
    if (onDrag) onDrag(this, e);
}

void ConnectorComponent::mouseUp(const juce::MouseEvent& e)
{
    if (onFinishDrag) onFinishDrag(this, e);
}

NodeComponent* ConnectorComponent::getParentNode() const
{
    return parentNode;
}

int ConnectorComponent::getIndex() const
{
    return index;
}

ConnectorType ConnectorComponent::getType() const
{
    return connectorType;
}