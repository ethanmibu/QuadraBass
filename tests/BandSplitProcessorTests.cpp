#include "../src/dsp/BandSplitProcessor.h"
#include <cmath>
#include <iostream>

namespace {

bool expect(bool condition, const std::string& message) {
    if (condition)
        return true;
    std::cerr << "FAIL: " << message << '\n';
    return false;
}

bool testCrossoverEnabled() {
    qbdsp::BandSplitProcessor processor;
    juce::dsp::ProcessSpec spec{48000.0, 512, 1};
    processor.prepare(spec);

    juce::AudioBuffer<float> input(1, 512);
    juce::AudioBuffer<float> low(1, 512);
    juce::AudioBuffer<float> high(1, 512);

    for (int i = 0; i < 512; ++i) {
        input.setSample(0, i, std::sin(2.0f * 3.14159265359f * 100.0f * static_cast<float>(i) / 48000.0f));
    }

    processor.process(input, low, high, 90.0f, true);

    bool hasLow = false;
    for (int i = 0; i < 512; ++i) {
        if (std::abs(low.getSample(0, i)) > 1e-4) {
            hasLow = true;
        }
    }
    if (!hasLow)
        std::cerr << "low is empty! first 5 samples: " << low.getSample(0, 0) << " " << low.getSample(0, 1) << " "
                  << low.getSample(0, 2) << "\n";

    return expect(hasLow, "Low buffer should be populated when crossoverEnabled=true");
}

bool testCrossoverBypassed() {
    qbdsp::BandSplitProcessor processor;
    juce::dsp::ProcessSpec spec{48000.0, 512, 1};
    processor.prepare(spec);

    juce::AudioBuffer<float> input(1, 512);
    juce::AudioBuffer<float> low(1, 512);
    juce::AudioBuffer<float> high(1, 512);

    for (int i = 0; i < 512; ++i) {
        input.setSample(0, i, 0.5f);
    }

    processor.process(input, low, high, 90.0f, false);

    bool ok = true;
    for (int i = 0; i < 512; ++i) {
        if (std::abs(low.getSample(0, i)) > 1e-4) {
            ok = false;
            break;
        }
    }
    for (int i = 0; i < 512; ++i) {
        if (std::abs(high.getSample(0, i) - 0.5f) > 1e-4) {
            std::cerr << "high mismatch at " << i << ": " << high.getSample(0, i) << "\n";
            ok = false;
            break;
        }
    }
    ok &= expect(ok, "When crossoverEnabled=false, low=0 and high=input");
    return ok;
}

} // namespace

int main() {
    bool ok = true;
    ok &= testCrossoverEnabled();
    ok &= testCrossoverBypassed();

    if (!ok)
        return 1;

    std::cout << "BandSplitProcessor tests passed.\n";
    return 0;
}
