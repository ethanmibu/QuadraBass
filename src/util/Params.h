#pragma once

#include <atomic>
#include <juce_audio_processors/juce_audio_processors.h>

namespace util {

class Params final {
  public:
    struct IDs {
        static constexpr const char* widthPercent = "width_percent";
        static constexpr const char* hilbertMode = "hilbert_mode";
        static constexpr const char* phaseAngleDeg = "phase_angle_deg";
        static constexpr const char* phaseRotationDeg = "phase_rotation_deg";
        static constexpr const char* outputGainDb = "output_gain_db";
    };

    explicit Params(juce::AudioProcessor& processor);

    juce::AudioProcessorValueTreeState apvts;

    float getWidthPercent() const noexcept;
    int getHilbertModeIndex() const noexcept;
    float getPhaseAngleDeg() const noexcept;
    float getPhaseRotationDeg() const noexcept;
    float getOutputGainDb() const noexcept;

    static juce::AudioProcessorValueTreeState::ParameterLayout createLayout();

  private:
    std::atomic<float>* widthPercent_ = nullptr;
    std::atomic<float>* hilbertMode_ = nullptr;
    std::atomic<float>* phaseAngleDeg_ = nullptr;
    std::atomic<float>* phaseRotationDeg_ = nullptr;
    std::atomic<float>* outputGainDb_ = nullptr;
};

} // namespace util
