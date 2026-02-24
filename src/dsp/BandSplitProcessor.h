#pragma once

#include <JuceHeader.h>

namespace qbdsp {

class BandSplitProcessor final {
  public:
    void prepare(const juce::dsp::ProcessSpec& spec);
    void reset() noexcept;
    void process(juce::AudioBuffer<float>& monoBuffer, float crossoverHz) noexcept;

  private:
    juce::dsp::ProcessSpec spec_{};
};

} // namespace qbdsp
