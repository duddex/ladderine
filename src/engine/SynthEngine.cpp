// Copyright (C) 2026 duddex
// SPDX-License-Identifier: AGPL-3.0-or-later

#include "SynthEngine.h"

namespace ladderine
{
namespace
{
inline float softClip (float x) noexcept
{
    constexpr float knee = 0.8f;

    if (x > knee)
        return knee + std::tanh ((x - knee) / (1.0f - knee)) * (1.0f - knee);

    if (x < -knee)
        return -knee - std::tanh ((-x - knee) / (1.0f - knee)) * (1.0f - knee);

    return x;
}

constexpr double kFootage[NumRanges] = { 0.25, 0.5, 1.0, 2.0, 4.0 };
constexpr float kKbdAmount[NumKbdTracks] = { 0.0f, 1.0f / 3.0f, 2.0f / 3.0f, 1.0f };
} // namespace

SynthEngine::SynthEngine()
{
    prepare (48000.0);
}

void SynthEngine::prepare (double sampleRate)
{
    sr_ = sampleRate > 0.0 ? sampleRate : 48000.0;

    ampEnv_.setSampleRate (sr_);
    filterEnv_.setSampleRate (sr_);
    ladder_.prepare (sr_);

    driftCoef_ = 1.0f - std::exp (-1.0f / (0.25f * (float) sr_));
    dcR_ = (float) std::exp (-2.0 * 3.141592653589793 * 5.0 / sr_);

    reset();
}

void SynthEngine::reset()
{
    ampEnv_.reset();
    filterEnv_.reset();
    ladder_.reset();

    numHeld_ = 0;
    currentNote_ = -1;
    sustain_ = false;
    playedSinceReset_ = false;

    dcX1_ = 0.0f;
    dcY1_ = 0.0f;

    currentNoteF_ = targetNoteF_ = 60.0f;
    bendSm_ = bendTarget_;
    modWheelSm_ = params_.modWheel;

    cutoffSm_ = params_.cutoffHz;
    emphasisSm_ = params_.emphasis;
    volumeSm_ = dbToGain (params_.volumeDb);
    tuneSm_ = params_.tuneSemis;
    noiseSm_ = params_.noiseLevel;
    driveSm_ = params_.drive;

    for (int i = 0; i < 3; ++i)
        levelSm_[i] = params_.oscLevel[i];

    for (auto& d : drift_)
        d = 0.0f;

    for (auto& d : driftTarget_)
        d = 0.0f;

    driftCountdown_ = 0;
    lastMod_ = 0.0f;
}

void SynthEngine::setParams (const EngineParams& p)
{
    params_ = p;

    params_.cutoffHz = clampf (params_.cutoffHz, 20.0f, 20000.0f);
    params_.emphasis = clampf (params_.emphasis, 0.0f, 1.0f);
    params_.contour = clampf (params_.contour, 0.0f, 1.0f);
    params_.modMix = clampf (params_.modMix, 0.0f, 1.0f);
    params_.modDepth = clampf (params_.modDepth, 0.0f, 1.0f);
    params_.modWheel = clampf (params_.modWheel, 0.0f, 1.0f);
    params_.drift = clampf (params_.drift, 0.0f, 1.0f);
    params_.tuneSemis = clampf (params_.tuneSemis, -1.0f, 1.0f);
    params_.velToAmp = clampf (params_.velToAmp, 0.0f, 1.0f);
    params_.velToFilter = clampf (params_.velToFilter, 0.0f, 1.0f);
    params_.bendRange = std::clamp (params_.bendRange, 1, 12);

    ampEnv_.setParams (params_.ampEnv);
    filterEnv_.setParams (params_.filterEnv);

    const float glide = std::max (params_.glideSeconds, 0.0005f);
    glideCoef_ = params_.glideMode == GlideOff
                     ? 1.0f
                     : 1.0f - std::exp (-4.6f / (glide * (float) sr_));
}

void SynthEngine::noteOn (int midiNote, float velocity)
{
    midiNote = std::clamp (midiNote, 0, 127);

    const bool wasHeld = numHeld_ > 0;

    // last-note priority; a repeated note moves to the top of the stack
    removeHeldNote (midiNote);

    if (numHeld_ < kMaxHeldNotes)
    {
        heldNotes_[numHeld_++] = midiNote;
    }
    else
    {
        for (int i = 1; i < kMaxHeldNotes; ++i)
            heldNotes_[i - 1] = heldNotes_[i];

        heldNotes_[kMaxHeldNotes - 1] = midiNote;
    }

    targetNoteF_ = (float) midiNote;
    currentNote_ = midiNote;

    if (! wasHeld)
    {
        velocity_ = clampf (velocity, 0.0f, 1.0f);
        ampEnv_.noteOn();
        filterEnv_.noteOn();
    }

    // The very first note after power-on must not glide in from the
    // engine's arbitrary start pitch.
    if (! wasHeld && ! playedSinceReset_)
        currentNoteF_ = targetNoteF_;

    playedSinceReset_ = true;

    if (params_.glideMode == GlideOff
        || (params_.glideMode == GlideAuto && ! wasHeld))
        currentNoteF_ = targetNoteF_;
}

void SynthEngine::noteOff (int midiNote)
{
    removeHeldNote (midiNote);

    if (numHeld_ > 0)
    {
        targetNoteF_ = (float) heldNotes_[numHeld_ - 1];
        currentNote_ = heldNotes_[numHeld_ - 1];

        if (params_.glideMode == GlideOff)
            currentNoteF_ = targetNoteF_;
    }
    else
    {
        currentNote_ = -1;
        releaseIfPossible();
    }
}

void SynthEngine::releaseIfPossible()
{
    if (numHeld_ == 0 && ! sustain_)
    {
        ampEnv_.noteOff();
        filterEnv_.noteOff();
    }
}

void SynthEngine::removeHeldNote (int midiNote)
{
    for (int i = 0; i < numHeld_; ++i)
    {
        if (heldNotes_[i] == midiNote)
        {
            for (int j = i + 1; j < numHeld_; ++j)
                heldNotes_[j - 1] = heldNotes_[j];

            --numHeld_;
            return;
        }
    }
}

void SynthEngine::allNotesOff (bool immediate)
{
    numHeld_ = 0;
    currentNote_ = -1;

    if (immediate)
    {
        ampEnv_.reset();
        filterEnv_.reset();
        ladder_.reset();
        dcX1_ = dcY1_ = 0.0f;
    }
    else
    {
        ampEnv_.noteOff();
        filterEnv_.noteOff();
    }
}

void SynthEngine::sustainPedal (bool down)
{
    sustain_ = down;

    if (! down)
        releaseIfPossible();
}

void SynthEngine::pitchBend (float value)
{
    bendTarget_ = clampf (value, -1.0f, 1.0f);
}

void SynthEngine::modWheel (float value)
{
    params_.modWheel = clampf (value, 0.0f, 1.0f);
}

bool SynthEngine::isActive() const noexcept
{
    return numHeld_ > 0 || ampEnv_.isActive();
}

float SynthEngine::nextRandomBipolar()
{
    rng_ ^= rng_ << 13;
    rng_ ^= rng_ >> 17;
    rng_ ^= rng_ << 5;

    return (float) (rng_ >> 8) * (2.0f / 16777216.0f) - 1.0f;
}

float SynthEngine::dcBlock (float x)
{
    const float y = x - dcX1_ + dcR_ * dcY1_;
    dcX1_ = x;
    dcY1_ = y;
    return y;
}

void SynthEngine::updateDrift()
{
    if (--driftCountdown_ <= 0)
    {
        driftCountdown_ = (int) (sr_ * 0.11);

        for (auto& d : driftTarget_)
            d = nextRandomBipolar();
    }

    for (int i = 0; i < 4; ++i)
        drift_[i] += (driftTarget_[i] - drift_[i]) * driftCoef_;
}

void SynthEngine::render (float* output, int numSamples)
{
    if (numSamples <= 0)
        return;

    if (! isActive())
    {
        // Keep the oscillators free-running between notes.
        for (int i = 0; i < numSamples; ++i)
        {
            osc_[0].advance();
            osc_[1].advance();
            osc_[2].advance();
            output[i] = 0.0f;
        }

        lastMod_ = 0.0f;
        return;
    }

    const float kCutoff = 1.0f - std::exp (-1.0f / (0.008f * (float) sr_));
    const float kLevel  = 1.0f - std::exp (-1.0f / (0.015f * (float) sr_));
    const float kNoise  = kLevel;
    const float kDrive  = kLevel;
    const float kEmph   = 1.0f - std::exp (-1.0f / (0.010f * (float) sr_));
    const float kVolume = 1.0f - std::exp (-1.0f / (0.010f * (float) sr_));
    const float kTune   = 1.0f - std::exp (-1.0f / (0.020f * (float) sr_));
    const float kBend   = 1.0f - std::exp (-1.0f / (0.004f * (float) sr_));
    const float kMod    = 1.0f - std::exp (-1.0f / (0.012f * (float) sr_));

    const float volumeTarget = dbToGain (params_.volumeDb);

    for (int i = 0; i < numSamples; ++i)
    {
        // ---- smoothing ----------------------------------------------------
        cutoffSm_   += (params_.cutoffHz - cutoffSm_) * kCutoff;
        emphasisSm_ += (params_.emphasis - emphasisSm_) * kEmph;
        volumeSm_   += (volumeTarget - volumeSm_) * kVolume;
        tuneSm_     += (params_.tuneSemis - tuneSm_) * kTune;
        noiseSm_    += (params_.noiseLevel - noiseSm_) * kNoise;
        driveSm_    += (params_.drive - driveSm_) * kDrive;
        bendSm_     += (bendTarget_ - bendSm_) * kBend;
        modWheelSm_ += (params_.modWheel - modWheelSm_) * kMod;

        for (int k = 0; k < 3; ++k)
            levelSm_[k] += (params_.oscLevel[k] - levelSm_[k]) * kLevel;

        // ---- glide --------------------------------------------------------
        currentNoteF_ += (targetNoteF_ - currentNoteF_) * glideCoef_;

        // ---- contours -----------------------------------------------------
        const float ampLevel = ampEnv_.process();
        const float fenvLevel = filterEnv_.process();

        updateDrift();

        // ---- oscillator 3: audio partial, or the modulation source --------
        const int range3 = std::clamp (params_.oscRange[2], 0, NumOsc3Ranges - 1);
        const int wave3 = std::clamp (params_.oscWave[2], 0, NumWaves - 1);

        const float basePitch = currentNoteF_ + tuneSm_ + bendSm_ * (float) params_.bendRange;

        double hz3 = 0.0;

        if (range3 == Osc3Lo)
        {
            // LO: a free-running 0.1 .. 20 Hz control oscillator.
            hz3 = 0.1 * std::pow (200.0, ((double) params_.oscTune[2] + 7.0) / 14.0);
        }
        else
        {
            const int foot = range3 - 1;
            const double note3 = params_.osc3Kbd ? (double) basePitch : 60.0;
            const double cents = (double) drift_[2] * params_.drift * 12.0;
            hz3 = noteToHz (note3) * kFootage[foot]
                  * std::pow (2.0, ((double) params_.oscTune[2] + cents / 100.0) / 12.0);
        }

        osc_[2].setFrequency (hz3, sr_);
        const float o3 = osc_[2].next (wave3);

        // ---- noise and modulation bus -------------------------------------
        const float noise = nextRandomBipolar();
        const float modSource = lerpf (o3, noise, params_.modMix);
        const float mod = modSource * modWheelSm_ * params_.modDepth;
        lastMod_ = mod;

        // ---- oscillators 1 and 2 ------------------------------------------
        const int range1 = std::clamp (params_.oscRange[0], 0, NumRanges - 1);
        const int range2 = std::clamp (params_.oscRange[1], 0, NumRanges - 1);
        const int wave1 = std::clamp (params_.oscWave[0], 0, NumWaves - 1);
        const int wave2 = std::clamp (params_.oscWave[1], 0, NumWaves - 1);

        float pitch = basePitch;

        if (params_.modDest == ModOsc || params_.modDest == ModBoth)
            pitch += mod * 2.0f;

        const double hz = noteToHz (pitch);
        const double cents1 = (double) drift_[0] * params_.drift * 12.0;
        const double cents2 = (double) drift_[1] * params_.drift * 12.0;

        const double hz1 = hz * kFootage[range1]
                           * std::pow (2.0, cents1 / 1200.0);
        const double hz2 = hz * kFootage[range2]
                           * std::pow (2.0, ((double) params_.oscTune[1] + cents2 / 100.0) / 12.0);

        osc_[0].setFrequency (hz1, sr_);
        osc_[1].setFrequency (hz2, sr_);

        const float o1 = osc_[0].next (wave1);
        const float o2 = osc_[1].next (wave2);

        // ---- mixer and overload -------------------------------------------
        float mix = 0.0f;

        if (params_.oscOn[0])
            mix += o1 * levelSm_[0];

        if (params_.oscOn[1])
            mix += o2 * levelSm_[1];

        if (params_.oscOn[2])
            mix += o3 * levelSm_[2];

        mix += noise * noiseSm_;

        const float driveGain = 1.0f + driveSm_ * 3.0f;
        mix = softClip (mix * driveGain);

        // a tiny noise floor: seeds self-oscillation like the hardware
        mix += nextRandomBipolar() * 2.0e-6f;

        // ---- ladder low-pass ----------------------------------------------
        const float k = emphasisSm_ * 4.35f;

        const int kbdTrack = std::clamp (params_.kbdTrack, 0, NumKbdTracks - 1);
        const float kbdOctaves = kKbdAmount[kbdTrack] * (currentNoteF_ - 60.0f) / 12.0f;
        const float velOctaves = params_.velToFilter * (velocity_ - 0.5f) * 2.0f;
        const float contourOctaves = params_.contour * fenvLevel * 7.0f;
        const float modOctaves = (params_.modDest == ModFilter || params_.modDest == ModBoth)
                                     ? mod * 3.5f
                                     : 0.0f;
        const float driftOctaves = drift_[3] * params_.drift * 0.12f;

        const float cutoff = cutoffSm_
                             * std::exp2 (kbdOctaves + velOctaves + contourOctaves
                                          + modOctaves + driftOctaves);

        ladder_.setCutoff (cutoff);

        float filtered = ladder_.process (mix * (1.0f + 0.35f * k), k,
                                          1.0f + driveSm_ * 1.5f);
        filtered = dcBlock (filtered);

        // ---- loudness ------------------------------------------------------
        const float velGain = 1.0f - params_.velToAmp * (1.0f - velocity_);
        float y = filtered * ampLevel * velGain * volumeSm_;

        // gentle output stage
        y = std::tanh (y * 1.2f) / 1.2f;

        output[i] = y;
    }
}
} // namespace ladderine
