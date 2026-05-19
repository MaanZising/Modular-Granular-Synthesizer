#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
ModularGranularSynthesizer::ModularGranularSynthesizer()
     : AudioProcessor (BusesProperties()
                     #if ! JucePlugin_IsMidiEffect
                      #if ! JucePlugin_IsSynth
                       .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                      #endif
                       .withOutput ("Output", juce::AudioChannelSet::stereo(), true)
                     #endif
                       ),
        mainProcessor (new juce::AudioProcessorGraph()) // initiate the main AudioProcessorGraph
{
    graphState.addListener(this);
}

ModularGranularSynthesizer::~ModularGranularSynthesizer()
{
}

//==============================================================================
const juce::String ModularGranularSynthesizer::getName() const
{
    return JucePlugin_Name;
}

bool ModularGranularSynthesizer::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool ModularGranularSynthesizer::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool ModularGranularSynthesizer::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double ModularGranularSynthesizer::getTailLengthSeconds() const
{
    return 0.0;
}

int ModularGranularSynthesizer::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int ModularGranularSynthesizer::getCurrentProgram()
{
    return 0;
}

void ModularGranularSynthesizer::setCurrentProgram (int index)
{
    juce::ignoreUnused (index);
}

const juce::String ModularGranularSynthesizer::getProgramName (int index)
{
    juce::ignoreUnused (index);
    return {};
}

void ModularGranularSynthesizer::changeProgramName (int index, const juce::String& newName)
{
    juce::ignoreUnused (index, newName);
}

//==============================================================================
void ModularGranularSynthesizer::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    mainProcessor->setPlayConfigDetails
    (
        getMainBusNumInputChannels(),
        getMainBusNumOutputChannels(),
        sampleRate,
        samplesPerBlock
    );
    mainProcessor->prepareToPlay (sampleRate, samplesPerBlock);
    initialiseGraph();
}

void ModularGranularSynthesizer::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
    mainProcessor->releaseResources();
}

bool ModularGranularSynthesizer::isBusesLayoutSupported (const BusesLayout& layouts) const
{
  #if JucePlugin_IsMidiEffect
    juce::ignoreUnused (layouts);
    return true;
  #else
    // This is the place where you check if the layout is supported.
    // In this template code we only support mono or stereo.
    // Some plugin hosts, such as certain GarageBand versions, will only
    // load plugins that support stereo bus layouts.
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono()
     && layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;

    // This checks if the input layout matches the output layout
   #if ! JucePlugin_IsSynth
    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
        return false;
   #endif

    return true;
  #endif
}

void ModularGranularSynthesizer::processBlock (juce::AudioBuffer<float>& buffer,
                                              juce::MidiBuffer& midiMessages)
{
    juce::ignoreUnused (midiMessages);

    juce::ScopedNoDenormals noDenormals;
    auto totalNumInputChannels  = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();

    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear (i, 0, buffer.getNumSamples());

    mainProcessor->processBlock (buffer, midiMessages);
}

//==============================================================================
bool ModularGranularSynthesizer::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* ModularGranularSynthesizer::createEditor()
{
    return new AudioPluginAudioProcessorEditor (*this);
}

//==============================================================================
void ModularGranularSynthesizer::getStateInformation (juce::MemoryBlock& destData)
{
    // You should use this method to store your parameters in the memory block.
    // You could do that either as raw data, or use the XML or ValueTree classes
    // as intermediaries to make it easy to save and load complex data.
    juce::ignoreUnused (destData);
    juce::MemoryOutputStream(destData, true).writeString(graphState.toXmlString());
}

void ModularGranularSynthesizer::setStateInformation (const void* data, int sizeInBytes)
{
    // You should use this method to restore your parameters from this memory block,
    // whose contents will have been created by the getStateInformation() call.
    juce::ignoreUnused (data, sizeInBytes);
    auto xml = juce::parseXML(juce::String::fromUTF8((const char*)data, sizeInBytes));
        if (xml != nullptr)
        {
            graphState = juce::ValueTree::fromXml(*xml);
        }
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new ModularGranularSynthesizer();
}






