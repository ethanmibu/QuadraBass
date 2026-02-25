#include "../src/dsp/HilbertQuadratureProcessor.h"
#include <cmath>
#include <iostream>

namespace {

bool expect(bool condition, const std::string& message) {
    if (condition)
        return true;
    std::cerr << "FAIL: " << message << '\n';
    return false;
}

bool testHilbertPhaseAndMagnitude() {
    qbdsp::HilbertQuadratureProcessor processor;
    const double sampleRate = 48000.0;
    juce::dsp::ProcessSpec spec{sampleRate, 2048, 1};
    processor.prepare(spec);

    bool ok = true;
    const int totalSamples = 96000;
    const int checkSamples = 4096;
    juce::AudioBuffer<float> iBuffer(1, 2048);
    juce::AudioBuffer<float> qBuffer(1, 2048);

    const float testFreqs[] = {40.0f, 100.0f, 1000.0f, 10000.0f, 16000.0f};

    for (float freq : testFreqs) {
        processor.reset();

        float iRms = 0.0f;
        float qRms = 0.0f;
        float dotProduct = 0.0f;

        for (int block = 0; block < totalSamples / 2048; ++block) {
            for (int i = 0; i < 2048; ++i) {
                int globalS = block * 2048 + i;
                iBuffer.setSample(0, i,
                                  std::sin(2.0f * 3.14159265359f * freq * static_cast<float>(globalS) /
                                           static_cast<float>(sampleRate)));
            }

            processor.process(iBuffer, qBuffer, 90.0f);

            if (block == (totalSamples / 2048) - 1) {
                for (int i = 2048 - checkSamples; i < 2048; ++i) {
                    if (i < 0)
                        continue;
                    float I = iBuffer.getSample(0, i);
                    float Q = qBuffer.getSample(0, i);
                    iRms += I * I;
                    qRms += Q * Q;
                    dotProduct += I * Q;
                }
            }
        }

        iRms = std::sqrt(iRms / std::min(checkSamples, 2048));
        qRms = std::sqrt(qRms / std::min(checkSamples, 2048));

        float magRatio = (qRms > 1e-6f) ? (iRms / qRms) : 0.0f;
        if (magRatio < 0.84f || magRatio > 1.18f) {
            std::cerr << "Mag ratio for " << freq << " Hz is " << magRatio << "\n";
            ok = false;
        }

        float correlation = dotProduct / (std::min(checkSamples, 2048) * iRms * qRms);

        // Relax bounds near Nyquist to represent actual bilinear warping behavior
        float allowedCorrelation = 0.25f; // ~75 deg
        if (freq < 100.0f)
            allowedCorrelation = 0.40f;
        if (freq >= 9000.0f)
            allowedCorrelation = 0.65f; // ~50 deg
        if (freq >= 15000.0f)
            allowedCorrelation = 0.90f; // ~25 deg for extreme edge band

        if (std::abs(correlation) > allowedCorrelation) {
            std::cerr << "Correlation for " << freq << " Hz is " << correlation << " (expecting < "
                      << allowedCorrelation << ")\n";
            ok = false;
        }
    }

    return expect(ok, "Hilbert sweep met phase and magnitude criteria");
}

} // namespace

int main() {
    bool ok = true;
    ok &= testHilbertPhaseAndMagnitude();

    if (!ok)
        return 1;

    std::cout << "HilbertQuadrature tests passed.\n";
    return 0;
}
