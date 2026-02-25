#include "Params.h"

namespace util {

Params::Params(juce::AudioProcessor& processor) : apvts(processor, nullptr, "PARAMS", createLayout()) {
    widthPercent_ = apvts.getRawParameterValue(IDs::widthPercent);
    hilbertMode_ = apvts.getRawParameterValue(IDs::hilbertMode);
    phaseAngleDeg_ = apvts.getRawParameterValue(IDs::phaseAngleDeg);
    phaseRotationDeg_ = apvts.getRawParameterValue(IDs::phaseRotationDeg);
    outputGainDb_ = apvts.getRawParameterValue(IDs::outputGainDb);

    jassert(widthPercent_ != nullptr);
    jassert(hilbertMode_ != nullptr);
    jassert(phaseAngleDeg_ != nullptr);
    jassert(phaseRotationDeg_ != nullptr);
    jassert(outputGainDb_ != nullptr);
}

float Params::getWidthPercent() const noexcept {
    return widthPercent_->load(std::memory_order_relaxed);
}

int Params::getHilbertModeIndex() const noexcept {
    const int idx = static_cast<int>(hilbertMode_->load(std::memory_order_relaxed));
    return juce::jlimit(0, 1, idx);
}

float Params::getPhaseAngleDeg() const noexcept {
    return phaseAngleDeg_->load(std::memory_order_relaxed);
}

float Params::getPhaseRotationDeg() const noexcept {
    return phaseRotationDeg_->load(std::memory_order_relaxed);
}

float Params::getOutputGainDb() const noexcept {
    return outputGainDb_->load(std::memory_order_relaxed);
}

juce::AudioProcessorValueTreeState::ParameterLayout Params::createLayout() {
    std::vector<std::unique_ptr<juce::RangedAudioParameter>> parameters;

    parameters.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID(IDs::widthPercent, 1), "Width", juce::NormalisableRange<float>(0.0f, 100.0f, 0.01f), 0.0f));

    parameters.push_back(std::make_unique<juce::AudioParameterChoice>(
        juce::ParameterID(IDs::hilbertMode, 1), "Hilbert Mode", juce::StringArray{"IIR", "FIR"}, 0));

    parameters.push_back(
        std::make_unique<juce::AudioParameterFloat>(juce::ParameterID(IDs::phaseAngleDeg, 1), "Phase Angle",
                                                    juce::NormalisableRange<float>(0.0f, 180.0f, 0.01f), 90.0f));

    parameters.push_back(
        std::make_unique<juce::AudioParameterFloat>(juce::ParameterID(IDs::phaseRotationDeg, 1), "Phase Rotation",
                                                    juce::NormalisableRange<float>(-180.0f, 180.0f, 0.01f), 0.0f));

    parameters.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID(IDs::outputGainDb, 1), "Gain", juce::NormalisableRange<float>(-60.0f, 12.0f, 0.01f), 0.0f));

    return {parameters.begin(), parameters.end()};
}

} // namespace util
