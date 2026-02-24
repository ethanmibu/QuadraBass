#pragma once

#include "PluginProcessor.h"
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
    juce::Label subtitle_;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(QuadraBassAudioProcessorEditor)
};
