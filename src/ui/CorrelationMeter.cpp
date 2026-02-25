#include "CorrelationMeter.h"

namespace qbui {

CorrelationMeter::CorrelationMeter() {
    startTimerHz(30);
}

void CorrelationMeter::reset() {
    sumLR_ = 0.0f;
    sumL2_ = 0.0f;
    sumR2_ = 0.0f;
    currentCorrelation_.store(1.0f, std::memory_order_relaxed);
    displayCorrelation_ = 1.0f;
}

void CorrelationMeter::processBlock(const float* left, const float* right, int numSamples) {
    for (int i = 0; i < numSamples; ++i) {
        float l = left[i];
        float r = right[i];

        sumLR_ += l * r;
        sumL2_ += l * l;
        sumR2_ += r * r;

        // Decay to avoid overflow and limit window (~200ms time constant at 48kHz)
        sumLR_ *= 0.9999f;
        sumL2_ *= 0.9999f;
        sumR2_ *= 0.9999f;
    }

    float denom = std::sqrt(sumL2_ * sumR2_);
    float corr = (denom > 1e-6f) ? (sumLR_ / denom) : 0.0f;
    corr = juce::jlimit(-1.0f, 1.0f, corr);

    currentCorrelation_.store(corr, std::memory_order_relaxed);
}

float CorrelationMeter::getCorrelation() const {
    return currentCorrelation_.load(std::memory_order_relaxed);
}

void CorrelationMeter::timerCallback() {
    float target = currentCorrelation_.load(std::memory_order_relaxed);
    // Smooth the display
    displayCorrelation_ += (target - displayCorrelation_) * 0.2f;
    repaint();
}

void CorrelationMeter::paint(juce::Graphics& g) {
    auto bounds = getLocalBounds().toFloat();
    float height = bounds.getHeight();
    float width = bounds.getWidth();
    float midY = height * 0.5f;

    g.setColour(juce::Colours::black);
    g.fillRect(bounds);

    g.setColour(juce::Colours::darkgrey);
    g.drawLine(0, midY, width, midY);

    float val = displayCorrelation_;
    float mappedY = juce::jmap(val, 1.0f, -1.0f, 0.0f, height);

    g.setColour(juce::Colours::cyan);
    g.fillEllipse(width * 0.5f - 4.0f, mappedY - 4.0f, 8.0f, 8.0f);
}

} // namespace qbui
