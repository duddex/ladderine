// Copyright (C) 2026 duddex
// SPDX-License-Identifier: AGPL-3.0-or-later

/*
    Ladderine - analogue-flavoured ADSR contour.

    Each stage is an exponential approach, the way an RC network behaves:
    the attack reaches ~99% of its target in the set time, decay and release
    fall to within ~1% in theirs. Sustain is tracked with a slow one-pole so
    moving the sustain knob does not step.
*/
#pragma once

#include "Types.h"

namespace ladderine
{
class Envelope
{
public:
    enum Stage { Idle = 0, Attack, Decay, Sustain, Release };

    void setSampleRate (double sampleRate) noexcept
    {
        sr_ = sampleRate > 0.0 ? sampleRate : 48000.0;
        updateCoefficients();
    }

    void setParams (const EnvParams& p) noexcept
    {
        params_ = p;
        updateCoefficients();
    }

    void noteOn() noexcept
    {
        stage_ = Attack;
    }

    void noteOff() noexcept
    {
        if (stage_ != Idle)
            stage_ = Release;
    }

    void reset() noexcept
    {
        stage_ = Idle;
        value_ = 0.0f;
    }

    bool isActive() const noexcept { return stage_ != Idle; }
    bool isReleasing() const noexcept { return stage_ == Release; }
    int stage() const noexcept { return stage_; }
    float value() const noexcept { return value_; }

    float process() noexcept
    {
        switch (stage_)
        {
            case Attack:
                value_ += (1.0f - value_) * attackCoef_;

                if (value_ >= 0.9990f)
                {
                    value_ = 1.0f;
                    stage_ = Decay;
                }
                break;

            case Decay:
                value_ += (params_.sustain - value_) * decayCoef_;

                if (std::abs (value_ - params_.sustain) < 0.0005f)
                {
                    value_ = params_.sustain;
                    stage_ = Sustain;
                }
                break;

            case Sustain:
                value_ += (params_.sustain - value_) * decayCoef_;
                break;

            case Release:
                value_ -= value_ * releaseCoef_;

                if (value_ <= 0.00005f)
                {
                    value_ = 0.0f;
                    stage_ = Idle;
                }
                break;

            default:
                value_ = 0.0f;
                break;
        }

        return value_;
    }

private:
    void updateCoefficients() noexcept
    {
        attackCoef_  = coefficient (params_.attack);
        decayCoef_   = coefficient (params_.decay);
        releaseCoef_ = coefficient (params_.release);
    }

    float coefficient (float seconds) const noexcept
    {
        // 99% of the way in the requested time.
        constexpr float ln100 = 4.60517019f;
        const float t = std::max (seconds, 0.0005f);
        return 1.0f - std::exp (-ln100 / (t * (float) sr_));
    }

    double sr_ = 48000.0;
    EnvParams params_;
    float value_ = 0.0f;
    float attackCoef_ = 1.0f;
    float decayCoef_ = 1.0f;
    float releaseCoef_ = 1.0f;
    int stage_ = Idle;
};
} // namespace ladderine
