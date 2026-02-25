#include "../src/PluginProcessor.h"
#include <algorithm>
#include <array>
#include <cmath>
#include <iostream>
#include <limits>
#include <string>

namespace {

bool expect(bool condition, const std::string& message) {
    if (condition)
        return true;
    std::cerr << "FAIL: " << message << '\n';
    return false;
}

enum class SignalKind { Sine, Square, Saw };

const char* signalName(SignalKind kind) {
    switch (kind) {
    case SignalKind::Sine:
        return "sine";
    case SignalKind::Square:
        return "square";
    case SignalKind::Saw:
        return "saw";
    }

    return "unknown";
}

float makeSignalSample(SignalKind kind, float freqHz, double sampleRate, int sampleIndex) {
    const double phase = 2.0 * juce::MathConstants<double>::pi * static_cast<double>(freqHz) * sampleIndex / sampleRate;

    switch (kind) {
    case SignalKind::Sine:
        return static_cast<float>(std::sin(phase));
    case SignalKind::Square:
        return std::sin(phase) >= 0.0 ? 1.0f : -1.0f;
    case SignalKind::Saw: {
        double cycle = std::fmod(static_cast<double>(freqHz) * sampleIndex / sampleRate, 1.0);
        if (cycle < 0.0)
            cycle += 1.0;
        return static_cast<float>(2.0 * cycle - 1.0);
    }
    }

    return 0.0f;
}

struct StereoStats {
    double inputRms = 0.0;
    double leftRms = 0.0;
    double rightRms = 0.0;
    double foldRms = 0.0;
    double spreadRatio = 0.0;
    double correlation = 0.0;

