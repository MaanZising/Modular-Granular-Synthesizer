#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include "ConnectorComponent.h"

class ConnectorComponent;
class AudioPluginAudioProcessorEditor;

class NodeComponent : public juce::Component
{
public:
    NodeComponent(const juce::String& name, int numInputs, int numOutputs);

    void resized() override;

    void paint(juce::Graphics& g) override;

    void mouseDown(const juce::MouseEvent& e) override
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

    void mouseDrag(const juce::MouseEvent& e) override
    {
        if (!isClickOnConnector(e))
        {
            dragger.dragComponent(this, e, nullptr);
            if (onMoved) onMoved(); // notify parent to repaint connections
        }
    }

    juce::int64 getUniqueId() const { return uniqueId; }

    void setUniqueId(juce::int64 id) { uniqueId = id; }

    juce::OwnedArray<ConnectorComponent> inputs;
    juce::OwnedArray<ConnectorComponent> outputs;
    std::function<void()> onMoved;
    std::function<void(NodeComponent*)> onContextMenu;

private:
    juce::Colour lightGrey {juce::Colour (195, 195, 195)};
    juce::Colour midGrey {juce::Colour (120, 120, 120)};
    juce::Colour darkGrey {juce::Colour (60, 60, 60)};
    juce::Colour orange {juce::Colour (225, 145, 10)};
    juce::Colour blue {juce::Colour (80, 125, 185)};

    juce::ComponentDragger dragger;
    juce::String nodeName;
    juce::int64 uniqueId;

    bool isClickOnConnector(const juce::MouseEvent& e)
    {
        for (auto* port : inputs)
            if (port->getBounds().contains(e.getPosition())) return true;
        for (auto* port : outputs)
            if (port->getBounds().contains(e.getPosition())) return true;
        return false;
    }
};
