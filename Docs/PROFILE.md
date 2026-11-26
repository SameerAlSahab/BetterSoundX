# ALCDSP v0.2 Punchy profile

| Stage | Setting | Purpose |
|---|---:|---|
| Pre-gain | 1.50x / +3.52 dB | Fixed maximum boost requested |
| Low shelf | +5.5 dB @ 105 Hz | Weight and punch in kick/bass |
| Peaking EQ | -1.5 dB @ 250 Hz, Q 0.85 | Reduce boxiness/mud |
| Soft limiter | threshold 0.94, ceiling 0.985 | Keep boosted peaks from hard-clipping |

The gain stage is 1.5x for samples that remain below the limiter. The limiter necessarily reduces gain on hot peaks; otherwise digital clipping would occur. The 150% setting is amplitude gain, not a promise of 50% more perceived loudness or 50% more physical speaker power.