    double levelDiffDb() const {
        const double eps = 1.0e-12;
        return std::abs(20.0 * std::log10((leftRms + eps) / (rightRms + eps)));
    }
};

StereoStats runSignalThroughProcessor(SignalKind kind, float freqHz, float widthPercent, int modeIndex) {
    constexpr double sampleRate = 48000.0;
    constexpr int blockSize = 1024;

    QuadraBassAudioProcessor processor;
    processor.prepareToPlay(sampleRate, blockSize);

    auto* widthParam = dynamic_cast<juce::AudioParameterFloat*>(
        processor.params().apvts.getParameter(util::Params::IDs::widthPercent));
    auto* modeParam = dynamic_cast<juce::AudioParameterChoice*>(
        processor.params().apvts.getParameter(util::Params::IDs::hilbertMode));

    if (widthParam != nullptr)
        *widthParam = widthPercent;
    if (modeParam != nullptr)
        modeParam->operator=(modeIndex);

    const int totalBlocks = modeIndex == 1 ? 24 : 12;
    const int captureBlocks = modeIndex == 1 ? 8 : 6;

    juce::AudioBuffer<float> buffer(2, blockSize);
    juce::MidiBuffer midi;

    double sumIn2 = 0.0;
    double sumL2 = 0.0;
    double sumR2 = 0.0;
    double sumFold2 = 0.0;
    double sumLR = 0.0;
    double sumD2 = 0.0;
    int captureCount = 0;

    int sampleOffset = 0;
    for (int block = 0; block < totalBlocks; ++block) {
        for (int i = 0; i < blockSize; ++i) {
            const float v = makeSignalSample(kind, freqHz, sampleRate, sampleOffset + i);
            buffer.setSample(0, i, v);
            buffer.setSample(1, i, v);
        }

        processor.processBlock(buffer, midi);

        if (block >= totalBlocks - captureBlocks) {
            auto* left = buffer.getReadPointer(0);
            auto* right = buffer.getReadPointer(1);
            for (int i = 0; i < blockSize; ++i) {
                const double input = static_cast<double>(makeSignalSample(kind, freqHz, sampleRate, sampleOffset + i));
                const double L = static_cast<double>(left[i]);
                const double R = static_cast<double>(right[i]);
                const double fold = 0.5 * (L + R);
                const double d = L - R;

                sumIn2 += input * input;
                sumL2 += L * L;
                sumR2 += R * R;
                sumFold2 += fold * fold;
                sumLR += L * R;
                sumD2 += d * d;
            }

            captureCount += blockSize;
        }

        sampleOffset += blockSize;
    }

    StereoStats stats;
    if (captureCount <= 0)
        return stats;

    stats.inputRms = std::sqrt(sumIn2 / static_cast<double>(captureCount));
    stats.leftRms = std::sqrt(sumL2 / static_cast<double>(captureCount));
    stats.rightRms = std::sqrt(sumR2 / static_cast<double>(captureCount));
    stats.foldRms = std::sqrt(sumFold2 / static_cast<double>(captureCount));

    const double corrDen = std::sqrt(sumL2 * sumR2);
    stats.correlation = corrDen > 1.0e-12 ? (sumLR / corrDen) : 0.0;

    const double diffRms = std::sqrt(sumD2 / static_cast<double>(captureCount));
    stats.spreadRatio = diffRms / std::max(stats.inputRms, 1.0e-12);
    return stats;
}

bool testStereoFolddown() {
    bool ok = true;
    for (int modeIndex : {0, 1}) {
        const double minRatio = modeIndex == 1 ? 0.60 : 0.45;
        const double maxRatio = modeIndex == 1 ? 1.20 : 1.30;
        for (float freq : {30.0f, 100.0f, 1000.0f, 10000.0f}) {
            const auto stats = runSignalThroughProcessor(SignalKind::Sine, freq, 100.0f, modeIndex);
            const double ratio = stats.foldRms / std::max(stats.inputRms, 1.0e-12);

            ok &= expect(ratio > minRatio && ratio < maxRatio,
                         "Fold-down RMS should remain bounded at width 100. mode=" + std::to_string(modeIndex) +
                             " freq=" + std::to_string(freq));
        }
    }

    return ok;
}

bool testWidthZeroIsMonoAcrossBand() {
    bool ok = true;

    for (int modeIndex : {0, 1}) {
        for (float freq : {30.0f, 120.0f, 1000.0f, 10000.0f}) {
            const auto stats = runSignalThroughProcessor(SignalKind::Sine, freq, 0.0f, modeIndex);
            ok &= expect(stats.spreadRatio < 1.0e-3, "Width 0 should remain mono for sine. mode=" +
                                                         std::to_string(modeIndex) + " freq=" + std::to_string(freq));
            ok &= expect(stats.levelDiffDb() < 0.05, "Width 0 should keep L/R matched. mode=" +
                                                         std::to_string(modeIndex) + " freq=" + std::to_string(freq));
        }

        for (SignalKind kind : {SignalKind::Square, SignalKind::Saw}) {
            const auto stats = runSignalThroughProcessor(kind, 220.0f, 0.0f, modeIndex);
            ok &= expect(stats.spreadRatio < 1.0e-3, "Width 0 should remain mono for " + std::string(signalName(kind)) +
                                                         ". mode=" + std::to_string(modeIndex));
        }
    }

    return ok;
}

bool testWidthConsistencyAcrossFrequencies() {
    bool ok = true;

    for (int modeIndex : {0, 1}) {
        float minSpread = std::numeric_limits<float>::max();
        float maxSpread = 0.0f;

        for (float freq : {40.0f, 120.0f, 500.0f, 1000.0f, 3000.0f, 8000.0f}) {
            const auto stats = runSignalThroughProcessor(SignalKind::Sine, freq, 100.0f, modeIndex);
            minSpread = std::min(minSpread, static_cast<float>(stats.spreadRatio));
            maxSpread = std::max(maxSpread, static_cast<float>(stats.spreadRatio));
        }

        ok &= expect(minSpread > 0.15f, "Width 100 should produce non-trivial spread across frequencies. mode=" +
                                            std::to_string(modeIndex));
        ok &= expect(maxSpread < minSpread * 2.0f,
                     "Width response should stay reasonably consistent across frequencies. mode=" +
                         std::to_string(modeIndex));
    }

    return ok;
}

bool testWidthMapsToLinearCorrelationOnSine() {
    bool ok = true;

    constexpr int modeIndex = 1; // FIR-only deterministic phase-law contract
    const double tolMid = 0.14;
    const double tolWide = 0.20;
    const double expectedMidCorr = std::cos(juce::MathConstants<double>::pi * 0.25);

    for (float freq : {60.0f, 250.0f, 1000.0f, 4000.0f, 10000.0f}) {
        const auto s0 = runSignalThroughProcessor(SignalKind::Sine, freq, 0.0f, modeIndex);
        const auto s50 = runSignalThroughProcessor(SignalKind::Sine, freq, 50.0f, modeIndex);
        const auto s100 = runSignalThroughProcessor(SignalKind::Sine, freq, 100.0f, modeIndex);

        ok &= expect(s0.correlation > 0.9,
                     "Width 0 correlation should stay near mono in FIR mode. freq=" + std::to_string(freq));
        ok &= expect(s0.correlation > s50.correlation + 0.05,
                     "FIR correlation should decrease from 0 to 50 width. freq=" + std::to_string(freq));
        ok &= expect(s50.correlation > s100.correlation + 0.05,
                     "FIR correlation should decrease from 50 to 100 width. freq=" + std::to_string(freq));
        ok &= expect(std::abs(s50.correlation - expectedMidCorr) < tolMid,
                     "FIR width 50 correlation should track phase-law target. freq=" + std::to_string(freq));
        ok &= expect(std::abs(s100.correlation) < tolWide,
                     "FIR width 100 correlation should approach decorrelated target. freq=" + std::to_string(freq));
    }

    return ok;
}

bool testHarmonicContentBalanceAndDecorrelation() {
    bool ok = true;

    constexpr int modeIndex = 1; // FIR-only strict balance target
    constexpr double balanceLimitDb = 0.35;

    for (SignalKind kind : {SignalKind::Square, SignalKind::Saw}) {
        for (float width : {25.0f, 50.0f, 75.0f, 100.0f}) {
            const auto stats = runSignalThroughProcessor(kind, 220.0f, width, modeIndex);

            ok &= expect(stats.levelDiffDb() < balanceLimitDb,
                         std::string("FIR L/R level mismatch should stay low for harmonic content. kind=") +
                             signalName(kind) + " width=" + std::to_string(width));
        }

        const auto s50 = runSignalThroughProcessor(kind, 220.0f, 50.0f, modeIndex);
        const auto s100 = runSignalThroughProcessor(kind, 220.0f, 100.0f, modeIndex);

        ok &= expect(s100.correlation < s50.correlation - 0.05,
                     std::string("FIR harmonic correlation should continue dropping toward width 100. kind=") +
                         signalName(kind));
        ok &= expect(s100.correlation < 0.55,
                     std::string("FIR harmonic content at width 100 should be meaningfully decorrelated. kind=") +
                         signalName(kind));
    }

    return ok;
}

bool testParameterCountInProcessor() {
    QuadraBassAudioProcessor processor;
    return expect(processor.getParameters().size() == 5, "Processor should expose exactly five parameters");
}

bool testFIRModeProducesStableOutput() {
    const auto stats = runSignalThroughProcessor(SignalKind::Sine, 1000.0f, 100.0f, 1);

    bool ok = true;
    ok &= expect(stats.leftRms > 0.01, "FIR mode output RMS should remain non-trivial");
    ok &= expect(stats.spreadRatio > 0.05, "FIR mode should still produce widening at width 100");
    return ok;
}

} // namespace

int main() {
    bool ok = true;
    ok &= testStereoFolddown();
    ok &= testWidthZeroIsMonoAcrossBand();
    ok &= testWidthConsistencyAcrossFrequencies();
    ok &= testWidthMapsToLinearCorrelationOnSine();
    ok &= testHarmonicContentBalanceAndDecorrelation();
    ok &= testParameterCountInProcessor();
    ok &= testFIRModeProducesStableOutput();

    if (!ok)
        return 1;

    std::cout << "DspCompliance tests passed.\n";
    return 0;
}
