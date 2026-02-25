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
                                    float phaseRotationDeg, bool useFirLinearWidthLaw) const noexcept {
    const int samples = outputBuffer.getNumSamples();
    const int numOutChannels = outputBuffer.getNumChannels();
    if (samples <= 0 || numOutChannels <= 0)
        return;

    const float w = juce::jlimit(0.0f, 1.0f, widthPercent * 0.01f);
    const float gmLegacy = std::sqrt(1.0f - w);
    const float gqLegacy = std::sqrt(w);
    const float gCompLegacy = 1.0f / std::sqrt(1.0f - 0.5f * w);

    // FIR width law: 0..45 degree per-side phase rotation equivalent.
    const float firPhase = juce::MathConstants<float>::pi * 0.25f * w;
    const float gmFir = std::cos(firPhase);
    const float gsFir = std::sin(firPhase);

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
        const float I = iBuffer.getNumChannels() > 0 ? iData[s] : 0.0f;
        const float Q = qBuffer.getNumChannels() > 0 ? qData[s] : 0.0f;
        const float xHigh = xHighBuffer.getNumChannels() > 0 ? xHighData[s] : 0.0f;

        float lh = 0.0f;
        float rh = 0.0f;

        if (useFirLinearWidthLaw) {
            lh = gmFir * I + gsFir * Q;
            rh = gmFir * I - gsFir * Q;
        } else {
            lh = gCompLegacy * (gmLegacy * xHigh + gqLegacy * I);
            rh = gCompLegacy * (gmLegacy * xHigh + gqLegacy * Q);
        }

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
