#include "../src/dsp/HilbertQuadratureProcessor.h"
#include <algorithm>
#include <cmath>
#include <iostream>
#include <vector>

namespace {

bool expect(bool condition, const std::string& message) {
    if (condition)
        return true;
    std::cerr << "FAIL: " << message << '\n';
    return false;
}

struct ToneMetrics {
    double magRatio = 0.0;
    double absCorr = 0.0;
    double phaseErrDeg = 0.0;
    double magErrDb = 0.0;
};

double wrapDegrees(double degrees) {
    while (degrees > 180.0)
        degrees -= 360.0;
    while (degrees < -180.0)
        degrees += 360.0;
    return degrees;
}

double nearestQuadratureError(double phaseDiffDeg) {
    const double errToPlus = std::abs(wrapDegrees(phaseDiffDeg - 90.0));
    const double errToMinus = std::abs(wrapDegrees(phaseDiffDeg + 90.0));
    return std::min(errToPlus, errToMinus);
}

ToneMetrics measureTone(qbdsp::HilbertQuadratureProcessor& processor, double sampleRate, float freqHz) {
    constexpr int blockSize = 512;
    juce::AudioBuffer<float> iBuffer(1, blockSize);
    juce::AudioBuffer<float> qBuffer(1, blockSize);

    const double omega = 2.0 * juce::MathConstants<double>::pi * static_cast<double>(freqHz) / sampleRate;
    const double tonePeriodSamples = sampleRate / std::max(1.0f, freqHz);
    const int settleSamples = processor.getLatencySamples() + static_cast<int>(std::ceil(2.0 * tonePeriodSamples));
    const int captureSamples = std::max(blockSize * 8, static_cast<int>(std::ceil(8.0 * tonePeriodSamples)));

    double phase = 0.0;
    int totalSamples = 0;
    int captured = 0;
    const int maxSamples = settleSamples + captureSamples + blockSize * 8;

    double iSin = 0.0;
    double iCos = 0.0;
    double qSin = 0.0;
    double qCos = 0.0;

    while (captured < captureSamples && totalSamples < maxSamples) {
        for (int i = 0; i < blockSize; ++i) {
            iBuffer.setSample(0, i, static_cast<float>(std::sin(phase)));
            phase += omega;
            if (phase > juce::MathConstants<double>::twoPi)
                phase -= juce::MathConstants<double>::twoPi;
        }

        processor.process(iBuffer, qBuffer, 90.0f);

        for (int i = 0; i < blockSize && captured < captureSamples; ++i) {
            if (totalSamples >= settleSamples) {
                const double I = static_cast<double>(iBuffer.getSample(0, i));
                const double Q = static_cast<double>(qBuffer.getSample(0, i));
                const double phi = omega * static_cast<double>(totalSamples);
                const double s = std::sin(phi);
                const double c = std::cos(phi);

                iSin += I * s;
                iCos += I * c;
                qSin += Q * s;
                qCos += Q * c;
                ++captured;
            }
            ++totalSamples;
        }
    }

    ToneMetrics metrics;
    if (captured <= 0)
        return metrics;

    const double scale = 2.0 / static_cast<double>(captured);
    const double iSinN = iSin * scale;
    const double iCosN = iCos * scale;
    const double qSinN = qSin * scale;
    const double qCosN = qCos * scale;

    const double ampI = std::hypot(iSinN, iCosN);
    const double ampQ = std::hypot(qSinN, qCosN);
    metrics.magRatio = (ampQ > 1.0e-12) ? (ampI / ampQ) : 0.0;

    const double phaseI = std::atan2(iCosN, iSinN);
    const double phaseQ = std::atan2(qCosN, qSinN);
    const double phaseDiffDeg = wrapDegrees((phaseQ - phaseI) * 180.0 / juce::MathConstants<double>::pi);

    metrics.phaseErrDeg = nearestQuadratureError(phaseDiffDeg);
    metrics.absCorr = std::abs(std::cos(phaseDiffDeg * juce::MathConstants<double>::pi / 180.0));
    metrics.magErrDb = std::abs(20.0 * std::log10(std::max(metrics.magRatio, 1.0e-12)));
    return metrics;
}

double percentile95(std::vector<double> values) {
    if (values.empty())
        return 0.0;
    std::sort(values.begin(), values.end());
    const size_t index = static_cast<size_t>(std::floor(0.95 * static_cast<double>(values.size() - 1)));
    return values[index];
}

bool testIIRRegression() {
    qbdsp::HilbertQuadratureProcessor processor;
    const double sampleRate = 48000.0;
    juce::dsp::ProcessSpec spec{sampleRate, 512, 1};
    processor.prepare(spec);
    processor.setMode(qbdsp::HilbertQuadratureProcessor::Mode::IIR);

    bool ok = true;
    ok &= expect(processor.getLatencySamples() == 0, "IIR mode latency should be zero");

    const float testFreqs[] = {40.0f, 100.0f, 1000.0f, 10000.0f, 16000.0f};

    for (float freq : testFreqs) {
        processor.reset();
        const auto metrics = measureTone(processor, sampleRate, freq);

        if (metrics.magRatio < 0.84 || metrics.magRatio > 1.18) {
            std::cerr << "IIR mag ratio for " << freq << " Hz is " << metrics.magRatio << '\n';
            ok = false;
        }

        double allowedCorr = 0.25; // ~75 deg
        if (freq < 100.0f)
            allowedCorr = 0.40;
        if (freq >= 100.0f && freq < 1000.0f)
            allowedCorr = 0.28;
        if (freq >= 9000.0f)
            allowedCorr = 0.65;
        if (freq >= 15000.0f)
            allowedCorr = 0.90;

        if (metrics.absCorr > allowedCorr) {
            std::cerr << "IIR |corr| for " << freq << " Hz is " << metrics.absCorr << " (expecting < " << allowedCorr
                      << ")\n";
            ok = false;
        }
    }

    return expect(ok, "IIR regression checks passed");
}

bool testFIRAccuracyTargets() {
    bool ok = true;

    for (double sampleRate : {44100.0, 48000.0, 96000.0}) {
        qbdsp::HilbertQuadratureProcessor processor;
        juce::dsp::ProcessSpec spec{sampleRate, 512, 1};
        processor.prepare(spec);
        processor.setMode(qbdsp::HilbertQuadratureProcessor::Mode::FIR);
        processor.reset();

        ok &= expect(processor.getLatencySamples() > 0, "FIR mode should report non-zero latency");

        std::vector<double> phaseErrorsMain;
        std::vector<double> magErrorsMain;
        std::vector<double> edgePhaseErrors;

        for (double freq : {30.0, 60.0, 120.0, 250.0, 500.0, 1000.0, 2000.0, 4000.0, 8000.0, 12000.0, 16000.0}) {
            if (freq > sampleRate * 0.45)
                continue;
            processor.reset();
            const auto metrics = measureTone(processor, sampleRate, static_cast<float>(freq));
            phaseErrorsMain.push_back(metrics.phaseErrDeg);
            magErrorsMain.push_back(metrics.magErrDb);
        }

        const double edgeFreq = std::min(sampleRate * 0.47, sampleRate * 0.5 - 200.0);
        processor.reset();
        edgePhaseErrors.push_back(measureTone(processor, sampleRate, static_cast<float>(edgeFreq)).phaseErrDeg);

        ok &= expect(!phaseErrorsMain.empty(), "Main-band FIR probe set should not be empty");
        ok &= expect(!magErrorsMain.empty(), "Main-band FIR magnitude probe set should not be empty");

        const double mainPhase95 = percentile95(phaseErrorsMain);
        const double mainPhaseMax =
            phaseErrorsMain.empty() ? 0.0 : *std::max_element(phaseErrorsMain.begin(), phaseErrorsMain.end());
        const double mainMag95 = percentile95(magErrorsMain);
        const double mainMagMax =
            magErrorsMain.empty() ? 0.0 : *std::max_element(magErrorsMain.begin(), magErrorsMain.end());
        const double edgePhaseMax =
            edgePhaseErrors.empty() ? 0.0 : *std::max_element(edgePhaseErrors.begin(), edgePhaseErrors.end());

        if (!(mainPhase95 <= 3.0 && mainPhaseMax <= 8.0 && mainMag95 <= 0.5 && mainMagMax <= 1.5 &&
              edgePhaseMax <= 12.0)) {
            std::cerr << "FIR stats @" << sampleRate << " Hz: phase95=" << mainPhase95 << " phaseMax=" << mainPhaseMax
                      << " mag95=" << mainMag95 << " magMax=" << mainMagMax << " edgePhaseMax=" << edgePhaseMax << '\n';
        }

        ok &= expect(mainPhase95 <= 3.0, "FIR main-band phase error 95th percentile should be <= 3 deg");
        ok &= expect(mainPhaseMax <= 8.0, "FIR main-band phase error max should be <= 8 deg");
        ok &= expect(mainMag95 <= 0.5, "FIR main-band magnitude error 95th percentile should be <= 0.5 dB");
        ok &= expect(mainMagMax <= 1.5, "FIR main-band magnitude error max should be <= 1.5 dB");
        ok &= expect(edgePhaseMax <= 12.0, "FIR edge-band phase error max should be <= 12 deg");
    }

    return expect(ok, "FIR accuracy target checks passed");
}

} // namespace

int main() {
    bool ok = true;
    ok &= testIIRRegression();
    ok &= testFIRAccuracyTargets();

    if (!ok)
        return 1;

    std::cout << "HilbertQuadrature tests passed.\n";
    return 0;
}
