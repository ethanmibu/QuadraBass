#include "PluginEditor.h"

QuadraBassAudioProcessorEditor::QuadraBassAudioProcessorEditor(QuadraBassAudioProcessor& processor)
    : AudioProcessorEditor(&processor), audioProcessor_(processor) {
    juce::ignoreUnused(audioProcessor_);

    title_.setText("QuadraBass", juce::dontSendNotification);
    title_.setJustificationType(juce::Justification::centred);
    title_.setFont(juce::Font(28.0f, juce::Font::bold));
    addAndMakeVisible(title_);

    subtitle_.setText("Hilbert Stereo Widener (Initialization Scaffold)", juce::dontSendNotification);
    subtitle_.setJustificationType(juce::Justification::centred);
    subtitle_.setFont(juce::Font(14.0f));
    addAndMakeVisible(subtitle_);

    setSize(760, 420);
}

void QuadraBassAudioProcessorEditor::paint(juce::Graphics& g) {
    g.fillAll(juce::Colour::fromRGB(18, 21, 28));

    const auto bounds = getLocalBounds().reduced(24);
    g.setColour(juce::Colour::fromRGB(41, 51, 68));
    g.drawRoundedRectangle(bounds.toFloat(), 8.0f, 1.5f);

    g.setColour(juce::Colour::fromRGB(89, 174, 255));
    g.drawLine(static_cast<float>(bounds.getX() + 20), 120.0f, static_cast<float>(bounds.getRight() - 20), 120.0f,
               2.0f);
}

void QuadraBassAudioProcessorEditor::resized() {
    const auto bounds = getLocalBounds().reduced(24);
    title_.setBounds(bounds.getX(), 24, bounds.getWidth(), 36);
    subtitle_.setBounds(bounds.getX(), 62, bounds.getWidth(), 24);
}
