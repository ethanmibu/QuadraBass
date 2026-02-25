#pragma once

#include <atomic>
#include <juce_gui_basics/juce_gui_basics.h>
#include <vector>

namespace qbui {

class GoniometerComponent : public juce::Component, public juce::Timer {
  public:
    GoniometerComponent();
    void paint(juce::Graphics& g) override;
    void timerCallback() override;

    void processBlock(const float* left, const float* right, int numSamples);

    // XY mapping for tests
    static void mapXY(float L, float R, float& x, float& y);

  private:
    void pushSample(float L, float R);

    juce::AbstractFifo fifo_{2048};
    std::vector<float> bufferL_;
    std::vector<float> bufferR_;

    juce::Image displayImage_;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(GoniometerComponent)
};

} // namespace qbui
