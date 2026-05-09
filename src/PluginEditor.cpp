#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
AudioPluginAudioProcessorEditor::AudioPluginAudioProcessorEditor (ModularGranularSynthesizer& p)
    : AudioProcessorEditor (&p), processorRef (p)
{
    juce::ignoreUnused (processorRef);
    setSize (800, 600);
    setResizable (true, true);

    // create nodes
    nodes.add (new NodeComponent("Node A"));
    nodes.add (new NodeComponent("Node B"));
    nodes.add (new NodeComponent("Node C"));

    for (auto* node : nodes)
    {
        addAndMakeVisible (node);
    }

    // position nodes
    nodes[0]->setBounds (50, 50, 100, 60);
    nodes[1]->setBounds (250, 150, 100, 60);
    nodes[2]->setBounds (150, 250, 100, 60);

    // hook connector events
    for (auto* node : nodes)
    {
        node->connector.onStartDrag = [this](ConnectorComponent* c, const juce::MouseEvent& e)
        {
            draggingConnector = c;
            dragPoint = e.getPosition();
        };

        node->connector.onDrag = [this](ConnectorComponent* c, const juce::MouseEvent& e)
        {
            dragPoint = e.getEventRelativeTo(this).getPosition();
            repaint();
        };

        node->connector.onFinishDrag = [this](ConnectorComponent* c, const juce::MouseEvent& e)
        {
            // Check if released over another connector
            for (auto* node : nodes)
            {
                if (&node->connector != c && node->connector.getBounds().contains(e.getEventRelativeTo(node).getPosition()))
                {
                    connections.push_back ({ c, &node->connector });
                    break;
                }
            }
            draggingConnector = nullptr;
            repaint();
        };

        node->onMoved = [this] { repaint(); };
    }
}

AudioPluginAudioProcessorEditor::~AudioPluginAudioProcessorEditor()
{
}

//==============================================================================
void AudioPluginAudioProcessorEditor::paint (juce::Graphics& g)
{
    g.fillAll(juce::Colours::white);

    // Draw permanent connections
    g.setColour(juce::Colours::darkred);

    for (auto& c : connections)
    {
        auto start = c.start->getBounds().getCentre() + c.start->getParentComponent()->getPosition();
        auto end = c.end->getBounds().getCentre() + c.end->getParentComponent()->getPosition();

        g.drawLine(
            (float)start.x, (float)start.y,
            (float)end.x,   (float)end.y,
            2.0f
        );
    }

    // Draw temporary line while dragging
    if (draggingConnector != nullptr)
    {
        auto start = draggingConnector->getBounds().getCentre() + draggingConnector->getParentComponent()->getPosition();
        g.drawLine(
            (float)start.x, (float)start.y,
            (float)dragPoint.x, (float)dragPoint.y,
            2.0f
        );
    }
}

void AudioPluginAudioProcessorEditor::resized()
{
    // This is generally where you'll want to lay out the positions of any
    // subcomponents in your editor..
}
