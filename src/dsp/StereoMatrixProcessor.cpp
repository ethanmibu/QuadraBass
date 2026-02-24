#include "StereoMatrixProcessor.h"

namespace dsp {

void StereoMatrixProcessor::prepare(const juce::dsp::ProcessSpec& spec) {
    spec_ = spec;
}

void StereoMatrixProcessor::reset() noexcept {
}

void StereoMatrixProcessor::process(const juce::AudioBuffer<float>& iBuffer, const juce::AudioBuffer<float>& qBuffer,
                                    juce::AudioBuffer<float>& outputBuffer, float widthPercent,
                                    float phaseRotationDeg) const noexcept {
    juce::ignoreUnused(phaseRotationDeg, spec_);

    const int samples = outputBuffer.getNumSamples();
    if (samples <= 0 || iBuffer.getNumChannels() <= 0)
        return;

    const float width = juce::jlimit(0.0f, 1.0f, widthPercent * 0.01f);
    const float* iData = iBuffer.getReadPointer(0);
    const float* qData = qBuffer.getNumChannels() > 0 ? qBuffer.getReadPointer(0) : iData;

    float* left = outputBuffer.getWritePointer(0);
    float* right = outputBuffer.getNumChannels() > 1 ? outputBuffer.getWritePointer(1) : nullptr;

    for (int i = 0; i < samples; ++i) {
        const float mono = iData[i];
        const float q = qData[i];
        left[i] = mono;
        if (right != nullptr)
            right[i] = juce::jmap(width, mono, q);
    }

    for (int ch = 2; ch < outputBuffer.getNumChannels(); ++ch)
        outputBuffer.clear(ch, 0, samples);
}

} // namespace dsp
