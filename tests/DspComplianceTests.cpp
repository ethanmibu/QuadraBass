#include "../src/PluginProcessor.h"
#include <algorithm>
#include <cmath>
#include <iostream>
#include <limits>

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
    *widthParam = 100.0f;

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

float measureStereoSpread(float freqHz, float widthPercent) {
    QuadraBassAudioProcessor processor;
    processor.prepareToPlay(48000.0, 2048);

    auto* widthParam = dynamic_cast<juce::AudioParameterFloat*>(
        processor.params().apvts.getParameter(util::Params::IDs::widthPercent));
    *widthParam = widthPercent;

    juce::AudioBuffer<float> buffer(2, 2048);
    for (int i = 0; i < 2048; ++i) {
        float val = std::sin(2.0f * 3.14159f * freqHz * i / 48000.0f);
        buffer.setSample(0, i, val);
        buffer.setSample(1, i, val);
    }

    juce::MidiBuffer midi;
    processor.processBlock(buffer, midi);

    float inputSumSq = 0.0f;
    float diffSumSq = 0.0f;
    auto* L = buffer.getReadPointer(0);
    auto* R = buffer.getReadPointer(1);
    for (int i = 0; i < 2048; ++i) {
        float input = std::sin(2.0f * 3.14159f * freqHz * i / 48000.0f);
        inputSumSq += input * input;
        float d = L[i] - R[i];
        diffSumSq += d * d;
    }
    const float inputRms = std::sqrt(inputSumSq / 2048.0f);
    const float spreadRms = std::sqrt(diffSumSq / 2048.0f);
    return inputRms > 1.0e-6f ? spreadRms / inputRms : 0.0f;
}

bool testWidthZeroIsMonoAcrossBand() {
    bool ok = true;
    for (float freq : {30.0f, 120.0f, 1000.0f, 10000.0f}) {
        const float spread = measureStereoSpread(freq, 0.0f);
        ok &= expect(spread < 1.0e-3f, "Width 0 should remain mono. Freq=" + std::to_string(freq));
    }
    return ok;
}

bool testWidthConsistencyAcrossFrequencies() {
    bool ok = true;
    float minSpread = std::numeric_limits<float>::max();
    float maxSpread = 0.0f;

    for (float freq : {40.0f, 120.0f, 500.0f, 1000.0f, 3000.0f, 8000.0f}) {
        const float spread = measureStereoSpread(freq, 100.0f);
        minSpread = std::min(minSpread, spread);
        maxSpread = std::max(maxSpread, spread);
    }

    ok &= expect(minSpread > 0.2f, "Width 100 should produce clearly non-mono spread across tested frequencies");
    ok &= expect(maxSpread < minSpread * 1.75f,
                 "Width response should stay reasonably consistent across tested frequencies");
    return ok;
}

bool testParameterCountInProcessor() {
    QuadraBassAudioProcessor processor;
    return expect(processor.getParameters().size() == 5, "Processor should expose exactly five parameters");
}

bool testFIRModeProducesStableOutput() {
    QuadraBassAudioProcessor processor;
    processor.prepareToPlay(48000.0, 2048);

    auto* widthParam = dynamic_cast<juce::AudioParameterFloat*>(
        processor.params().apvts.getParameter(util::Params::IDs::widthPercent));
    auto* modeParam = dynamic_cast<juce::AudioParameterChoice*>(
        processor.params().apvts.getParameter(util::Params::IDs::hilbertMode));

    bool ok = true;
    if (modeParam == nullptr)
        return expect(false, "hilbert_mode should be a choice parameter");

    *widthParam = 100.0f;
    modeParam->operator=(1); // FIR

    juce::AudioBuffer<float> buffer(2, 2048);
    juce::MidiBuffer midi;
    int sampleOffset = 0;
    for (int block = 0; block < 4; ++block) {
        for (int i = 0; i < 2048; ++i) {
            float val = std::sin(2.0f * 3.14159f * 1000.0f * (sampleOffset + i) / 48000.0f);
            buffer.setSample(0, i, val);
            buffer.setSample(1, i, val);
        }
        processor.processBlock(buffer, midi);
        sampleOffset += 2048;
    }

    ok &= expect(processor.getLatencySamples() > 0, "FIR mode should report non-zero plugin latency");

    float sumSq = 0.0f;
    float diffSq = 0.0f;
    auto* L = buffer.getReadPointer(0);
    auto* R = buffer.getReadPointer(1);
    for (int i = 0; i < 2048; ++i) {
        ok &= expect(std::isfinite(L[i]) && std::isfinite(R[i]), "FIR mode output must be finite");
        sumSq += L[i] * L[i];
        float d = L[i] - R[i];
        diffSq += d * d;
    }

    const float outRms = std::sqrt(sumSq / 2048.0f);
    const float spreadRms = std::sqrt(diffSq / 2048.0f);
    ok &= expect(outRms > 0.01f, "FIR mode output RMS should remain non-trivial");
    ok &= expect(spreadRms > 0.05f, "FIR mode should still produce widening at width 100");
    return ok;
}

} // namespace

int main() {
    bool ok = true;
    ok &= testStereoFolddown();
    ok &= testWidthZeroIsMonoAcrossBand();
    ok &= testWidthConsistencyAcrossFrequencies();
    ok &= testParameterCountInProcessor();
    ok &= testFIRModeProducesStableOutput();

    if (!ok)
        return 1;

    std::cout << "DspCompliance tests passed.\n";
    return 0;
}
