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
    mainProcessor->setPlayConfigDetails (getMainBusNumInputChannels(),
        getMainBusNumOutputChannels(),
        sampleRate,
        samplesPerBlock);
    mainProcessor->prepareToPlay (sampleRate, samplesPerBlock);
    initialiseGraph();

    std::vector<Node::Ptr> node(2);
    for (long unsigned int i = 0; i < 2; ++i)
        node[i] = mainProcessor->addNode (std::make_unique<OscillatorProcessor>());

    auto granulatorNode = mainProcessor->addNode (std::make_unique<GranulatorProcessor>());

    mainProcessor->addConnection ({ {node[0]->nodeID, 0}, {audioOutputNode->nodeID, 0} });
    mainProcessor->addConnection ({ {node[0]->nodeID, 0}, {audioOutputNode->nodeID, 1} });
    mainProcessor->addConnection ({ {node[0]->nodeID, 0}, {granulatorNode->nodeID, 2} });

    mainProcessor->addConnection ({ {audioInputNode->nodeID, 0}, {granulatorNode->nodeID, 0} });
    mainProcessor->addConnection ({ {audioInputNode->nodeID, 1}, {granulatorNode->nodeID, 1} });
    //mainProcessor->addConnection ({ {granulatorNode->nodeID, 0}, {audioOutputNode->nodeID, 0} });
    //mainProcessor->addConnection ({ {granulatorNode->nodeID, 1}, {audioOutputNode->nodeID, 1} });
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
            graphState = juce::ValueTree::fromXml(*xml);
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new ModularGranularSynthesizer();
}
