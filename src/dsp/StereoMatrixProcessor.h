#pragma once

#include <JuceHeader.h>

namespace dsp {

class StereoMatrixProcessor final {
  public:
    void prepare(const juce::dsp::ProcessSpec& spec);
    void reset() noexcept;
    void process(const juce::AudioBuffer<float>& iBuffer, const juce::AudioBuffer<float>& qBuffer,
                 juce::AudioBuffer<float>& outputBuffer, float widthPercent, float phaseRotationDeg) const noexcept;

  private:
    juce::dsp::ProcessSpec spec_{};
};

} // namespace dsp
