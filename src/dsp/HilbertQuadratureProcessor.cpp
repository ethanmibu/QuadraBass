#include "HilbertQuadratureProcessor.h"
#include <algorithm>
#include <cmath>

namespace qbdsp {

int HilbertQuadratureProcessor::chooseFIRTapCount(double sampleRate) noexcept {
    if (sampleRate <= 0.0)
        return kBaseFIRTaps;

    const double scaled = std::round(kBaseFIRTaps * (sampleRate / 48000.0));
    int taps = static_cast<int>(scaled);
    taps = juce::jlimit(1023, kMaxFIRTaps, taps);
    if ((taps % 2) == 0)
        ++taps;
    return juce::jmin(taps, kMaxFIRTaps);
}

void HilbertQuadratureProcessor::designFIR(double sampleRate) {
    firTapCount_ = chooseFIRTapCount(sampleRate);
    firLatencySamples_ = (firTapCount_ - 1) / 2;
    std::fill(firCoeffs_.begin(), firCoeffs_.end(), 0.0f);

    const int half = firLatencySamples_;
    constexpr double pi = juce::MathConstants<double>::pi;
    constexpr double twoPi = juce::MathConstants<double>::twoPi;
    const double denom = static_cast<double>(firTapCount_ - 1);

    for (int d = 0; d < firTapCount_; ++d) {
        const int n = d - half;
        if (n == 0 || (n % 2) == 0)
            continue;

        const double base = 2.0 / (pi * static_cast<double>(n));
        const double phase = twoPi * static_cast<double>(d) / denom;
        const double blackman = 0.42 - 0.5 * std::cos(phase) + 0.08 * std::cos(2.0 * phase);
        firCoeffs_[static_cast<size_t>(d)] = static_cast<float>(base * blackman);
    }

    // Least-squares passband normalization (do not force DC/Nyquist).
    const double sampleRateSafe = sampleRate > 1.0 ? sampleRate : 48000.0;
    const double minFn = 30.0 / sampleRateSafe; // cycles/sample, avoid DC singular behavior
    const double maxFn = 0.45 * 0.5;            // 0.45 * Nyquist (in cycles/sample)
    constexpr int kNormBins = 512;

    double sumMag = 0.0;
    double sumMag2 = 0.0;
    for (int k = 0; k < kNormBins; ++k) {
        const double u = static_cast<double>(k) / static_cast<double>(kNormBins - 1);
        const double fn = minFn + (maxFn - minFn) * u;
        const double omega = juce::MathConstants<double>::twoPi * fn;

        double real = 0.0;
        double imag = 0.0;
        for (int d = 0; d < firTapCount_; ++d) {
            const double c = static_cast<double>(firCoeffs_[static_cast<size_t>(d)]);
            const double angle = omega * static_cast<double>(d);
            real += c * std::cos(angle);
            imag -= c * std::sin(angle);
        }

        const double mag = std::hypot(real, imag);
        sumMag += mag;
        sumMag2 += mag * mag;
    }

    const double scale = sumMag2 > 1.0e-12 ? (sumMag / sumMag2) : 1.0;
    const float fScale = static_cast<float>(scale);
    for (int d = 0; d < firTapCount_; ++d)
        firCoeffs_[static_cast<size_t>(d)] *= fScale;
}

void HilbertQuadratureProcessor::prepare(const juce::dsp::ProcessSpec& spec) {
    spec_ = spec;

    // Exact 1st-order phase difference coefficients for ~48kHz wideband 90-degree shift
    coeffsI_[0] = -0.9995117f;
    coeffsI_[1] = -0.9858398f;
    coeffsI_[2] = -0.8657227f;
    coeffsI_[3] = -0.3559570f;

    coeffsQ_[0] = -0.9975586f;
    coeffsQ_[1] = -0.9570312f;
    coeffsQ_[2] = -0.6552734f;
    coeffsQ_[3] = -0.0537109f;

    designFIR(spec.sampleRate);
    reset();
}

void HilbertQuadratureProcessor::reset() noexcept {
    for (int i = 0; i < 4; ++i) {
        stateI_[i] = 0.0f;
        stateQ_[i] = 0.0f;
    }

    std::fill(firHistory_.begin(), firHistory_.end(), 0.0f);
    firWriteIndex_ = 0;
}

void HilbertQuadratureProcessor::setMode(Mode mode) noexcept {
    if (mode_ == mode)
        return;

    mode_ = mode;
    reset();
}

HilbertQuadratureProcessor::Mode HilbertQuadratureProcessor::getMode() const noexcept {
    return mode_;
}

int HilbertQuadratureProcessor::getLatencySamples() const noexcept {
    return mode_ == Mode::FIR ? firLatencySamples_ : 0;
}

void HilbertQuadratureProcessor::process(juce::AudioBuffer<float>& iBuffer, juce::AudioBuffer<float>& qBuffer,
                                         float phaseAngleDeg) noexcept {
    juce::ignoreUnused(phaseAngleDeg);

    if (mode_ == Mode::FIR) {
        processFIR(iBuffer, qBuffer);
        return;
    }

    processIIR(iBuffer, qBuffer);
}

void HilbertQuadratureProcessor::processIIR(juce::AudioBuffer<float>& iBuffer,
                                            juce::AudioBuffer<float>& qBuffer) noexcept {
    const int numSamples = iBuffer.getNumSamples();
    float* iData = iBuffer.getWritePointer(0);
    float* qData = qBuffer.getWritePointer(0);

    qBuffer.copyFrom(0, 0, iBuffer, 0, 0, numSamples);

    for (int s = 0; s < numSamples; ++s) {
        const float x = iData[s];

        float currI = x;
        for (int stage = 0; stage < 4; ++stage) {
            const float y = coeffsI_[stage] * currI + stateI_[stage];
            stateI_[stage] = currI - coeffsI_[stage] * y;
            currI = y;
        }
        iData[s] = currI;

        float currQ = qData[s];
        for (int stage = 0; stage < 4; ++stage) {
            const float y = coeffsQ_[stage] * currQ + stateQ_[stage];
            stateQ_[stage] = currQ - coeffsQ_[stage] * y;
            currQ = y;
        }
        qData[s] = currQ;
    }
}

void HilbertQuadratureProcessor::processFIR(juce::AudioBuffer<float>& iBuffer,
                                            juce::AudioBuffer<float>& qBuffer) noexcept {
    const int numSamples = iBuffer.getNumSamples();
    float* iData = iBuffer.getWritePointer(0);
    float* qData = qBuffer.getWritePointer(0);
    const int firstNonZeroTap = ((firLatencySamples_ % 2) == 0) ? 1 : 0;

    for (int s = 0; s < numSamples; ++s) {
        const float x = iData[s];
        firHistory_[static_cast<size_t>(firWriteIndex_)] = x;

        float q = 0.0f;
        int tapIndex = firWriteIndex_ - firstNonZeroTap;
        if (tapIndex < 0)
            tapIndex += firTapCount_;

        for (int d = firstNonZeroTap; d < firTapCount_; d += 2) {
            q += firCoeffs_[static_cast<size_t>(d)] * firHistory_[static_cast<size_t>(tapIndex)];
            tapIndex -= 2;
            if (tapIndex < 0)
                tapIndex += firTapCount_;
        }

        int delayedIndex = firWriteIndex_ - firLatencySamples_;
        if (delayedIndex < 0)
            delayedIndex += firTapCount_;

        iData[s] = firHistory_[static_cast<size_t>(delayedIndex)];
        qData[s] = q;

        ++firWriteIndex_;
        if (firWriteIndex_ >= firTapCount_)
            firWriteIndex_ = 0;
    }
}

} // namespace qbdsp
