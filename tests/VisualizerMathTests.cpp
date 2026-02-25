#include "../src/ui/CorrelationMeter.h"
#include "../src/ui/GoniometerComponent.h"
#include <cmath>
#include <iostream>
#include <string>

namespace {

bool expect(bool condition, const std::string& message) {
    if (condition)
        return true;
    std::cerr << "FAIL: " << message << '\n';
    return false;
}

bool testCorrelationMath() {
    qbui::CorrelationMeter meter;

    // Mono signal = 1.0 correlation
    float leftMono[512];
    float rightMono[512];
    for (int i = 0; i < 512; ++i) {
        leftMono[i] = std::sin(2.0f * 3.14159f * 100.0f * i / 48000.0f);
        rightMono[i] = leftMono[i];
    }

    meter.processBlock(leftMono, rightMono, 512);
    float corrMono = meter.getCorrelation();
    bool ok = expect(std::abs(corrMono - 1.0f) < 1e-4f, "Mono should have correlation 1.0");

    // Anti-phase signal = -1.0 correlation
    meter.reset();
    float leftAnti[512];
    float rightAnti[512];
    for (int i = 0; i < 512; ++i) {
        leftAnti[i] = std::sin(2.0f * 3.14159f * 100.0f * i / 48000.0f);
        rightAnti[i] = -leftAnti[i];
    }

    meter.processBlock(leftAnti, rightAnti, 512);
    float corrAnti = meter.getCorrelation();
    ok &= expect(std::abs(corrAnti - (-1.0f)) < 1e-4f, "Anti-phase should have correlation -1.0");

    // Quadrature signal = 0.0 correlation
    meter.reset();
    float leftQuad[4800];
    float rightQuad[4800];
    for (int i = 0; i < 4800; ++i) {
        leftQuad[i] = std::sin(2.0f * 3.14159f * 100.0f * i / 48000.0f);
        // 90 degrees out of phase
        rightQuad[i] = std::cos(2.0f * 3.14159f * 100.0f * i / 48000.0f);
    }

    meter.processBlock(leftQuad, rightQuad, 4800);
    float corrQuad = meter.getCorrelation();
    if (std::abs(corrQuad) >= 1e-2f)
        std::cerr << "corrQuad is " << corrQuad << "\n";
    ok &= expect(std::abs(corrQuad) < 1e-2f, "Quadrature should have near 0.0 correlation");

    return ok;
}

bool testXYMapping() {
    qbui::GoniometerComponent goniometer;

    float L = 1.0f;
    float R = 0.0f;

    float x, y;
    goniometer.mapXY(L, R, x, y);

    bool ok = expect(std::abs(x - 0.7071f) < 1e-3f, "L=1, R=0 should map to x=0.707");
    ok &= expect(std::abs(y - 0.7071f) < 1e-3f, "L=1, R=0 should map to y=0.707");

    L = 0.0f;
    R = 1.0f;
    goniometer.mapXY(L, R, x, y);
    ok &= expect(std::abs(x - (-0.7071f)) < 1e-3f, "L=0, R=1 should map to x=-0.707");
    ok &= expect(std::abs(y - 0.7071f) < 1e-3f, "L=0, R=1 should map to y=0.707");

    L = 1.0f;
    R = -1.0f;
    goniometer.mapXY(L, R, x, y);
    ok &= expect(std::abs(x - 1.4142f) < 1e-3f, "Anti-phase L=1, R=-1 should map to x=1.414");
    ok &= expect(std::abs(y - 0.0f) < 1e-3f, "Anti-phase L=1, R=-1 should map to y=0.0");

    return ok;
}

} // namespace

int main() {
    bool ok = true;
    ok &= testCorrelationMath();
    ok &= testXYMapping();

    if (!ok)
        return 1;

    std::cout << "VisualizerMath tests passed.\n";
    return 0;
}
