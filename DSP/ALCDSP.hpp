#pragma once
#include <cstddef>
#include <cstdint>

namespace alcdsp {

struct Config {
    float outputGain = 1.50f;       // +3.5218 dB
    float bassGainDb = 5.5f;        // punchy low shelf
    float bassCornerHz = 105.0f;
    float mudCutDb = -1.5f;         // reduce 250 Hz congestion
    float mudCenterHz = 250.0f;
    float mudQ = 0.85f;
    float limiterThreshold = 0.94f;
    float limiterCeiling = 0.985f;
};

struct Meter {
    float inputPeak = 0.0f;
    float outputPeak = 0.0f;
    uint64_t clippedSamples = 0;
};

// Real-time-safe, allocation-free stereo float DSP. Call Reset() when the
// stream is restarted or its sample rate changes. Process() is in-place.
class Engine final {
public:
    explicit Engine(const Config& config = Config{});
    bool Prepare(double sampleRate) noexcept;
    void Reset() noexcept;
    void Process(float* interleavedStereo, std::size_t frames) noexcept;
    const Meter& GetMeter() const noexcept { return meter_; }
    const Config& GetConfig() const noexcept { return config_; }

private:
    struct Biquad {
        double b0=1, b1=0, b2=0, a1=0, a2=0;
        double z1=0, z2=0;
        inline float Run(float x) noexcept;
    };
    static Biquad LowShelf(double fs, double f0, double gainDb) noexcept;
    static Biquad Peaking(double fs, double f0, double gainDb, double q) noexcept;
    static float SoftLimit(float x, float threshold, float ceiling) noexcept;

    Config config_;
    Biquad bass_[2];
    Biquad mud_[2];
    double sampleRate_ = 0;
    Meter meter_{};
};

} // namespace alcdsp
