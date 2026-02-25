#include "GoniometerComponent.h"
#include <cmath>

namespace qbui {

GoniometerComponent::GoniometerComponent() {
    bufferL_.resize(2048, 0.0f);
    bufferR_.resize(2048, 0.0f);
    startTimerHz(30);
}

void GoniometerComponent::mapXY(float L, float R, float& x, float& y) {
    x = (L - R) / 1.41421356f;
    y = (L + R) / 1.41421356f;
}

void GoniometerComponent::pushSample(float L, float R) {
    if (fifo_.getFreeSpace() > 0) {
        int start1, size1, start2, size2;
        fifo_.prepareToWrite(1, start1, size1, start2, size2);

        if (size1 > 0) {
            bufferL_[(size_t)start1] = L;
            bufferR_[(size_t)start1] = R;
        }

        fifo_.finishedWrite(size1 + size2);
    }
}

void GoniometerComponent::processBlock(const float* left, const float* right, int numSamples) {
    // Decimate to not overwhelm UI buffer
    const int decimationFactor = 4;
    for (int i = 0; i < numSamples; i += decimationFactor) {
        pushSample(left[i], right[i]);
    }
}

void GoniometerComponent::timerCallback() {
    repaint();
}

void GoniometerComponent::paint(juce::Graphics& g) {
    auto bounds = getLocalBounds().toFloat();
    float width = bounds.getWidth();
    float height = bounds.getHeight();
    float cx = width * 0.5f;
    float cy = height * 0.5f;
    float scale = std::min(width, height) * 0.4f;

    g.setColour(juce::Colours::black);
    g.fillRect(bounds);

    g.setColour(juce::Colours::darkgrey.withAlpha(0.5f));
    g.drawLine(cx, 0, cx, height);
    g.drawLine(0, cy, width, cy);

    int start1, size1, start2, size2;
    fifo_.prepareToRead(fifo_.getNumReady(), start1, size1, start2, size2);

    g.setColour(juce::Colours::cyan.withAlpha(0.6f));
    auto drawPoints = [&](int start, int size) {
        for (int i = 0; i < size; ++i) {
            float L = bufferL_[(size_t)(start + i)];
            float R = bufferR_[(size_t)(start + i)];
            float x, y;
            mapXY(L, R, x, y);

            float screenX = cx + x * scale;
            float screenY = cy - y * scale; // Y inverted for screen coordinates

            g.fillEllipse(screenX - 1.0f, screenY - 1.0f, 2.0f, 2.0f);
        }
    };

    drawPoints(start1, size1);
    drawPoints(start2, size2);

    fifo_.finishedRead(size1 + size2);
}

} // namespace qbui
