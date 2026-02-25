#pragma once

#include <array>
#include <juce_dsp/juce_dsp.h>

namespace qbdsp {

class HilbertQuadratureProcessor final {
  public:
    enum class Mode : int { IIR = 0, FIR = 1 };
    static constexpr int kBaseFIRTaps = 8191;
    static constexpr int kMaxFIRTaps = 16383;

    void prepare(const juce::dsp::ProcessSpec& spec);
    void reset() noexcept;
    void setMode(Mode mode) noexcept;
    Mode getMode() const noexcept;
    int getLatencySamples() const noexcept;
    void process(juce::AudioBuffer<float>& iBuffer, juce::AudioBuffer<float>& qBuffer, float phaseAngleDeg) noexcept;

  private:
    void processIIR(juce::AudioBuffer<float>& iBuffer, juce::AudioBuffer<float>& qBuffer) noexcept;
    void processFIR(juce::AudioBuffer<float>& iBuffer, juce::AudioBuffer<float>& qBuffer) noexcept;
    void designFIR(double sampleRate);
    static int chooseFIRTapCount(double sampleRate) noexcept;

    juce::dsp::ProcessSpec spec_{};
    Mode mode_ = Mode::IIR;

    // 4 stages per branch
    float coeffsI_[4] = {0};
    float coeffsQ_[4] = {0};

    float stateI_[4] = {0};
    float stateQ_[4] = {0};

    std::array<float, kMaxFIRTaps> firCoeffs_{};
    std::array<float, kMaxFIRTaps> firHistory_{};
    int firTapCount_ = kBaseFIRTaps;
    int firLatencySamples_ = (kBaseFIRTaps - 1) / 2;
    int firWriteIndex_ = 0;
};

} // namespace qbdsp
