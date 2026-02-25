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
    return "QuadraBass";
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

    bandSplit_.prepare(processSpec_);
    hilbert_.prepare(processSpec_);
    stereoMatrix_.prepare(processSpec_);

    const int bufferSize = juce::jmax(1, samplesPerBlock);
    monoBuffer_.setSize(1, bufferSize, false, true, true);
    lowBuffer_.setSize(1, bufferSize, false, true, true);
    xHighBuffer_.setSize(1, bufferSize, false, true, true);
    highBuffer_.setSize(1, bufferSize, false, true, true);
    qBuffer_.setSize(1, bufferSize, false, true, true);
}

void QuadraBassAudioProcessor::releaseResources() {
    bandSplit_.reset();
    hilbert_.reset();
    stereoMatrix_.reset();
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
    const int samples = buffer.getNumSamples();

    for (int channel = totalNumInputChannels; channel < totalNumOutputChannels; ++channel)
        buffer.clear(channel, 0, buffer.getNumSamples());

    if (totalNumInputChannels <= 0 || samples <= 0)
        return;

    monoBuffer_.setSize(1, samples, false, false, true);
    lowBuffer_.setSize(1, samples, false, false, true);
    xHighBuffer_.setSize(1, samples, false, false, true);
    highBuffer_.setSize(1, samples, false, false, true);
    qBuffer_.setSize(1, samples, false, false, true);

    float* monoData = monoBuffer_.getWritePointer(0);
    juce::FloatVectorOperations::clear(monoData, samples);

    const float mixScale = 1.0f / static_cast<float>(totalNumInputChannels);
    for (int channel = 0; channel < totalNumInputChannels; ++channel)
        juce::FloatVectorOperations::addWithMultiply(monoData, buffer.getReadPointer(channel), mixScale, samples);

    bandSplit_.process(monoBuffer_, lowBuffer_, highBuffer_, params_.getCrossoverHz(), params_.getCrossoverEnabled());

    xHighBuffer_.copyFrom(0, 0, highBuffer_, 0, 0, samples);
    hilbert_.process(highBuffer_, qBuffer_, params_.getPhaseAngleDeg());
    stereoMatrix_.process(lowBuffer_, xHighBuffer_, highBuffer_, qBuffer_, buffer, params_.getWidthPercent(),
                          params_.getPhaseAngleDeg(), params_.getPhaseRotationDeg());

    outputGain_.setGainDecibels(params_.getOutputGainDb());
    juce::dsp::AudioBlock<float> block(buffer);
    juce::dsp::ProcessContextReplacing<float> context(block);
    outputGain_.process(context);

    if (auto* gonio = static_cast<qbui::GoniometerComponent*>(activeGoniometer_.load(std::memory_order_relaxed)))
        gonio->processBlock(buffer.getReadPointer(0),
                            buffer.getNumChannels() > 1 ? buffer.getReadPointer(1) : buffer.getReadPointer(0), samples);

    if (auto* corr = static_cast<qbui::CorrelationMeter*>(activeCorrelationMeter_.load(std::memory_order_relaxed)))
        corr->processBlock(buffer.getReadPointer(0),
                           buffer.getNumChannels() > 1 ? buffer.getReadPointer(1) : buffer.getReadPointer(0), samples);
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
