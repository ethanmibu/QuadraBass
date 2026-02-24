#include "HilbertProcessor.h"

namespace dsp {

void HilbertProcessor::prepare(const juce::dsp::ProcessSpec& spec) {
    spec_ = spec;
}

void HilbertProcessor::reset() noexcept {
}

void HilbertProcessor::process(const juce::AudioBuffer<float>& iBuffer, juce::AudioBuffer<float>& qBuffer,
                               float phaseAngleDeg) const {
    juce::ignoreUnused(phaseAngleDeg, spec_);
    qBuffer.makeCopyOf(iBuffer, true);
}

} // namespace dsp
