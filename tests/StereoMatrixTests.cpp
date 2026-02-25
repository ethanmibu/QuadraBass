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

struct BasicStats {
    float leftRms = 0.0f;
    float rightRms = 0.0f;
    float corr = 0.0f;
    float diffRms = 0.0f;
};

BasicStats measureStats(const juce::AudioBuffer<float>& output) {
    const int samples = output.getNumSamples();
    const float* left = output.getReadPointer(0);
    const float* right = output.getReadPointer(1);

    double sumL2 = 0.0;
    double sumR2 = 0.0;
    double sumLR = 0.0;
    double sumD2 = 0.0;

    for (int i = 0; i < samples; ++i) {
        const double L = static_cast<double>(left[i]);
        const double R = static_cast<double>(right[i]);
        const double d = L - R;
        sumL2 += L * L;
        sumR2 += R * R;
        sumLR += L * R;
        sumD2 += d * d;
    }

    BasicStats stats;
    const double n = static_cast<double>(samples);
    stats.leftRms = static_cast<float>(std::sqrt(sumL2 / n));
    stats.rightRms = static_cast<float>(std::sqrt(sumR2 / n));
    stats.diffRms = static_cast<float>(std::sqrt(sumD2 / n));

    const double denom = std::sqrt(sumL2 * sumR2);
    stats.corr = denom > 1.0e-12 ? static_cast<float>(sumLR / denom) : 0.0f;
    return stats;
}

bool testSymmetricWidthAtNinetyDegrees() {
    qbdsp::StereoMatrixProcessor processor;
    juce::dsp::ProcessSpec spec{48000.0, 2048, 2};
    processor.prepare(spec);

    juce::AudioBuffer<float> lowBuffer(1, 2048);
    juce::AudioBuffer<float> xHighBuffer(1, 2048);
    juce::AudioBuffer<float> iBuffer(1, 2048);
    juce::AudioBuffer<float> qBuffer(1, 2048);
    juce::AudioBuffer<float> output(2, 2048);

    for (int i = 0; i < 2048; ++i) {
        const float phase = 2.0f * juce::MathConstants<float>::pi * 1000.0f * i / 48000.0f;
        lowBuffer.setSample(0, i, 0.0f);
        xHighBuffer.setSample(0, i, 0.0f);
        iBuffer.setSample(0, i, std::sin(phase));
        qBuffer.setSample(0, i, std::cos(phase));
    }

    processor.process(lowBuffer, xHighBuffer, iBuffer, qBuffer, output, 0.0f, 90.0f, 0.0f, true);
    const auto s0 = measureStats(output);

    bool ok = true;
    ok &= expect(s0.diffRms < 1.0e-4f, "Width 0 should produce mono output");
    ok &= expect(s0.corr > 0.99f, "Width 0 correlation should be near +1");

    processor.process(lowBuffer, xHighBuffer, iBuffer, qBuffer, output, 50.0f, 90.0f, 0.0f, true);
    const auto s50 = measureStats(output);

    processor.process(lowBuffer, xHighBuffer, iBuffer, qBuffer, output, 100.0f, 90.0f, 0.0f, true);
    const auto s100 = measureStats(output);

    const float levelDiffDb = std::abs(20.0f * std::log10((s100.leftRms + 1.0e-12f) / (s100.rightRms + 1.0e-12f)));

    ok &= expect(s0.corr > s50.corr + 0.05f, "Correlation should decrease from width 0 to 50");
    ok &= expect(s50.corr > s100.corr + 0.05f, "Correlation should decrease from width 50 to 100");
    ok &= expect(std::abs(s50.corr - 0.7071f) < 0.12f, "Width 50 should target roughly cos(45deg) correlation");
    ok &= expect(std::abs(s100.corr) < 0.15f, "Width 100 should be near decorrelated");
    ok &= expect(levelDiffDb < 0.1f, "Symmetric matrix should keep L/R levels matched");

    return ok;
}

} // namespace

int main() {
    bool ok = true;
    ok &= testSymmetricWidthAtNinetyDegrees();

    if (!ok)
        return 1;

    std::cout << "StereoMatrix tests passed.\n";
    return 0;
}
