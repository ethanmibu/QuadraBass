#include "../src/dsp/StereoMatrixProcessor.h"
#include <cmath>
#include <iostream>

namespace {

bool expect(bool condition, const std::string& message) {
    if (condition)
        return true;
    std::cerr << "FAIL: " << message << '\n';
    return false;
}

bool testStereoMatrixFolddown() {
    qbdsp::StereoMatrixProcessor processor;
    juce::dsp::ProcessSpec spec{48000.0, 512, 2};
    processor.prepare(spec);

    juce::AudioBuffer<float> lowBuffer(1, 512);
    juce::AudioBuffer<float> xHighBuffer(1, 512);
    juce::AudioBuffer<float> iBuffer(1, 512);
    juce::AudioBuffer<float> qBuffer(1, 512);
    juce::AudioBuffer<float> outputBuffer(2, 512);

    // Default width 0
    for (int i = 0; i < 512; ++i) {
        lowBuffer.setSample(0, i, 0.5f);
        xHighBuffer.setSample(0, i, 0.5f);
        iBuffer.setSample(0, i, 0.5f);
        qBuffer.setSample(0, i, 0.5f);
    }

    processor.process(lowBuffer, xHighBuffer, iBuffer, qBuffer, outputBuffer, 0.0f, 90.0f, 0.0f);

    float sumLeft = 0.0f;
    float sumRight = 0.0f;
    for (int i = 0; i < 512; ++i) {
        sumLeft += outputBuffer.getSample(0, i);
        sumRight += outputBuffer.getSample(1, i);
    }

    bool ok = expect(std::abs(sumLeft - sumRight) < 1e-4f, "Left and Right should be equal for 0% width");

    // Width 100 with quadrature signals representing an allpass shifted version of xHigh
    float originalMonoRms = 0.0f;
    for (int i = 0; i < 512; ++i) {
        float low = std::sin(2.0f * 3.14159f * 40.0f * i / 48000.0f);     // 40 Hz low
        float xHigh = std::sin(2.0f * 3.14159f * 1000.0f * i / 48000.0f); // 1 kHz original high
        // I and Q are phase shifted from xHigh. Say I is +45 and Q is -45 degrees respectively.
        float I = std::sin(2.0f * 3.14159f * 1000.0f * i / 48000.0f + 3.14159f / 4.0f);
        float Q = std::sin(2.0f * 3.14159f * 1000.0f * i / 48000.0f - 3.14159f / 4.0f);

        lowBuffer.setSample(0, i, low);
        xHighBuffer.setSample(0, i, xHigh);
        iBuffer.setSample(0, i, I);
        qBuffer.setSample(0, i, Q);

        float monoInput = low + xHigh;
        originalMonoRms += monoInput * monoInput;
    }
    originalMonoRms = std::sqrt(originalMonoRms / 512.0f);

    processor.process(lowBuffer, xHighBuffer, iBuffer, qBuffer, outputBuffer, 100.0f, 90.0f, 0.0f);

    float folddownRms = 0.0f;
    for (int i = 0; i < 512; ++i) {
        float l = outputBuffer.getSample(0, i);
        float r = outputBuffer.getSample(1, i);
        float mono = (l + r) * 0.5f;
        folddownRms += mono * mono;
    }
    folddownRms = std::sqrt(folddownRms / 512.0f);

    // Check if the RMS difference is within 1.0 dB (~12%)
    float ratio = folddownRms / originalMonoRms;
    std::cout << "Original Mono RMS: " << originalMonoRms << ", Folddown RMS: " << folddownRms << " Ratio: " << ratio
              << "\n";
    ok &= expect(ratio > 0.88f && ratio < 1.12f, "Fold-down should remain close to original mono RMS at 100% width");

    return ok;
}

} // namespace

int main() {
    bool ok = true;
    ok &= testStereoMatrixFolddown();

    if (!ok)
        return 1;

    std::cout << "StereoMatrix tests passed.\n";
    return 0;
}
