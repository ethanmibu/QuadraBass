#pragma once

#include <JuceHeader.h>

namespace ui {

class StereometerComponent final : public juce::Component {
  public:
    StereometerComponent() = default;

    void setPurity(float newPurity) noexcept;
    void paint(juce::Graphics& g) override;

  private:
    float purity_ = 0.0f;
};

} // namespace ui
