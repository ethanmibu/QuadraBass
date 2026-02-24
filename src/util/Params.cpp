#include "Params.h"

namespace util {

Params::Params(juce::AudioProcessor& processor) : apvts(processor, nullptr, "PARAMS", createLayout()) {
    crossoverEnabled_ = apvts.getRawParameterValue(IDs::crossoverEnabled);
    crossoverHz_ = apvts.getRawParameterValue(IDs::crossoverHz);
    widthPercent_ = apvts.getRawParameterValue(IDs::widthPercent);
    phaseAngleDeg_ = apvts.getRawParameterValue(IDs::phaseAngleDeg);
    phaseRotationDeg_ = apvts.getRawParameterValue(IDs::phaseRotationDeg);
    outputGainDb_ = apvts.getRawParameterValue(IDs::outputGainDb);

    jassert(crossoverEnabled_ != nullptr);
    jassert(crossoverHz_ != nullptr);
    jassert(widthPercent_ != nullptr);
    jassert(phaseAngleDeg_ != nullptr);
    jassert(phaseRotationDeg_ != nullptr);
    jassert(outputGainDb_ != nullptr);
}

bool Params::getCrossoverEnabled() const noexcept {
    return crossoverEnabled_->load(std::memory_order_relaxed) >= 0.5f;
}

float Params::getCrossoverHz() const noexcept {
    return crossoverHz_->load(std::memory_order_relaxed);
}

float Params::getWidthPercent() const noexcept {
    return widthPercent_->load(std::memory_order_relaxed);
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

    parameters.push_back(
        std::make_unique<juce::AudioParameterBool>(juce::ParameterID(IDs::crossoverEnabled, 1), "Crossover On", true));

    juce::NormalisableRange<float> crossoverRange(20.0f, 500.0f, 0.01f);
    crossoverRange.setSkewForCentre(100.0f);
    parameters.push_back(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID(IDs::crossoverHz, 1),
                                                                     "Crossover", crossoverRange, 90.0f));

    parameters.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID(IDs::widthPercent, 1), "Width", juce::NormalisableRange<float>(0.0f, 100.0f, 0.01f), 0.0f));

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
