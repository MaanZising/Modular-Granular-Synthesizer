#include "PluginProcessor.h"
#include "PluginEditor.h"
#include "NodeComponent.h"
#include "Oscillator.h"

//==============================================================================
AudioPluginAudioProcessorEditor::AudioPluginAudioProcessorEditor (ModularGranularSynthesizer& p)
    : AudioProcessorEditor (&p), processorRef (p)
{
    //juce::ignoreUnused (processorRef);
    setSize (800, 600);
    setResizable (true, true);

    // initial nodes
    //addNode ("Node A", 1, 2, juce::Rectangle<int>(50, 50, 80, 50));
    //addNode ("Node B", 2, 1, juce::Rectangle<int>(300, 150, 80, 50));

    // load graph from processor state
    auto graph = processorRef.graphState;

    for (int i = 0; i < graph.getNumChildren(); ++i)
    {
        auto nodeTree = graph.getChild(i);

        // only process children that are actually Nodes
        if (!nodeTree.hasType("Node"))
            continue;
        
        juce::Rectangle<int> bounds(nodeTree["x"], nodeTree["y"], nodeTree["w"], nodeTree["h"]);

        auto* node = new NodeComponent(nodeTree["name"].toString(),
                                       (int)nodeTree.getProperty("inputs", 2),
                                       (int)nodeTree.getProperty("outputs", 2));

        // restore ID from saved state
        node->setUniqueId((juce::int64)nodeTree["id"]);

        // update processor’s counter so new nodes don’t collide
        processorRef.nextNodeId = std::max(processorRef.nextNodeId, node->getUniqueId() + 1);

        // restore dropdown menu state for oscillator
        if (nodeTree["name"].toString() == "Oscillator")
        {
            int savedType = nodeTree.getProperty("waveType", 0);
            // JUCE ComboBox indices are 1-based, your processor types are 0-based
            node->setWaveTypeComboBoxId(savedType + 1); 
        }

        nodes.add(node);
        addAndMakeVisible(node);
        node->setBounds(bounds);
        hookUpNode(node);
    }

    // load connections
    for (int i = 0; i < processorRef.graphState.getNumChildren(); ++i)
    {
        auto child = processorRef.graphState.getChild(i);
        if (child.hasType("Connection"))
        {
            auto* sourceNode = findNodeById((juce::int64)child["sourceId"]);
            auto* destNode   = findNodeById((juce::int64)child["destId"]);

            if (sourceNode && destNode)
            {
                auto* sourcePort = sourceNode->getOutputPort((int)child["sourcePort"]);
                auto* destPort   = destNode->getInputPort((int)child["destPort"]);
                connections.push_back({ sourcePort, destPort });
            }
        }
    }
}

AudioPluginAudioProcessorEditor::~AudioPluginAudioProcessorEditor()
{
}

