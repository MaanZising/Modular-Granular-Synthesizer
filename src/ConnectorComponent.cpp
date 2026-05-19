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