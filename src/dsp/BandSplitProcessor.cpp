#include "BandSplitProcessor.h"

namespace dsp {

void BandSplitProcessor::prepare(const juce::dsp::ProcessSpec& spec) {
    spec_ = spec;
}

void BandSplitProcessor::reset() noexcept {
}

void BandSplitProcessor::process(juce::AudioBuffer<float>& monoBuffer, float crossoverHz) noexcept {
    juce::ignoreUnused(monoBuffer, crossoverHz, spec_);
    // Placeholder: final Linkwitz-Riley split will be added in later milestones.
}

} // namespace dsp
