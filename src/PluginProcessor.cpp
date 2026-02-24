#include "PluginProcessor.h"
#include "PluginEditor.h"

QuadraBassAudioProcessor::QuadraBassAudioProcessor()
    : AudioProcessor(BusesProperties()
#if !JucePlugin_IsMidiEffect
#if !JucePlugin_IsSynth
                         .withInput("Input", juce::AudioChannelSet::stereo(), true)
#endif
                         .withOutput("Output", juce::AudioChannelSet::stereo(), true)
#endif
                         ),
      params_(*this) {
}

const juce::String QuadraBassAudioProcessor::getName() const {
    return JucePlugin_Name;
}

bool QuadraBassAudioProcessor::acceptsMidi() const {
    return false;
}

bool QuadraBassAudioProcessor::producesMidi() const {
    return false;
}

bool QuadraBassAudioProcessor::isMidiEffect() const {
    return false;
}

double QuadraBassAudioProcessor::getTailLengthSeconds() const {
    return 0.0;
}

int QuadraBassAudioProcessor::getNumPrograms() {
    return 1;
}

int QuadraBassAudioProcessor::getCurrentProgram() {
    return 0;
}

void QuadraBassAudioProcessor::setCurrentProgram(int index) {
    juce::ignoreUnused(index);
}

const juce::String QuadraBassAudioProcessor::getProgramName(int index) {
    juce::ignoreUnused(index);
    return {};
}

void QuadraBassAudioProcessor::changeProgramName(int index, const juce::String& newName) {
    juce::ignoreUnused(index, newName);
}

void QuadraBassAudioProcessor::prepareToPlay(double sampleRate, int samplesPerBlock) {
    processSpec_.sampleRate = sampleRate;
    processSpec_.maximumBlockSize = static_cast<juce::uint32>(samplesPerBlock);
    processSpec_.numChannels =
        static_cast<juce::uint32>(juce::jmax(getTotalNumInputChannels(), getTotalNumOutputChannels()));

    outputGain_.reset();
    outputGain_.prepare(processSpec_);
    outputGain_.setRampDurationSeconds(0.02);
    outputGain_.setGainDecibels(params_.getOutputGainDb());
}

void QuadraBassAudioProcessor::releaseResources() {
}

#if !JucePlugin_PreferredChannelConfigurations
bool QuadraBassAudioProcessor::isBusesLayoutSupported(const BusesLayout& layouts) const {
    const auto& mainIn = layouts.getMainInputChannelSet();
    const auto& mainOut = layouts.getMainOutputChannelSet();

    if (mainOut != juce::AudioChannelSet::mono() && mainOut != juce::AudioChannelSet::stereo())
        return false;

#if !JucePlugin_IsSynth
    if (mainIn != mainOut)
        return false;
#endif

    return true;
}
#endif

void QuadraBassAudioProcessor::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages) {
    juce::ignoreUnused(midiMessages);
    juce::ScopedNoDenormals noDenormals;

    const int totalNumInputChannels = getTotalNumInputChannels();
    const int totalNumOutputChannels = getTotalNumOutputChannels();

    for (int channel = totalNumInputChannels; channel < totalNumOutputChannels; ++channel)
        buffer.clear(channel, 0, buffer.getNumSamples());

    outputGain_.setGainDecibels(params_.getOutputGainDb());
    juce::dsp::AudioBlock<float> block(buffer);
    juce::dsp::ProcessContextReplacing<float> context(block);
    outputGain_.process(context);
}

bool QuadraBassAudioProcessor::hasEditor() const {
    return true;
}

juce::AudioProcessorEditor* QuadraBassAudioProcessor::createEditor() {
    return new QuadraBassAudioProcessorEditor(*this);
}

void QuadraBassAudioProcessor::getStateInformation(juce::MemoryBlock& destData) {
    auto state = params_.apvts.copyState();
    std::unique_ptr<juce::XmlElement> xml(state.createXml());
    copyXmlToBinary(*xml, destData);
}

void QuadraBassAudioProcessor::setStateInformation(const void* data, int sizeInBytes) {
    std::unique_ptr<juce::XmlElement> xmlState(getXmlFromBinary(data, sizeInBytes));
    if (xmlState == nullptr)
        return;

    if (!xmlState->hasTagName(params_.apvts.state.getType()))
        return;

    params_.apvts.replaceState(juce::ValueTree::fromXml(*xmlState));
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter() {
    return new QuadraBassAudioProcessor();
}
