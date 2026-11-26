#pragma once
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct ALCDSPContext ALCDSPContext;

typedef struct ALCDSPMeter {
    float input_peak;
    float output_peak;
    uint64_t limited_samples;
} ALCDSPMeter;

// Create/destroy/prepare only on a non-real-time control thread.
ALCDSPContext* alcdsp_create(void);
void alcdsp_destroy(ALCDSPContext* context);
int alcdsp_prepare(ALCDSPContext* context, double sample_rate);
void alcdsp_reset(ALCDSPContext* context);
// Call from the HAL mix callback: interleaved stereo Float32, in-place.
// No allocation, locks, logging, file I/O, or CoreAudio client calls occur here.
void alcdsp_process_f32_stereo(ALCDSPContext* context, float* samples, size_t frames);
void alcdsp_get_meter(const ALCDSPContext* context, ALCDSPMeter* meter);

#ifdef __cplusplus
}
#endif
