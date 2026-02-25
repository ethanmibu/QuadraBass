#include "../src/util/Params.h"
#include <cmath>
#include <iostream>
#include <juce_audio_processors/juce_audio_processors.h>
#include <string>

namespace {

class DummyProcessor final : public juce::AudioProcessor {
  public:
    DummyProcessor()
        : AudioProcessor(BusesProperties()
                             .withInput("In", juce::AudioChannelSet::stereo(), true)
                             .withOutput("Out", juce::AudioChannelSet::stereo(), true)) {}

    const juce::String getName() const override { return "DummyProcessor"; }
    void prepareToPlay(double, int) override {}
    void releaseResources() override {}
    bool isBusesLayoutSupported(const BusesLayout& layouts) const override {
        return layouts.getMainInputChannelSet() == layouts.getMainOutputChannelSet();
    }
    void processBlock(juce::AudioBuffer<float>&, juce::MidiBuffer&) override {}
    juce::AudioProcessorEditor* createEditor() override { return nullptr; }
    bool hasEditor() const override { return false; }
    bool acceptsMidi() const override { return false; }
    bool producesMidi() const override { return false; }
    bool isMidiEffect() const override { return false; }
    double getTailLengthSeconds() const override { return 0.0; }
    int getNumPrograms() override { return 1; }
    int getCurrentProgram() override { return 0; }
    void setCurrentProgram(int) override {}
    const juce::String getProgramName(int) override { return {}; }
    void changeProgramName(int, const juce::String&) override {}
    void getStateInformation(juce::MemoryBlock&) override {}
    void setStateInformation(const void*, int) override {}
};

bool expect(bool condition, const std::string& message) {
    if (condition)
        return true;

    std::cerr << "FAIL: " << message << '\n';
    return false;
}

bool isNear(float a, float b, float epsilon = 1.0e-4f) {
    return std::abs(a - b) <= epsilon;
}

bool testParameterIdsExist() {
    DummyProcessor processor;
    util::Params params(processor);

    bool ok = true;
    ok &= expect(params.apvts.getParameter(util::Params::IDs::widthPercent) != nullptr, "Missing width_percent");
    ok &= expect(params.apvts.getParameter(util::Params::IDs::hilbertMode) != nullptr, "Missing hilbert_mode");
    ok &= expect(params.apvts.getParameter(util::Params::IDs::phaseAngleDeg) != nullptr, "Missing phase_angle_deg");
    ok &=
        expect(params.apvts.getParameter(util::Params::IDs::phaseRotationDeg) != nullptr, "Missing phase_rotation_deg");
    ok &= expect(params.apvts.getParameter(util::Params::IDs::outputGainDb) != nullptr, "Missing output_gain_db");
    ok &= expect(processor.getParameters().size() == 5, "Expected exactly five plugin parameters");
    return ok;
}

bool testDefaultValues() {
    DummyProcessor processor;
    util::Params params(processor);

    bool ok = true;
    ok &= expect(isNear(params.getWidthPercent(), 0.0f), "Default width should be 0%");
    auto* mode = dynamic_cast<juce::AudioParameterChoice*>(params.apvts.getParameter(util::Params::IDs::hilbertMode));
    if (mode != nullptr) {
        ok &= expect(mode->getIndex() == 1, "Default Hilbert mode should be FIR");
    } else {
        ok &= expect(false, "hilbert_mode is not a choice parameter");
    }
    ok &= expect(isNear(params.getPhaseAngleDeg(), 90.0f), "Default phase angle should be 90 deg");
    ok &= expect(isNear(params.getPhaseRotationDeg(), 0.0f), "Default phase rotation should be 0 deg");
    ok &= expect(isNear(params.getOutputGainDb(), 0.0f), "Default gain should be 0 dB");
    return ok;
}

bool testParameterRanges() {
    DummyProcessor processor;
    util::Params params(processor);

    auto expectRange = [&](const char* id, float minValue, float maxValue) {
        const auto* parameter = dynamic_cast<const juce::AudioParameterFloat*>(params.apvts.getParameter(id));
        if (parameter == nullptr)
            return expect(false, std::string("Parameter not float: ") + id);

        const auto range = parameter->range;
        bool ok = true;
        ok &= expect(isNear(range.start, minValue), std::string("Unexpected min for ") + id);
        ok &= expect(isNear(range.end, maxValue), std::string("Unexpected max for ") + id);
        return ok;
    };

    bool ok = true;
    ok &= expectRange(util::Params::IDs::widthPercent, 0.0f, 100.0f);
    ok &= expectRange(util::Params::IDs::phaseAngleDeg, 0.0f, 180.0f);
    ok &= expectRange(util::Params::IDs::phaseRotationDeg, -180.0f, 180.0f);
    ok &= expectRange(util::Params::IDs::outputGainDb, -60.0f, 12.0f);
    return ok;
}

} // namespace

int main() {
    bool ok = true;
    ok &= testParameterIdsExist();
    ok &= testDefaultValues();
    ok &= testParameterRanges();

    if (!ok)
        return 1;

    std::cout << "QuadraBass ParamLayout tests passed.\n";
    return 0;
}
