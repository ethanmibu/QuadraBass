#pragma once

#include <atomic>
#include <juce_gui_basics/juce_gui_basics.h>

namespace qbui {

class CorrelationMeter : public juce::Component, public juce::Timer {
  public:
    CorrelationMeter();
    void paint(juce::Graphics& g) override;
    void timerCallback() override;

    void processBlock(const float* left, const float* right, int numSamples);
    void reset();
    float getCorrelation() const;

  private:
    float sumLR_ = 0.0f;
    float sumL2_ = 0.0f;
    float sumR2_ = 0.0f;

    std::atomic<float> currentCorrelation_{1.0f};

    // Smoothing
    float displayCorrelation_ = 1.0f;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(CorrelationMeter)
};

} // namespace qbui
