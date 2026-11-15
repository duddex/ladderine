/*
    Ladderine - zero-delay-feedback 4-pole ladder low-pass.

    Four TPT one-pole sections with the Moog feedback path solved at sample
    rate (two fixed-point iterations around the tanh in the feedback), so the
    cutoff tracks accurately up to Nyquist and the filter stays stable at any
    resonance. Above ~4.1 the loop self-oscillates, as the hardware does.

    The ladder is deliberately nonlinear: the tanh in the feedback path is
    what gives the resonance its compression and the filter its voice.
*/
#pragma once

#include "Types.h"

namespace ladderine
{
class LadderFilter
{
public:
    void prepare (double sampleRate) noexcept
    {
        sr_ = sampleRate > 0.0 ? sampleRate : 48000.0;
        reset();
    }

    void reset() noexcept
    {
        for (auto& z : z_)
            z = 0.0f;
    }

    void setCutoff (float hz) noexcept
    {
        const double nyquistLimit = sr_ * 0.45;
        const double fc = std::clamp ((double) hz, 12.0, nyquistLimit);
        const double g = std::tan (jucePi * fc / sr_);
        G_ = (float) (g / (1.0 + g));
    }

    // k = 0..~4.5, drive >= 1
    float process (float x, float k, float drive) noexcept
    {
        if (drive > 1.0001f)
            x = std::tanh (x * drive) / drive;

        const float G = G_;
        const float omg = 1.0f - G;
        const float s1 = omg * z_[0];
        const float s2 = omg * z_[1];
        const float s3 = omg * z_[2];
        const float s4 = omg * z_[3];

        const float A = G * G * G * G;
        const float B = G * G * G * s1 + G * G * s2 + G * s3 + s4;

        // Solve u = x - k * tanh (A * u + B) by fixed-point iteration.
        float u = x - k * std::tanh (B);

        for (int i = 0; i < 2; ++i)
            u = x - k * std::tanh (A * u + B);

        float v = (u - z_[0]) * G;
        const float y1 = v + z_[0];
        z_[0] = y1 + v;

        v = (y1 - z_[1]) * G;
        const float y2 = v + z_[1];
        z_[1] = y2 + v;

        v = (y2 - z_[2]) * G;
        const float y3 = v + z_[2];
        z_[2] = y3 + v;

        v = (y3 - z_[3]) * G;
        const float y4 = v + z_[3];
        z_[3] = y4 + v;

        return y4;
    }

private:
    static constexpr double jucePi = 3.14159265358979323846;

    double sr_ = 48000.0;
    float G_ = 0.0f;
    float z_[4] { 0.0f, 0.0f, 0.0f, 0.0f };
};
} // namespace ladderine
