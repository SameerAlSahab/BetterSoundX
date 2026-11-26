#include "ALCDSP.hpp"
#include <cmath>
#include <cstdio>
#include <vector>

static bool near(float a, float b, float e) { return std::fabs(a-b) < e; }
int main() {
    alcdsp::Engine engine;
    if (!engine.Prepare(48000.0)) return 1;
    // A low-level DC test checks the fixed 1.5x gain without triggering limiter.
    std::vector<float> dc(4096 * 2, 0.05f);
    engine.Process(dc.data(), 4096);
    const float steady = dc[dc.size()-2];
    if (!(steady > 0.130f && steady < 0.150f)) {
        std::fprintf(stderr, "gain test failed: %.6f\n", steady); return 2;
    }
    // Large input must remain bounded by the configured limiter ceiling.
    std::vector<float> hot(512 * 2, 0.8f);
    engine.Process(hot.data(), 512);
    for (float x : hot) if (!std::isfinite(x) || std::fabs(x) > 0.986f) return 3;
    // Frequency response: low bass should be measurably stronger than 1 kHz.
    auto measure = [](double hz) {
        alcdsp::Engine e;
        e.Prepare(48000.0);
        const int n = 48000;
        std::vector<float> tone(static_cast<std::size_t>(n) * 2);
        for (int i=0; i<n; ++i) {
            const float v = 0.02f * std::sin(static_cast<float>(2.0 * 3.141592653589793 * hz * i / 48000.0));
            tone[static_cast<std::size_t>(i)*2] = tone[static_cast<std::size_t>(i)*2+1] = v;
        }
        e.Process(tone.data(), tone.size()/2);
        double sum = 0.0;
        for (std::size_t i=4000*2; i<tone.size(); ++i) sum += tone[i]*tone[i];
        return std::sqrt(sum / static_cast<double>(tone.size()-8000));
    };
    const double bassRms = measure(60.0);
    const double midRms = measure(1000.0);
    if (!(bassRms > midRms * 1.45)) {
        std::fprintf(stderr, "bass curve test failed: bass %.6f mid %.6f\n", bassRms, midRms); return 6;
    }
    // Null pointer and zero frames must be safe.
    engine.Process(nullptr, 1);
    engine.Process(hot.data(), 0);
    // Sample-rate validation.
    if (engine.Prepare(8000.0)) return 4;
    if (!engine.Prepare(44100.0)) return 5;
    std::puts("ALCDSP tests passed");
    return 0;
}
