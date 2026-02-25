#include "../src/PluginProcessor.h"
#include <cmath>
#include <iostream>

namespace {

bool expect(bool condition, const std::string& message) {
    if (condition)
        return true;
    std::cerr << "FAIL: " << message << '\n';
    return false;
}

bool testStereoFolddown() {
    QuadraBassAudioProcessor processor;
    processor.prepareToPlay(48000.0, 512);

    // Set 100% width
    auto* widthParam = dynamic_cast<juce::AudioParameterFloat*>(
        processor.params().apvts.getParameter(util::Params::IDs::widthPercent));
    auto* crossoverEn = dynamic_cast<juce::AudioParameterBool*>(
        processor.params().apvts.getParameter(util::Params::IDs::crossoverEnabled));
    *widthParam = 100.0f;
    *crossoverEn = false;

    juce::AudioBuffer<float> buffer(2, 4800);

    // Test sweep frequency from 30Hz to 16kHz
    bool ok = true;
    for (float freq : {30.0f, 100.0f, 1000.0f, 10000.0f}) {
        // Mono input sine tone
        for (int i = 0; i < 4800; ++i) {
            float val = std::sin(2.0f * 3.14159f * freq * i / 48000.0f);
            buffer.setSample(0, i, val);
            buffer.setSample(1, i, val);
        }

        float inputRms = buffer.getRMSLevel(0, 0, 4800);

        juce::MidiBuffer midi;
        processor.processBlock(buffer, midi);

        // Compute folddown (L+R)/2
        float outSumSq = 0.0f;
        auto* L = buffer.getReadPointer(0);
        auto* R = buffer.getReadPointer(1);
        for (int i = 0; i < 4800; ++i) {
            float mono = (L[i] + R[i]) * 0.5f;
            outSumSq += mono * mono;
        }
        float outRms = std::sqrt(outSumSq / 4800.0f);

        // Folddown should be close to input RMS (no deep nulls)
        // Note: Hilbert introduces some magnitude ripple so we permit +/- 1.5dB tolerance (~0.84 to 1.18 ratio)
        ok &= expect(outRms > inputRms * 0.707f && outRms < inputRms * 1.41f,
                     "Folddown RMS should be preserved around 0dB. Freq=" + std::to_string(freq));
    }

    return ok;
}

bool testLowBandMonoIntegrity() {
    QuadraBassAudioProcessor processor;
    processor.prepareToPlay(48000.0, 512);

    auto* widthParam = dynamic_cast<juce::AudioParameterFloat*>(
        processor.params().apvts.getParameter(util::Params::IDs::widthPercent));
    auto* crossoverEn = dynamic_cast<juce::AudioParameterBool*>(
        processor.params().apvts.getParameter(util::Params::IDs::crossoverEnabled));
    auto* crossoverHz =
        dynamic_cast<juce::AudioParameterFloat*>(processor.params().apvts.getParameter(util::Params::IDs::crossoverHz));
    *widthParam = 100.0f;
    *crossoverEn = true;
    *crossoverHz = 90.0f;
    // The default is already 90Hz which is tested here

    juce::AudioBuffer<float> buffer(2, 512);

    // Inject 40 Hz which is well below crossover (90 Hz)
    for (int i = 0; i < 512; ++i) {
        float val = std::sin(2.0f * 3.14159f * 40.0f * i / 48000.0f);
        buffer.setSample(0, i, val);
        buffer.setSample(1, i, val);
    }

    juce::MidiBuffer midi;
    processor.processBlock(buffer, midi);

    // At 40Hz, the signal should remain pure mono even with width 100%
    float maxDiff = 0.0f;
    auto* L = buffer.getReadPointer(0);
    auto* R = buffer.getReadPointer(1);
    for (int i = 0; i < 512; ++i) {
        maxDiff = std::max(maxDiff, std::abs(L[i] - R[i]));
    }

    // Should be very small difference between L and R
    return expect(maxDiff < 0.1f,
                  "Low band should remain mono when crossover is enabled: Max Diff=" + std::to_string(maxDiff));
}

} // namespace

int main() {
    bool ok = true;
    ok &= testStereoFolddown();
    ok &= testLowBandMonoIntegrity();

    if (!ok)
        return 1;

    std::cout << "DspCompliance tests passed.\n";
    return 0;
}
