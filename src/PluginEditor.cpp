#include "PluginEditor.h"
#include "util/Params.h"

QuadraBassAudioProcessorEditor::QuadraBassAudioProcessorEditor(QuadraBassAudioProcessor& processor)
    : AudioProcessorEditor(processor), audioProcessor_(processor) {
    title_.setText("QuadraBass", juce::dontSendNotification);
    title_.setJustificationType(juce::Justification::centredLeft);
    title_.setFont(juce::Font(28.0f, juce::Font::bold));
    addAndMakeVisible(title_);

    addAndMakeVisible(stereometer_);
    stereometer_.setPurity(0.0f);

    auto setupSlider = [this](juce::Slider& slider, juce::Label& label, const juce::String& labelText) {
        slider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
        slider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 88, 18);
        slider.setColour(juce::Slider::rotarySliderFillColourId, juce::Colour::fromRGB(89, 174, 255));
        slider.setColour(juce::Slider::thumbColourId, juce::Colour::fromRGB(195, 230, 255));
        addAndMakeVisible(slider);

        label.setText(labelText, juce::dontSendNotification);
        label.setJustificationType(juce::Justification::centred);
        label.setColour(juce::Label::textColourId, juce::Colour::fromRGB(192, 205, 220));
        addAndMakeVisible(label);
    };

    setupSlider(crossoverSlider_, crossoverLabel_, "Crossover");
    setupSlider(widthSlider_, widthLabel_, "Width");
    setupSlider(phaseAngleSlider_, phaseAngleLabel_, "Phase Angle");
    setupSlider(phaseRotationSlider_, phaseRotationLabel_, "Phase Rotation");
    setupSlider(gainSlider_, gainLabel_, "Gain");

    auto& apvts = audioProcessor_.params().apvts;
    crossoverAttachment_ = std::make_unique<SliderAttachment>(apvts, util::Params::IDs::crossoverHz, crossoverSlider_);
    widthAttachment_ = std::make_unique<SliderAttachment>(apvts, util::Params::IDs::widthPercent, widthSlider_);
    phaseAngleAttachment_ =
        std::make_unique<SliderAttachment>(apvts, util::Params::IDs::phaseAngleDeg, phaseAngleSlider_);
    phaseRotationAttachment_ =
        std::make_unique<SliderAttachment>(apvts, util::Params::IDs::phaseRotationDeg, phaseRotationSlider_);
    gainAttachment_ = std::make_unique<SliderAttachment>(apvts, util::Params::IDs::outputGainDb, gainSlider_);

    setSize(760, 420);
}

void QuadraBassAudioProcessorEditor::paint(juce::Graphics& g) {
    g.fillAll(juce::Colour::fromRGB(18, 21, 28));

    const auto bounds = getLocalBounds().reduced(24);
    g.setColour(juce::Colour::fromRGB(41, 51, 68));
    g.drawRoundedRectangle(bounds.toFloat(), 8.0f, 1.5f);

    g.setColour(juce::Colour::fromRGB(89, 174, 255));
    g.drawLine(static_cast<float>(bounds.getX() + 20), 88.0f, static_cast<float>(bounds.getRight() - 20), 88.0f,
               1.5f);
}

void QuadraBassAudioProcessorEditor::resized() {
    const auto bounds = getLocalBounds().reduced(24);
    title_.setBounds(bounds.getX() + 6, 20, bounds.getWidth() - 12, 40);

    auto topArea = bounds.withTop(98).withHeight(178);
    auto meterArea = topArea.removeFromLeft(240).reduced(8);
    stereometer_.setBounds(meterArea);

    auto knobsArea = topArea.reduced(6);
    const int knobWidth = knobsArea.getWidth() / 5;

    auto placeKnob = [](juce::Rectangle<int> area, juce::Slider& slider, juce::Label& label) {
        label.setBounds(area.removeFromTop(18));
        slider.setBounds(area.reduced(6));
    };

    placeKnob(knobsArea.removeFromLeft(knobWidth), crossoverSlider_, crossoverLabel_);
    placeKnob(knobsArea.removeFromLeft(knobWidth), widthSlider_, widthLabel_);
    placeKnob(knobsArea.removeFromLeft(knobWidth), phaseAngleSlider_, phaseAngleLabel_);
    placeKnob(knobsArea.removeFromLeft(knobWidth), phaseRotationSlider_, phaseRotationLabel_);
    placeKnob(knobsArea.removeFromLeft(knobsArea.getWidth()), gainSlider_, gainLabel_);
}
