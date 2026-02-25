#pragma once

#include "dsp/HilbertQuadratureProcessor.h"
#include "dsp/StereoMatrixProcessor.h"
#include "util/Params.h"
#include <JuceHeader.h>

class QuadraBassAudioProcessor final : public juce::AudioProcessor {
  public:
    QuadraBassAudioProcessor();
    ~QuadraBassAudioProcessor() override = default;

    void prepareToPlay(double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

#if !JucePlugin_PreferredChannelConfigurations
    bool isBusesLayoutSupported(const BusesLayout& layouts) const override;
#endif

    void processBlock(juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    const juce::String getName() const override;

    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;

    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram(int index) override;
    const juce::String getProgramName(int index) override;
    void changeProgramName(int index, const juce::String& newName) override;

    void getStateInformation(juce::MemoryBlock& destData) override;
    void setStateInformation(const void* data, int sizeInBytes) override;

    util::Params& params() noexcept { return params_; }
    const util::Params& params() const noexcept { return params_; }

  private:
    util::Params params_;
    qbdsp::HilbertQuadratureProcessor hilbert_;
    qbdsp::StereoMatrixProcessor stereoMatrix_;
    qbdsp::HilbertQuadratureProcessor::Mode activeHilbertMode_ = qbdsp::HilbertQuadratureProcessor::Mode::IIR;
    juce::AudioBuffer<float> monoBuffer_;
    juce::AudioBuffer<float> xHighBuffer_;
    juce::AudioBuffer<float> qBuffer_;
    juce::AudioBuffer<float> zeroBuffer_;
    juce::dsp::Gain<float> outputGain_;
    juce::dsp::ProcessSpec processSpec_{};

  public:
    std::atomic<void*> activeGoniometer_{nullptr};
    std::atomic<void*> activeCorrelationMeter_{nullptr};

  private:
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(QuadraBassAudioProcessor)
};
