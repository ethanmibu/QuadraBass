#include "StereometerComponent.h"

namespace ui {

void StereometerComponent::setPurity(float newPurity) noexcept {
    purity_ = juce::jlimit(0.0f, 1.0f, newPurity);
    repaint();
}

void StereometerComponent::paint(juce::Graphics& g) {
    const auto area = getLocalBounds().reduced(10).toFloat();
    const auto center = area.getCentre();
    const float radius = juce::jmin(area.getWidth(), area.getHeight()) * 0.45f;

    g.fillAll(juce::Colour::fromRGB(12, 16, 23));

    g.setColour(juce::Colour::fromRGB(40, 48, 62));
    g.drawRect(area);
    g.drawLine(center.x, area.getY(), center.x, area.getBottom(), 1.0f);
    g.drawLine(area.getX(), center.y, area.getRight(), center.y, 1.0f);

    g.setColour(juce::Colour::fromRGB(89, 174, 255));
    g.drawEllipse(center.x - radius, center.y - radius, radius * 2.0f, radius * 2.0f, 1.5f);

    g.setColour(juce::Colour::fromRGB(152, 229, 143));
    g.setFont(juce::Font(13.0f));
    g.drawText("Purity: " + juce::String(juce::roundToInt(purity_ * 100.0f)) + "%", getLocalBounds().reduced(14),
               juce::Justification::bottomRight, false);
}

} // namespace ui
