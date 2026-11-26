#include "ALCDSP.hpp"
#include "ALCDSP_C.h"

struct ALCDSPContext {
    ALCDSPContext() : engine() {}
    alcdsp::Engine engine;
};

extern "C" ALCDSPContext* alcdsp_create(void) {
    return new ALCDSPContext();
}
extern "C" void alcdsp_destroy(ALCDSPContext* context) { delete context; }
extern "C" int alcdsp_prepare(ALCDSPContext* context, double sample_rate) {
    return context && context->engine.Prepare(sample_rate) ? 1 : 0;
}
extern "C" void alcdsp_reset(ALCDSPContext* context) {
    if (context) context->engine.Reset();
}
extern "C" void alcdsp_process_f32_stereo(ALCDSPContext* context, float* samples, size_t frames) {
    if (context) context->engine.Process(samples, frames);
}
extern "C" void alcdsp_get_meter(const ALCDSPContext* context, ALCDSPMeter* meter) {
    if (!context || !meter) return;
    const auto& m = context->engine.GetMeter();
    meter->input_peak = m.inputPeak;
    meter->output_peak = m.outputPeak;
    meter->limited_samples = m.clippedSamples;
}
