#pragma once

#include <JuceHeader.h>

namespace qbdsp {

class HilbertProcessor final {
  public:
    void prepare(const juce::dsp::ProcessSpec& spec);
    void reset() noexcept;
    void process(const juce::AudioBuffer<float>& iBuffer, juce::AudioBuffer<float>& qBuffer, float phaseAngleDeg) const;

  private:
    juce::dsp::ProcessSpec spec_{};
};

} // namespace qbdsp
