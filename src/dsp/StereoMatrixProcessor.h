#pragma once

#include <juce_dsp/juce_dsp.h>

namespace qbdsp {

class StereoMatrixProcessor final {
  public:
    void prepare(const juce::dsp::ProcessSpec& spec);
    void reset() noexcept;
    void process(const juce::AudioBuffer<float>& lowBuffer, const juce::AudioBuffer<float>& xHighBuffer,
                 const juce::AudioBuffer<float>& iBuffer, const juce::AudioBuffer<float>& qBuffer,
                 juce::AudioBuffer<float>& outputBuffer, float widthPercent, float phaseAngleDeg,
                 float phaseRotationDeg, bool useFirLinearWidthLaw) const noexcept;

  private:
    juce::dsp::ProcessSpec spec_{};
};

} // namespace qbdsp
