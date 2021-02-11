/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
Ladder_Filter_WAudioProcessor::Ladder_Filter_WAudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
    : AudioProcessor(BusesProperties()
#if ! JucePlugin_IsMidiEffect
#if ! JucePlugin_IsSynth
        .withInput("Input", juce::AudioChannelSet::stereo(), true)
#endif
        .withOutput("Output", juce::AudioChannelSet::stereo(), true)
#endif
    ),
    treeState(*this, nullptr, "PARAMETER", createParameterLayout())
#endif
{
}

Ladder_Filter_WAudioProcessor::~Ladder_Filter_WAudioProcessor()
{
}

juce::AudioProcessorValueTreeState::ParameterLayout Ladder_Filter_WAudioProcessor::createParameterLayout()
{
    std::vector <std::unique_ptr<juce::RangedAudioParameter>> params;
    params.reserve(4);


    auto driveParam = std::make_unique<juce::AudioParameterFloat>(driveSliderId, driveSliderName, 0.0, 10.0, 0.0);
    auto cutoffParam = std::make_unique<juce::AudioParameterInt>(cutoffSliderId, cutoffSliderName, 20, 20000, 750);
    auto resoParam = std::make_unique<juce::AudioParameterFloat>(resoDelaySliderId, resoDelaySliderName, 0.0, 1.0, 0.5);
    auto trimParam = std::make_unique<juce::AudioParameterFloat>(trimSliderId, trimSliderName, -36.0, 36.0, 0.0);

    params.push_back(std::move(driveParam));
    params.push_back(std::move(cutoffParam));
    params.push_back(std::move(resoParam));
    params.push_back(std::move(trimParam));

    return { params.begin(), params.end() };
}

//==============================================================================
const juce::String Ladder_Filter_WAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool Ladder_Filter_WAudioProcessor::acceptsMidi() const
{
#if JucePlugin_WantsMidiInput
    return true;
#else
    return false;
#endif
}

bool Ladder_Filter_WAudioProcessor::producesMidi() const
{
#if JucePlugin_ProducesMidiOutput
    return true;
#else
    return false;
#endif
}

bool Ladder_Filter_WAudioProcessor::isMidiEffect() const
{
#if JucePlugin_IsMidiEffect
    return true;
#else
    return false;
#endif
}

double Ladder_Filter_WAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int Ladder_Filter_WAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int Ladder_Filter_WAudioProcessor::getCurrentProgram()
{
    return 0;
}

void Ladder_Filter_WAudioProcessor::setCurrentProgram(int index)
{
}

const juce::String Ladder_Filter_WAudioProcessor::getProgramName(int index)
{
    return {};
}

void Ladder_Filter_WAudioProcessor::changeProgramName(int index, const juce::String& newName)
{
}

//==============================================================================
void Ladder_Filter_WAudioProcessor::prepareToPlay(double sampleRate, int samplesPerBlock)
{
    juce::dsp::ProcessSpec spec;
    spec.maximumBlockSize = samplesPerBlock;
    spec.sampleRate = sampleRate;
    spec.numChannels = getTotalNumOutputChannels();

    ladderProcessor.prepare(spec);
    ladderProcessor.setDrive(1.0);

    trimProcessor.prepare(spec);
}

void Ladder_Filter_WAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool Ladder_Filter_WAudioProcessor::isBusesLayoutSupported(const BusesLayout& layouts) const
{
#if JucePlugin_IsMidiEffect
    juce::ignoreUnused(layouts);
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

void Ladder_Filter_WAudioProcessor::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
    auto totalNumInputChannels = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();

    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear(i, 0, buffer.getNumSamples());

    auto* rawDrive = treeState.getRawParameterValue(driveSliderId);
    auto* rawCutoff = treeState.getRawParameterValue(cutoffSliderId);
    auto* rawReso = treeState.getRawParameterValue(resoDelaySliderId);
    auto* rawTrim = treeState.getRawParameterValue(trimSliderId);

    for (int channel = 0; channel < totalNumInputChannels; ++channel) {
        auto* outputData = buffer.getWritePointer(channel);
        auto* inputData = buffer.getReadPointer(channel);

        for (int sample = 0; sample < buffer.getNumSamples(); sample++) {
            outputData[sample] = softClip(inputData[sample], *rawDrive * 5);
        }
    }

    juce::dsp::AudioBlock<float> audioBlock{ buffer };

    ladderProcessor.setCutoffFrequencyHz(*rawCutoff);
    ladderProcessor.setResonance(*rawReso);

    trimProcessor.setGainDecibels(*rawTrim);

    ladderProcessor.process(juce::dsp::ProcessContextReplacing<float>(audioBlock));
    trimProcessor.process(juce::dsp::ProcessContextReplacing<float>(audioBlock));
}

float Ladder_Filter_WAudioProcessor::softClip(const float& input, const float& drive) {

    //1.5f to account for drop in gain from the saturation initial state
    //pow(10, (-1 * drive) * 0.04f) to account for the increase in gain when the drive goes up

    return piDivisor * atan(pow(10, drive * 0.05f) * input) * 1.5f * pow(10, (-1 * drive) * 0.04f);
}

//==============================================================================
bool Ladder_Filter_WAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* Ladder_Filter_WAudioProcessor::createEditor()
{
    return new Ladder_Filter_WAudioProcessorEditor(*this);
}

//==============================================================================
void Ladder_Filter_WAudioProcessor::getStateInformation(juce::MemoryBlock& destData)
{
    juce::MemoryOutputStream stream(destData, false);
    treeState.state.writeToStream(stream);
}

void Ladder_Filter_WAudioProcessor::setStateInformation(const void* data, int sizeInBytes)
{
    juce::ValueTree tree = juce::ValueTree::readFromData(data, size_t(sizeInBytes));
    if (tree.isValid()) {
        treeState.state = tree;
    }
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new Ladder_Filter_WAudioProcessor();
}
