#pragma once

#include "PluginProcessor.h"
#include "ui/StereometerComponent.h"
#include <JuceHeader.h>

class QuadraBassAudioProcessorEditor final : public juce::AudioProcessorEditor {
  public:
    explicit QuadraBassAudioProcessorEditor(QuadraBassAudioProcessor&);
    ~QuadraBassAudioProcessorEditor() override = default;

    void paint(juce::Graphics&) override;
    void resized() override;

  private:
    QuadraBassAudioProcessor& audioProcessor_;
    juce::Label title_;
    ui::StereometerComponent stereometer_;

    juce::Slider crossoverSlider_;
    juce::Slider widthSlider_;
    juce::Slider phaseAngleSlider_;
    juce::Slider phaseRotationSlider_;
    juce::Slider gainSlider_;

    juce::Label crossoverLabel_;
    juce::Label widthLabel_;
    juce::Label phaseAngleLabel_;
    juce::Label phaseRotationLabel_;
    juce::Label gainLabel_;

    using SliderAttachment = juce::AudioProcessorValueTreeState::SliderAttachment;
    std::unique_ptr<SliderAttachment> crossoverAttachment_;
    std::unique_ptr<SliderAttachment> widthAttachment_;
    std::unique_ptr<SliderAttachment> phaseAngleAttachment_;
    std::unique_ptr<SliderAttachment> phaseRotationAttachment_;
    std::unique_ptr<SliderAttachment> gainAttachment_;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(QuadraBassAudioProcessorEditor)
};