//==============================================================================
void AudioPluginAudioProcessorEditor::paint (juce::Graphics& g)
{
    g.fillAll(lightGrey);

    // Draw permanent connections
    g.setColour(blue);

    for (auto& c : connections)
    {
        if (c.start != nullptr && c.end != nullptr)
        {
            auto start = c.start->getBounds().getCentre() + c.start->getParentComponent()->getPosition();
            auto end = c.end->getBounds().getCentre() + c.end->getParentComponent()->getPosition();

            g.drawLine(
                (float)start.x, (float)start.y,
                (float)end.x,   (float)end.y,
                2.0f
            );
        }
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

void AudioPluginAudioProcessorEditor::removeConnection (ConnectorComponent* p)
{
    // remove from local vector
    connections.erase
    (
        std::remove_if
        (
            connections.begin(),
            connections.end(),
            [p](const Connection& c)
            {
                if (c.start == p || c.end == p)
                {
                    return true;
                }
                return false;
            }
        ),
        connections.end()
    );

    // remove from processor state
    for (int i = processorRef.graphState.getNumChildren(); --i >= 0;)
    {
        auto child = processorRef.graphState.getChild(i);
        if (child.hasType("Connection"))
        {
            if ((juce::int64)child["sourceId"] == p->getParentNode()->getUniqueId() ||
                (juce::int64)child["destId"] == p->getParentNode()->getUniqueId())
            {
                processorRef.graphState.removeChild(i, nullptr);
                break;
            }
        }
    }

    repaint();
}

void AudioPluginAudioProcessorEditor::addNode(const juce::String& name, int numInputs, int numOutputs, juce::Rectangle<int> bounds)
{
    auto* node = new NodeComponent(name, numInputs, numOutputs);
    node->setUniqueId(processorRef.generateNodeId());
    nodes.add (node);
    addAndMakeVisible (node);
    node->setBounds (bounds);
    hookUpNode (node);

    // update processor state
    juce::ValueTree nodeTree("Node");
    nodeTree.setProperty("id", node->getUniqueId(), nullptr);
    nodeTree.setProperty("name", name, nullptr);
    nodeTree.setProperty("x", bounds.getX(), nullptr);
    nodeTree.setProperty("y", bounds.getY(), nullptr);
    nodeTree.setProperty("w", bounds.getWidth(), nullptr);
    nodeTree.setProperty("h", bounds.getHeight(), nullptr);
    nodeTree.setProperty("inputs", numInputs, nullptr);
    nodeTree.setProperty("outputs", numOutputs, nullptr);
    processorRef.graphState.addChild(nodeTree, -1, nullptr);

    repaint();
}

void AudioPluginAudioProcessorEditor::removeNode(NodeComponent* node)
{
    if (node == nullptr) return;

    // explicitly hide and remove from parent view before deleting from memory
    removeChildComponent(node);

    // capture the id before the node is deleted
    const auto idToRemove = node->getUniqueId();
    
    // remove connections involving this node's ports
    connections.erase(
        std::remove_if(connections.begin(), connections.end(),
                       [node](const Connection& c)
                       {
                           return node->inputs.contains(c.start) || node->inputs.contains(c.end) ||
                                  node->outputs.contains(c.start) || node->outputs.contains(c.end);
                       }),
        connections.end());

    // remove all connections involving this node
    auto connsTree = processorRef.graphState.getChildWithName("Connections");
    for (int i = processorRef.graphState.getNumChildren(); --i >= 0;)
    {
        auto child = processorRef.graphState.getChild(i);
        if (child.hasType("Connection"))
        {
            if ((juce::int64)child["sourceId"] == node->getUniqueId() ||
                (juce::int64)child["destId"] == node->getUniqueId())
            {
                processorRef.graphState.removeChild(i, nullptr);
                break;
            }
        }
    }

    // remove node from processor state
    for (int i = processorRef.graphState.getNumChildren(); --i >= 0;)
    {
        auto child = processorRef.graphState.getChild(i);
        if (child.hasType("Node") && (juce::int64)child["id"] == idToRemove)
        {
            processorRef.graphState.removeChild(child, nullptr);
            break;
        }
    }

    // remove node from OwnedArray (this deletes the node automatically)
    nodes.removeObject(node);
    repaint();
}

void AudioPluginAudioProcessorEditor::mouseDown (const juce::MouseEvent& e)
{
    if (e.mods.isRightButtonDown())
    {
        // check if click was on empty canvas (not on a node)
        bool clickedOnNode = false;
        for (auto* node : nodes)
        {
            if (node->getBounds().contains(e.getPosition()))
            {
                clickedOnNode = true;
                break;
            }
        }

        if (!clickedOnNode)
        {
            juce::PopupMenu menu;
            menu.addItem(1, "Audio Input");
            menu.addItem(2, "Audio Output");
            menu.addItem(3, "Granulator");
            menu.addItem(4, "Oscillator");
            menu.addItem(5, "Number Box");

            menu.showMenuAsync
            (
                juce::PopupMenu::Options()
                    .withTargetComponent(this)
                    .withTargetScreenArea(juce::Rectangle<int>(e.getScreenX(), e.getScreenY(), 1, 1)),
                [this, pos = e.getPosition()](int result)
                {
                    switch (result)
                    {
                    case 0:
                        return; // cancelled
                    case 1:
                        addNode ("Audio Input", 0, 2, juce::Rectangle<int>(pos.x, pos.y, 120, 50));
                        break;
                    case 2:
                        addNode ("Audio Output", 2, 0, juce::Rectangle<int>(pos.x, pos.y, 120, 50));
                        break;
                    case 3:
                        addNode ("Granulator", 6, 2, juce::Rectangle<int>(pos.x, pos.y, 100, 150));
                        break;
                    case 4:
                        addNode ("Oscillator", 1, 1, juce::Rectangle<int>(pos.x, pos.y, 160, 35));
                        break;
                    case 5:
                        addNode ("Number Box", 0, 1, juce::Rectangle<int>(pos.x, pos.y, 60, 35));
                        break;
                    }
                }
            );
        }
    }
}

void AudioPluginAudioProcessorEditor::hookUpNode (NodeComponent* node)
{
    auto hook = [this](ConnectorComponent* c, const juce::MouseEvent& e, bool finish)
    {
        if (!finish) // dragging
        {
            draggingConnector = c;
            dragPoint = e.getEventRelativeTo(this).getPosition();
            repaint();
        }
        else // finished
        {
            ConnectorComponent* sourcePort = nullptr;
            ConnectorComponent* destPort = nullptr;

            // search all nodes for a valid target port at the mouse position
            for (auto* otherNode : nodes)
            {
                // look for input when dragging output, look for output when dragging input
                auto& targetPorts = (c->getType() == ConnectorType::Output) ? otherNode->inputs : otherNode->outputs;

                for (auto* target : targetPorts)
                {
                    if (target->getBounds().contains(e.getEventRelativeTo(otherNode).getPosition()))
                    {
                        // implement directionality
                        if (c->getType() == ConnectorType::Output)
                        {
                            sourcePort = c;
                            destPort = target;
                        }
                        else
                        {
                            sourcePort = target;
                            destPort = c;
                        }
                        break;
                    }
                }
                if (destPort != nullptr) break;
            }

            // create the connection if a valid pair was found 
            if (sourcePort != nullptr && destPort != nullptr)
            {
                // prevent duplicate connections
                bool exists = std::any_of(connections.begin(), connections.end(),
                    [sourcePort, destPort](const Connection& conn) {
                        return conn.start == sourcePort && conn.end == destPort;
                    });

                if (!exists)
                {
                    connections.push_back({ sourcePort, destPort });

                    // persist to processor state
                    juce::ValueTree connTree("Connection");
                    connTree.setProperty("sourceId", sourcePort->getParentNode()->getUniqueId(), nullptr);
                    connTree.setProperty("sourcePort", sourcePort->getIndex(), nullptr);
                    connTree.setProperty("destId", destPort->getParentNode()->getUniqueId(), nullptr);
                    connTree.setProperty("destPort", destPort->getIndex(), nullptr);
                    
                    processorRef.graphState.addChild(connTree, -1, nullptr);
                }
            }

            draggingConnector = nullptr;
            repaint();
        }
    };

    for (auto* port : node->inputs)
    {
        port->onStartDrag = [hook](ConnectorComponent* c, const juce::MouseEvent& e){ hook(c,e,false); };
        port->onDrag = [hook](ConnectorComponent* c, const juce::MouseEvent& e){ hook(c,e,false); };
        port->onFinishDrag = [hook](ConnectorComponent* c, const juce::MouseEvent& e){ hook(c,e,true); };
        port->onRightClick = [this](ConnectorComponent* p){ removeConnection (p); };
    }
    for (auto* port : node->outputs)
    {
        port->onStartDrag = [hook](ConnectorComponent* c, const juce::MouseEvent& e){ hook(c,e,false); };
        port->onDrag = [hook](ConnectorComponent* c, const juce::MouseEvent& e){ hook(c,e,false); };
        port->onFinishDrag = [hook](ConnectorComponent* c, const juce::MouseEvent& e){ hook(c,e,true); };
        port->onRightClick = [this](ConnectorComponent* p){ removeConnection (p); };
    }

    // wave type selection
    node->onWaveTypeChanged = [this, node](int zeroBasedWaveType)
    {
        // 1. Update the ValueTree so it saves/restores
        for (int i = 0; i < processorRef.graphState.getNumChildren(); ++i)
        {
            auto child = processorRef.graphState.getChild(i);
            if (child.hasType("Node") && (juce::int64)child["id"] == node->getUniqueId())
            {
                child.setProperty("waveType", zeroBasedWaveType, nullptr);
                break;
            }
        }

        // 2. Push the change directly to the audio engine processor instantly
        auto graphId = processorRef.getGraphIdForGuiId(node->getUniqueId());
        if (auto* graphNode = processorRef.mainProcessor->getNodeForId(graphId))
        {
            if (auto* oscProc = dynamic_cast<OscillatorProcessor*>(graphNode->getProcessor()))
            {
                oscProc->setWaveType(zeroBasedWaveType);
            }
        }
    };

    // node movement repaint
    node->onMoved = [this] { repaint(); };
    
    // remove node
    node->onContextMenu = [this](NodeComponent* n)
    {
        removeNode(n);
    };
}

//////////////////////////////////////////////////////////////////

NodeComponent* AudioPluginAudioProcessorEditor::findNodeById(juce::int64 id)
{
    for (auto* node : nodes)
    {
        if (node->getUniqueId() == id)
            return node;
    }
    return nullptr; // not found
}

void AudioPluginAudioProcessorEditor::updateNodePosition(NodeComponent* node)
{
    // find the corresponding ValueTree in processor.graphState
    for (int i = 0; i < processorRef.graphState.getNumChildren(); ++i)
    {
        auto child = processorRef.graphState.getChild(i);
        if ((juce::int64)child["id"] == node->getUniqueId())
        {
            auto b = node->getBounds();
            child.setProperty("x", b.getX(), nullptr);
            child.setProperty("y", b.getY(), nullptr);
            child.setProperty("w", b.getWidth(), nullptr);
            child.setProperty("h", b.getHeight(), nullptr);
            break;
        }
    }
}

void AudioPluginAudioProcessorEditor::childBoundsChanged(Component* child)
{
    if (auto* node = dynamic_cast<NodeComponent*>(child))
        updateNodePosition(node);
}
