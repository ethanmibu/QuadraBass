#include "BandSplitProcessor.h"

namespace qbdsp {

void BandSplitProcessor::prepare(const juce::dsp::ProcessSpec& spec) {
    spec_ = spec;
    lpFilter_.setType(juce::dsp::LinkwitzRileyFilterType::lowpass);
    hpFilter_.setType(juce::dsp::LinkwitzRileyFilterType::highpass);
    lpFilter_.prepare(spec);
    hpFilter_.prepare(spec);
}

void BandSplitProcessor::reset() noexcept {
    lpFilter_.reset();
    hpFilter_.reset();
}

void BandSplitProcessor::process(const juce::AudioBuffer<float>& inputBuffer, juce::AudioBuffer<float>& lowBuffer,
                                 juce::AudioBuffer<float>& highBuffer, float crossoverHz,
                                 bool crossoverEnabled) noexcept {
    const auto numSamples = inputBuffer.getNumSamples();
    const auto numChannels = inputBuffer.getNumChannels();

    if (!crossoverEnabled) {
        lowBuffer.clear();
        for (int ch = 0; ch < numChannels; ++ch) {
            highBuffer.copyFrom(ch, 0, inputBuffer, ch, 0, numSamples);
        }
        return;
    }

    lpFilter_.setCutoffFrequency(crossoverHz);
    hpFilter_.setCutoffFrequency(crossoverHz);

    for (int ch = 0; ch < numChannels; ++ch) {
        lowBuffer.copyFrom(ch, 0, inputBuffer, ch, 0, numSamples);
        highBuffer.copyFrom(ch, 0, inputBuffer, ch, 0, numSamples);
    }

    juce::dsp::AudioBlock<float> lowBlock(lowBuffer);
    juce::dsp::ProcessContextReplacing<float> lowCtx(lowBlock);
    lpFilter_.process(lowCtx);

    juce::dsp::AudioBlock<float> highBlock(highBuffer);
    juce::dsp::ProcessContextReplacing<float> highCtx(highBlock);
    hpFilter_.process(highCtx);
}

} // namespace qbdsp
