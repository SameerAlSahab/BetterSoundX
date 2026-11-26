#include "ALCDSP.hpp"
#include <algorithm>
#include <cmath>

namespace alcdsp {
namespace { constexpr double kPi = 3.1415926535897932384626433832795; }

float Engine::Biquad::Run(float x) noexcept {
    const double y = b0 * x + z1;
    z1 = b1 * x - a1 * y + z2;
    z2 = b2 * x - a2 * y;
    return static_cast<float>(y);
}

Engine::Biquad Engine::LowShelf(double fs, double f0, double gainDb) noexcept {
    Biquad c;
    const double A = std::pow(10.0, gainDb / 40.0);
    const double w0 = 2.0 * kPi * f0 / fs;
    const double cs = std::cos(w0), sn = std::sin(w0);
    const double alpha = sn / 2.0 * std::sqrt(2.0);
    const double beta = 2.0 * std::sqrt(A) * alpha;
    const double a0 = (A + 1.0) + (A - 1.0) * cs + beta;
    c.b0 = A * ((A + 1.0) - (A - 1.0) * cs + beta) / a0;
    c.b1 = 2.0 * A * ((A - 1.0) - (A + 1.0) * cs) / a0;
    c.b2 = A * ((A + 1.0) - (A - 1.0) * cs - beta) / a0;
    c.a1 = -2.0 * ((A - 1.0) + (A + 1.0) * cs) / a0;
    c.a2 = ((A + 1.0) + (A - 1.0) * cs - beta) / a0;
    return c;
}

Engine::Biquad Engine::Peaking(double fs, double f0, double gainDb, double q) noexcept {
    Biquad c;
    const double A = std::pow(10.0, gainDb / 40.0);
    const double w0 = 2.0 * kPi * f0 / fs;
    const double cs = std::cos(w0), alpha = std::sin(w0) / (2.0 * q);
    const double a0 = 1.0 + alpha / A;
    c.b0 = (1.0 + alpha * A) / a0;
    c.b1 = (-2.0 * cs) / a0;
    c.b2 = (1.0 - alpha * A) / a0;
    c.a1 = (-2.0 * cs) / a0;
    c.a2 = (1.0 - alpha / A) / a0;
    return c;
}

Engine::Engine(const Config& config) : config_(config) {}

bool Engine::Prepare(double sampleRate) noexcept {
    if (!(sampleRate >= 22050.0 && sampleRate <= 384000.0)) return false;
    if (config_.outputGain < 1.0f || config_.outputGain > 1.5f) return false;
    if (config_.bassCornerHz <= 0 || config_.bassCornerHz >= sampleRate * 0.45) return false;
    sampleRate_ = sampleRate;
    for (int ch=0; ch<2; ++ch) {
        bass_[ch] = LowShelf(sampleRate, config_.bassCornerHz, config_.bassGainDb);
        mud_[ch] = Peaking(sampleRate, config_.mudCenterHz, config_.mudCutDb, config_.mudQ);
    }
    Reset();
    return true;
}

void Engine::Reset() noexcept {
    for (int ch=0; ch<2; ++ch) { bass_[ch].z1=bass_[ch].z2=0; mud_[ch].z1=mud_[ch].z2=0; }
    meter_ = Meter{};
}

float Engine::SoftLimit(float x, float threshold, float ceiling) noexcept {
    const float ax = std::fabs(x);
    if (ax <= threshold) return x;
    const float span = std::max(0.0001f, ceiling - threshold);
    const float compressed = threshold + span * std::tanh((ax - threshold) / span);
    return std::copysign(std::min(compressed, ceiling), x);
}

void Engine::Process(float* samples, std::size_t frames) noexcept {
    if (!samples || sampleRate_ == 0) return;
    float inPeak = 0.0f, outPeak = 0.0f;
    uint64_t clipped = 0;
    for (std::size_t i=0; i<frames; ++i) {
        for (int ch=0; ch<2; ++ch) {
            const std::size_t ix = i * 2 + static_cast<std::size_t>(ch);
            const float input = samples[ix];
            inPeak = std::max(inPeak, std::fabs(input));
            float x = input * config_.outputGain;
            x = bass_[ch].Run(x);
            x = mud_[ch].Run(x);
            const float y = SoftLimit(x, config_.limiterThreshold, config_.limiterCeiling);
            if (std::fabs(x) > config_.limiterThreshold) ++clipped;
            samples[ix] = y;
            outPeak = std::max(outPeak, std::fabs(y));
        }
    }
    meter_.inputPeak = inPeak;
    meter_.outputPeak = outPeak;
    meter_.clippedSamples += clipped;
}

} // namespace alcdsp
