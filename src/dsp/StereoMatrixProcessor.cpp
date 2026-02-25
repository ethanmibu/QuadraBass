#include "StereoMatrixProcessor.h"
#include <cmath>

namespace qbdsp {

void StereoMatrixProcessor::prepare(const juce::dsp::ProcessSpec& spec) {
    spec_ = spec;
}

void StereoMatrixProcessor::reset() noexcept {}

void StereoMatrixProcessor::process(const juce::AudioBuffer<float>& lowBuffer,
                                    const juce::AudioBuffer<float>& xHighBuffer,
                                    const juce::AudioBuffer<float>& iBuffer, const juce::AudioBuffer<float>& qBuffer,
                                    juce::AudioBuffer<float>& outputBuffer, float widthPercent, float phaseAngleDeg,
                                    float phaseRotationDeg) const noexcept {
    const int samples = outputBuffer.getNumSamples();
    const int numOutChannels = outputBuffer.getNumChannels();
    if (samples <= 0 || numOutChannels <= 0)
        return;

    const float w = juce::jlimit(0.0f, 1.0f, widthPercent * 0.01f);
    const float gm = std::sqrt(1.0f - w);
    const float gq = std::sqrt(w);
    const float gComp = 1.0f / std::sqrt(1.0f - 0.5f * w);

    float angleDiffRad = (phaseAngleDeg - 90.0f) * juce::MathConstants<float>::pi / 180.0f;
    float theta = angleDiffRad * 0.5f;
    float cosTheta = std::cos(theta);
    float sinTheta = std::sin(theta);

    float rotRad = phaseRotationDeg * juce::MathConstants<float>::pi / 180.0f;
    float cosRot = std::cos(rotRad);
    float sinRot = std::sin(rotRad);

    const float* lowData = lowBuffer.getReadPointer(0);
    const float* xHighData = xHighBuffer.getReadPointer(0);
    const float* iData = iBuffer.getReadPointer(0);
    const float* qData = qBuffer.getReadPointer(0);

    float* left = outputBuffer.getWritePointer(0);
    float* right = numOutChannels > 1 ? outputBuffer.getWritePointer(1) : nullptr;

    for (int s = 0; s < samples; ++s) {
        float low = lowBuffer.getNumChannels() > 0 ? lowData[s] : 0.0f;
        float xHigh = xHighBuffer.getNumChannels() > 0 ? xHighData[s] : 0.0f;
        float I = iBuffer.getNumChannels() > 0 ? iData[s] : 0.0f;
        float Q = qBuffer.getNumChannels() > 0 ? qData[s] : 0.0f;

        // The doc: Lh = gComp * (gm * xHigh + gq * I)
        //          Rh = gComp * (gm * xHigh + gq * Q)
        float lh = gComp * (gm * xHigh + gq * I);
        float rh = gComp * (gm * xHigh + gq * Q);

        // Advanced transforms requested AFTER this stage
        // "phaseAngleDeg offsets quadrature relationship around 90 deg."
        // Meaning we mix Lh and Rh together to adjust width/phase
        float Lh_mix = lh * cosTheta - rh * sinTheta;
        float Rh_mix = rh * cosTheta + lh * sinTheta;

        float L = low + Lh_mix;
        float R = low + Rh_mix;

        // Apply rotation to final stereo vector
        float L_rot = L * cosRot - R * sinRot;
        float R_rot = L * sinRot + R * cosRot;

        left[s] = L_rot;
        if (right != nullptr)
            right[s] = R_rot;
    }

    for (int ch = 2; ch < numOutChannels; ++ch) {
        outputBuffer.clear(ch, 0, samples);
    }
}

} // namespace qbdsp