//////////

std::unique_ptr<juce::AudioProcessor> ModularGranularSynthesizer::createProcessorForType(const juce::String& name)
{
    if (name == "Audio Input")
        return std::make_unique<juce::AudioProcessorGraph::AudioGraphIOProcessor> (juce::AudioProcessorGraph::AudioGraphIOProcessor::audioInputNode);

    if (name == "Audio Output")
        return std::make_unique<juce::AudioProcessorGraph::AudioGraphIOProcessor> (juce::AudioProcessorGraph::AudioGraphIOProcessor::audioOutputNode);

    if (name == "Granulator")
        return std::make_unique<GranulatorProcessor>();

    if (name == "Oscillator")
        return std::make_unique<OscillatorProcessor>();
    
    if (name == "Number Box")
        return std::make_unique<NumberBox>();
        
    return nullptr;
}

void ModularGranularSynthesizer::createAudioNodeFromState(juce::ValueTree child)
{
    if (!child.hasType("Node")) return;

    if (child.hasProperty("graphNodeId"))
    {
        auto existingId = (int)child.getProperty("graphNodeId");
        if (mainProcessor->getNodeForId((juce::AudioProcessorGraph::NodeID)existingId) != nullptr)
            return; 
    }

    auto name = child.getProperty("name").toString();
    auto processor = createProcessorForType(name);
    
    if (processor != nullptr)
    {
        // if it's an oscillator, make sure we check if a waveType was already saved
        if (name == "Oscillator")
        {
            int savedType = child.getProperty("waveType", 0); // default to 0 (Sine)
            if (auto* osc = dynamic_cast<OscillatorProcessor*>(processor.get()))
                osc->setWaveType(savedType);
        }

        auto node = mainProcessor->addNode(std::move(processor));
        
        // Use a variant property safely
        child.setProperty("graphNodeId", (int)node->nodeID.uid, nullptr);
        
        // Prepare the node immediately with current processing constraints
        node->getProcessor()->prepareToPlay(getSampleRate(), getBlockSize());
    }
}

// Helper function to find the internal Graph NodeID from your custom GUI ID
juce::AudioProcessorGraph::NodeID ModularGranularSynthesizer::getGraphIdForGuiId(juce::int64 guiId)
{
    for (int i = 0; i < graphState.getNumChildren(); ++i)
    {
        auto child = graphState.getChild(i);
        if (child.hasType("Node") && (juce::int64)child.getProperty("id") == guiId)
        {
            if (child.hasProperty("graphNodeId"))
            {
                return juce::AudioProcessorGraph::NodeID((juce::uint32)(int)child.getProperty("graphNodeId"));
            }
        }
    }
    return {}; // invalid ID
}

void ModularGranularSynthesizer::valueTreeChildAdded(juce::ValueTree& parent, juce::ValueTree& child)
{
    if (child.hasType("Node"))
    {
        createAudioNodeFromState(child);
    }
    else if (child.hasType("Connection"))
    {
        // fetch custom GUI IDs and port indexes from the tree
        juce::int64 sourceGuiId = child.getProperty("sourceId");
        juce::int64 destGuiId   = child.getProperty("destId");
        int sourcePortIndex     = child.getProperty("sourcePort");
        int destPortIndex       = child.getProperty("destPort");

        // translate GUI IDs to the Graph's internal NodeIDs
        auto sourceGraphId = getGraphIdForGuiId(sourceGuiId);
        auto destGraphId   = getGraphIdForGuiId(destGuiId);

        // apply the connection to the audio graph if both nodes exist
        if (sourceGraphId != juce::AudioProcessorGraph::NodeID() && 
            destGraphId   != juce::AudioProcessorGraph::NodeID())
        {
            juce::AudioProcessorGraph::NodeAndChannel sourceNode { sourceGraphId, sourcePortIndex };
            juce::AudioProcessorGraph::NodeAndChannel destNode   { destGraphId,   destPortIndex };
            juce::AudioProcessorGraph::Connection conn           { sourceNode,    destNode };

            if (mainProcessor->canConnect (conn))
            {
                mainProcessor->addConnection (conn);
                juce::Logger::writeToLog("Audio Connection successfully established in backend!");
                mainProcessor->removeIllegalConnections();
            }
            else
            {
                juce::Logger::writeToLog("Audio Connection failed.");
            }
        }
    }
}

