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

    // FIR mode can target a stricter width law due predictable Hilbert behavior.
    const float rhoFir = 1.0f - w;
    const float gmFir = std::sqrt(0.5f * (1.0f + rhoFir));
    const float gsFir = std::sqrt(0.5f * (1.0f - rhoFir));

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

    double sumI2 = 0.0;
    double sumQ2 = 0.0;
    double sumIQ = 0.0;
    if (useFirLinearWidthLaw) {
        // Orthogonalize side against mid per block so harmonic-rich content still
        // yields balanced L/R energy with imperfect quadrature.
        for (int s = 0; s < samples; ++s) {
            const double I = iBuffer.getNumChannels() > 0 ? static_cast<double>(iData[s]) : 0.0;
            const double Q = qBuffer.getNumChannels() > 0 ? static_cast<double>(qData[s]) : 0.0;
            sumI2 += I * I;
            sumQ2 += Q * Q;
            sumIQ += I * Q;
        }
    }

    const double eps = 1.0e-12;
    const float qProjOnI = useFirLinearWidthLaw ? static_cast<float>(sumIQ / std::max(sumI2, eps)) : 0.0f;
    const double sumS2 = sumQ2 - 2.0 * static_cast<double>(qProjOnI) * sumIQ +
                         static_cast<double>(qProjOnI) * static_cast<double>(qProjOnI) * sumI2;
    const float sideNorm = useFirLinearWidthLaw && sumS2 > eps ? static_cast<float>(std::sqrt(sumI2 / sumS2)) : 1.0f;

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
            const float S = (Q - qProjOnI * I) * sideNorm;
            lh = gmFir * I + gsFir * S;
            rh = gmFir * I - gsFir * S;
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
