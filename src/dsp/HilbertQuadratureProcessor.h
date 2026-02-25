#pragma once

#include <juce_dsp/juce_dsp.h>

namespace qbdsp {

class HilbertQuadratureProcessor final {
  public:
    void prepare(const juce::dsp::ProcessSpec& spec);
    void reset() noexcept;
    void process(juce::AudioBuffer<float>& iBuffer, juce::AudioBuffer<float>& qBuffer, float phaseAngleDeg) noexcept;

  private:
    juce::dsp::ProcessSpec spec_{};

    // 4 stages per branch
    float coeffsI_[4] = {0};
    float coeffsQ_[4] = {0};

    float stateI_[4] = {0};
    float stateQ_[4] = {0};
};

} // namespace qbdsp
