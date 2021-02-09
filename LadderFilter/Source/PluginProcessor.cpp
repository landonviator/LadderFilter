/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
LadderFilterAudioProcessor::LadderFilterAudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
     : AudioProcessor (BusesProperties()
                     #if ! JucePlugin_IsMidiEffect
                      #if ! JucePlugin_IsSynth
                       .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                      #endif
                       .withOutput ("Output", juce::AudioChannelSet::stereo(), true)
                     #endif
                       ),
treeState (*this, nullptr, "PARAMETER", createParameterLayout())
#endif
{
}

LadderFilterAudioProcessor::~LadderFilterAudioProcessor()
{
}

juce::AudioProcessorValueTreeState::ParameterLayout LadderFilterAudioProcessor::createParameterLayout()
{
    std::vector <std::unique_ptr<juce::RangedAudioParameter>> params;
    params.reserve(3);
    
    
    auto driveParam = std::make_unique<juce::AudioParameterInt>(driveSliderId, driveSliderName, 1, 10, 1);
    auto cutoffParam = std::make_unique<juce::AudioParameterInt>(cutoffSliderId, cutoffSliderName, 20, 20000, 500);
    auto resoParam = std::make_unique<juce::AudioParameterInt>(resoDelaySliderId, resoDelaySliderName, 0, 10, 0);

    params.push_back(std::move(driveParam));
    params.push_back(std::move(cutoffParam));
    params.push_back(std::move(resoParam));
    
    return { params.begin(), params.end() };
}

//==============================================================================
const juce::String LadderFilterAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool LadderFilterAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool LadderFilterAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool LadderFilterAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double LadderFilterAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int LadderFilterAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int LadderFilterAudioProcessor::getCurrentProgram()
{
    return 0;
}

void LadderFilterAudioProcessor::setCurrentProgram (int index)
{
}

const juce::String LadderFilterAudioProcessor::getProgramName (int index)
{
    return {};
}

void LadderFilterAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
}

//==============================================================================
void LadderFilterAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    juce::dsp::ProcessSpec spec;
    spec.maximumBlockSize = samplesPerBlock;
    spec.sampleRate = sampleRate;
    spec.numChannels = getTotalNumOutputChannels();
    
    ladderProcessor.prepare(spec);
}

void LadderFilterAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool LadderFilterAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
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
#endif

void LadderFilterAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
    auto totalNumInputChannels  = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();

    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear (i, 0, buffer.getNumSamples());

    juce::dsp::AudioBlock<float> audioBlock {buffer};
        
    auto* rawDrive = treeState.getRawParameterValue(driveSliderId);
    auto* rawCutoff = treeState.getRawParameterValue(cutoffSliderId);
    auto* rawReso = treeState.getRawParameterValue(resoDelaySliderId);
        
    ladderProcessor.setDrive(*rawDrive);
    ladderProcessor.setCutoffFrequencyHz(*rawCutoff);
    ladderProcessor.setResonance(*rawReso);
        
    ladderProcessor.process(juce::dsp::ProcessContextReplacing<float> (audioBlock));
}

//==============================================================================
bool LadderFilterAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* LadderFilterAudioProcessor::createEditor()
{
    return new LadderFilterAudioProcessorEditor (*this);
}

//==============================================================================
void LadderFilterAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    juce::MemoryOutputStream stream(destData, false);
    treeState.state.writeToStream (stream);
}

void LadderFilterAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    juce::ValueTree tree = juce::ValueTree::readFromData (data, size_t (sizeInBytes));
    if (tree.isValid()) {
        treeState.state = tree;
        }
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new LadderFilterAudioProcessor();
}
