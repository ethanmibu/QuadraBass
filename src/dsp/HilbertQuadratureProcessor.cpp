#include "HilbertQuadratureProcessor.h"

namespace qbdsp {

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

    reset();
}

void HilbertQuadratureProcessor::reset() noexcept {
    for (int i = 0; i < 4; ++i) {
        stateI_[i] = 0.0f;
        stateQ_[i] = 0.0f;
    }
}

void HilbertQuadratureProcessor::process(juce::AudioBuffer<float>& iBuffer, juce::AudioBuffer<float>& qBuffer,
                                         float phaseAngleDeg) noexcept {
    juce::ignoreUnused(phaseAngleDeg);

    const int numSamples = iBuffer.getNumSamples();
    float* iData = iBuffer.getWritePointer(0);
    float* qData = qBuffer.getWritePointer(0);

    qBuffer.copyFrom(0, 0, iBuffer, 0, 0, numSamples);

    for (int s = 0; s < numSamples; ++s) {
        float x = iData[s];

        // I Branch (1st-order z^-1 cascade)
        float currI = x;
        for (int stage = 0; stage < 4; ++stage) {
            float y = coeffsI_[stage] * currI + stateI_[stage];
            stateI_[stage] = currI - coeffsI_[stage] * y;
            currI = y;
        }
        iData[s] = currI;

        // Q Branch
        float currQ = qData[s];
        for (int stage = 0; stage < 4; ++stage) {
            float y = coeffsQ_[stage] * currQ + stateQ_[stage];
            stateQ_[stage] = currQ - coeffsQ_[stage] * y;
            currQ = y;
        }
        qData[s] = currQ;
    }
}

} // namespace qbdsp
