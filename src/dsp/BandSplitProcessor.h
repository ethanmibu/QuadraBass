#pragma once

#include <juce_dsp/juce_dsp.h>

namespace qbdsp {

class BandSplitProcessor final {
  public:
    void prepare(const juce::dsp::ProcessSpec& spec);
    void reset() noexcept;
    void process(const juce::AudioBuffer<float>& inputBuffer, juce::AudioBuffer<float>& lowBuffer,
                 juce::AudioBuffer<float>& highBuffer, float crossoverHz, bool crossoverEnabled) noexcept;

  private:
    juce::dsp::ProcessSpec spec_{};
    juce::dsp::LinkwitzRileyFilter<float> lpFilter_;
    juce::dsp::LinkwitzRileyFilter<float> hpFilter_;
};

} // namespace qbdsp
