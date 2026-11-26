#pragma once
#include "../DSP/ALCDSP_C.h"

// The eventual AudioServerPlugIn adapter owns one ALCDSPContext per device.
// Create + Prepare on Initialize/StartIO (control thread), invoke Process only
// from the driver's post-mix real-time callback, and destroy after StopIO.
// For a stereo Float32 interleaved mix:
//     alcdsp_process_f32_stereo(context, (float*)ioMainBuffer->mBuffers[0].mData, frames);
// Verify the stream format and buffer channel layout before making this call.
