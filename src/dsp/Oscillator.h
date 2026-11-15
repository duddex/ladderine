/*
    Ladderine - band-limited oscillator.

    The Model D's six waveform positions are approximated with polyBLEP
    saw / pulse waves and naive triangle shapes (their harmonics fall off
    fast enough that aliasing stays inaudible for a mono instrument).
    Oscillators free-run: notes never reset phase, which is part of the
    original's unstable, living character.
*/
#pragma once

#include "Types.h"

namespace ladderine
{
class Oscillator
{
public:
    void reset (double phase = 0.0) noexcept
    {
        phase_ = phase;
    }

    void setFrequency (double hz, double sampleRate) noexcept
    {
        // Leave a little headroom below Nyquist; above this a polyBLEP
        // oscillator only produces aliases anyway.
        inc_ = std::clamp (hz / sampleRate, 0.0, 0.45);
    }

    void advance() noexcept
    {
        phase_ += inc_;

        if (phase_ >= 1.0)
            phase_ -= 1.0;
    }

    double phase() const noexcept { return phase_; }
    double increment() const noexcept { return inc_; }

    float next (int wave) noexcept
    {
        const float v = shape (wave, phase_, inc_);
        advance();
        return v;
    }

    // polyBLEP correction around a unit phase reset
    static double polyBlep (double t, double dt) noexcept
    {
        if (dt <= 0.0)
            return 0.0;

        if (t < dt)
        {
            t /= dt;
            return t + t - t * t - 1.0;
        }

        if (t > 1.0 - dt)
        {
            t = (t - 1.0) / dt;
            return t * t + t + t + 1.0;
        }

        return 0.0;
    }

    static float shape (int wave, double t, double dt) noexcept
    {
        switch (wave)
        {
            case WaveTri:
                return (float) (1.0 - 4.0 * std::abs (t - 0.5));

            case WaveTriSaw:
            {
                // The original's second position: a shark-fin saw with a
                // rounded (triangular) rising edge and a fast fall.
                constexpr double knee = 0.88;

                if (t < knee)
                    return (float) (2.0 * t / knee - 1.0);

                return (float) (1.0 - 2.0 * (t - knee) / (1.0 - knee));
            }

            case WaveSaw:
                return (float) (2.0 * t - 1.0 - polyBlep (t, dt));

            case WaveSquare:
            {
                double v = t < 0.5 ? 1.0 : -1.0;
                v += polyBlep (t, dt);

                double u = t + 0.5;

                if (u >= 1.0)
                    u -= 1.0;

                v -= polyBlep (u, dt);
                return (float) v;
            }

            case WavePulse:
                return pulse (t, dt, 0.25);

            case WaveNarrow:
                return pulse (t, dt, 0.12);

            default:
                return 0.0f;
        }
    }

private:
    static float pulse (double t, double dt, double width) noexcept
    {
        double v = t < width ? 1.0 : -1.0;
        v += polyBlep (t, dt);

        double u = t + (1.0 - width);

        if (u >= 1.0)
            u -= 1.0;

        v -= polyBlep (u, dt);
        return (float) v;
    }

    double phase_ = 0.0;
    double inc_ = 0.0;
};
} // namespace ladderine
