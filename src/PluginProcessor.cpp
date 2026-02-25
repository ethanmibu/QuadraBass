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

    hilbert_.prepare(processSpec_);
    activeHilbertMode_ = params_.getHilbertModeIndex() == static_cast<int>(qbdsp::HilbertQuadratureProcessor::Mode::FIR)
                             ? qbdsp::HilbertQuadratureProcessor::Mode::FIR
                             : qbdsp::HilbertQuadratureProcessor::Mode::IIR;
    hilbert_.setMode(activeHilbertMode_);
    setLatencySamples(hilbert_.getLatencySamples());
    stereoMatrix_.prepare(processSpec_);

    const int bufferSize = juce::jmax(1, samplesPerBlock);
    monoBuffer_.setSize(1, bufferSize, false, true, true);
    xHighBuffer_.setSize(1, bufferSize, false, true, true);
    qBuffer_.setSize(1, bufferSize, false, true, true);
    zeroBuffer_.setSize(1, bufferSize, false, true, true);
}

void QuadraBassAudioProcessor::releaseResources() {
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
    xHighBuffer_.setSize(1, samples, false, false, true);
    qBuffer_.setSize(1, samples, false, false, true);
    zeroBuffer_.setSize(1, samples, false, false, true);
    zeroBuffer_.clear();

    auto downmixToMono = [samples](const juce::AudioBuffer<float>& src, int srcChannels,
                                   juce::AudioBuffer<float>& dst) noexcept {
        float* monoData = dst.getWritePointer(0);
        juce::FloatVectorOperations::clear(monoData, samples);
        if (srcChannels <= 0)
            return;

        const float mixScale = 1.0f / static_cast<float>(srcChannels);
        for (int ch = 0; ch < srcChannels; ++ch)
            juce::FloatVectorOperations::addWithMultiply(monoData, src.getReadPointer(ch), mixScale, samples);
    };

    // Keep widening full-band so width behavior stays consistent across the spectrum.
    downmixToMono(buffer, totalNumInputChannels, monoBuffer_);

    const auto requestedMode =
        params_.getHilbertModeIndex() == static_cast<int>(qbdsp::HilbertQuadratureProcessor::Mode::FIR)
            ? qbdsp::HilbertQuadratureProcessor::Mode::FIR
            : qbdsp::HilbertQuadratureProcessor::Mode::IIR;
    if (requestedMode != activeHilbertMode_) {
        activeHilbertMode_ = requestedMode;
        hilbert_.setMode(activeHilbertMode_);
        setLatencySamples(hilbert_.getLatencySamples());
    }

    xHighBuffer_.copyFrom(0, 0, monoBuffer_, 0, 0, samples);
    hilbert_.process(monoBuffer_, qBuffer_, params_.getPhaseAngleDeg());
    stereoMatrix_.process(zeroBuffer_, xHighBuffer_, monoBuffer_, qBuffer_, buffer, params_.getWidthPercent(),
                          params_.getPhaseAngleDeg(), params_.getPhaseRotationDeg(),
                          activeHilbertMode_ == qbdsp::HilbertQuadratureProcessor::Mode::FIR);

    if (totalNumOutputChannels == 1 && buffer.getNumChannels() > 1)
        buffer.clear(1, 0, samples);

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
