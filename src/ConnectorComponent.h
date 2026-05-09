#pragma once

#include <juce_audio_processors/juce_audio_processors.h>

class ConnectorComponent : public juce::Component
{
public:
    ConnectorComponent() {}

    void paint(juce::Graphics& g) override
    {
        g.setColour(juce::Colours::darkblue);
        g.fillEllipse(getLocalBounds().toFloat());
    }

    void mouseDown(const juce::MouseEvent& e) override
    {
        if (onStartDrag) onStartDrag(this, e);
    }

    void mouseDrag(const juce::MouseEvent& e) override
    {
        if (onDrag) onDrag(this, e);
    }

    void mouseUp(const juce::MouseEvent& e) override
    {
        if (onFinishDrag) onFinishDrag(this, e);
    }

    std::function<void (ConnectorComponent*, const juce::MouseEvent&)> onStartDrag;
    std::function<void (ConnectorComponent*, const juce::MouseEvent&)> onDrag;
    std::function<void (ConnectorComponent*, const juce::MouseEvent&)> onFinishDrag;
};
