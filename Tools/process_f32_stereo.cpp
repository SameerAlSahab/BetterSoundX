#include "ALCDSP.hpp"
#include <cstdio>
#include <cstdlib>
#include <vector>

// Usage: dsp_process input.f32 output.f32 [sample-rate]
// Files contain native-endian interleaved stereo IEEE-754 float32 samples.
int main(int argc, char** argv) {
    if (argc < 3 || argc > 4) {
        std::fprintf(stderr, "usage: %s input.f32 output.f32 [sample-rate]\n", argv[0]); return 2;
    }
    const double rate = argc == 4 ? std::atof(argv[3]) : 48000.0;
    FILE* in = std::fopen(argv[1], "rb");
    if (!in) { std::perror("input"); return 1; }
    FILE* out = std::fopen(argv[2], "wb");
    if (!out) { std::perror("output"); std::fclose(in); return 1; }
    alcdsp::Engine dsp;
    if (!dsp.Prepare(rate)) { std::fprintf(stderr, "unsupported sample rate\n"); return 1; }
    std::vector<float> block(1024 * 2);
    while (true) {
        const std::size_t n = std::fread(block.data(), sizeof(float) * 2, 1024, in);
        if (n) {
            dsp.Process(block.data(), n);
            if (std::fwrite(block.data(), sizeof(float) * 2, n, out) != n) { std::perror("write"); return 1; }
        }
        if (n < 1024) { if (std::ferror(in)) { std::perror("read"); return 1; } break; }
    }
    std::fclose(in); std::fclose(out);
    const auto& m = dsp.GetMeter();
    std::fprintf(stderr, "input peak %.3f | output peak %.3f | limiter samples %llu\n",
                 m.inputPeak, m.outputPeak, static_cast<unsigned long long>(m.clippedSamples));
    return 0;
}