void ModularGranularSynthesizer::valueTreeChildRemoved(juce::ValueTree& parent, juce::ValueTree& child, int index)
{
    if (child.hasType("Node"))
    {
        // Find the internal node using the ID we stored
        auto graphId = juce::AudioProcessorGraph::NodeID((juce::int32)child.getProperty("graphNodeId"));
        
        if (auto* node = mainProcessor->getNodeForId(graphId))
        {
            mainProcessor->removeNode(graphId);
        }
    }
    else if (child.hasType("Connection"))
    {
        // translate IDs to find the existing connection
        juce::int64 sourceGuiId = child.getProperty("sourceId");
        juce::int64 destGuiId   = child.getProperty("destId");
        int sourcePortIndex     = child.getProperty("sourcePort");
        int destPortIndex       = child.getProperty("destPort");

        auto sourceGraphId = getGraphIdForGuiId(sourceGuiId);
        auto destGraphId   = getGraphIdForGuiId(destGuiId);

        if (sourceGraphId != juce::AudioProcessorGraph::NodeID() && 
            destGraphId   != juce::AudioProcessorGraph::NodeID())
        {
            juce::AudioProcessorGraph::NodeAndChannel sourceNode { sourceGraphId, sourcePortIndex };
            juce::AudioProcessorGraph::NodeAndChannel destNode   { destGraphId,   destPortIndex };
            juce::AudioProcessorGraph::Connection conn           { sourceNode,    destNode };

            if (mainProcessor->isConnected(conn))
            {
                mainProcessor->removeConnection(conn);
            }
        }
    }
}

void ModularGranularSynthesizer::initialiseGraph()
{
    mainProcessor->clear();
    
    // reload nodes from existing state
    for (int i = 0; i < graphState.getNumChildren(); ++i)
    {
        auto child = graphState.getChild(i);
        if (child.hasType ("Node"))
        {
            auto name = child.getProperty ("name").toString();
            auto processor = createProcessorForType (name);
            
            if (processor != nullptr)
            {
                auto node = mainProcessor->addNode (std::move (processor));
                child.setPropertyExcludingListener (this, "graphNodeId", (int)node->nodeID.uid, nullptr);
                node->getProcessor()->prepareToPlay (getSampleRate(), getBlockSize());
            }
        }
    }

    // reload connections from existing state
    for (int i = 0; i < graphState.getNumChildren(); ++i)
    {
        auto child = graphState.getChild(i);
        
        if (child.hasType ("Connection"))
        {
            juce::int64 sourceGuiId = child.getProperty ("sourceId");
            juce::int64 destGuiId   = child.getProperty ("destId");
            int sourcePortIndex     = child.getProperty ("sourcePort");
            int destPortIndex       = child.getProperty ("destPort");

            auto sourceGraphId = getGraphIdForGuiId (sourceGuiId);
            auto destGraphId   = getGraphIdForGuiId (destGuiId);

            if (sourceGraphId != juce::AudioProcessorGraph::NodeID() && 
                destGraphId   != juce::AudioProcessorGraph::NodeID())
            {
                // Explicit initialization
                juce::AudioProcessorGraph::NodeAndChannel sourceNode { sourceGraphId, sourcePortIndex };
                juce::AudioProcessorGraph::NodeAndChannel destNode   { destGraphId,   destPortIndex };
                juce::AudioProcessorGraph::Connection conn           { sourceNode,    destNode };

                if (mainProcessor->canConnect (conn))
                {
                    mainProcessor->addConnection (conn);
                }
            }
        }
    }
}
