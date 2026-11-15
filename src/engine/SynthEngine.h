/*
    Ladderine - the voice.

    One monophonic, three-oscillator voice in the spirit of the Model D:
    three free-running oscillators, a noise source, a mixer that can be
    overloaded, a 4-pole ladder low-pass with its own contour, a loudness
    contour, glide and the oscillator-3 / noise modulation bus.

    No allocation, no locking, no JUCE.
*/
#pragma once

#include "../dsp/Types.h"
#include "../dsp/Oscillator.h"
#include "../dsp/Envelope.h"
#include "../dsp/LadderFilter.h"

namespace ladderine
{
class SynthEngine
{
public:
    static constexpr int kMaxHeldNotes = 16;

    SynthEngine();

    void prepare (double sampleRate);
    void reset();

    void setParams (const EngineParams& p);
    const EngineParams& params() const noexcept { return params_; }

    void noteOn (int midiNote, float velocity);
    void noteOff (int midiNote);
    void allNotesOff (bool immediate = false);
    void sustainPedal (bool down);

    // Performance controls, -1..+1 and 0..1. Smoothed internally.
    void pitchBend (float value);
    void modWheel (float value);

    void render (float* output, int numSamples);

    bool isActive() const noexcept;
    int currentNote() const noexcept { return currentNote_; }
    float loudnessLevel() const noexcept { return ampEnv_.value(); }
    float filterEnvelopeLevel() const noexcept { return filterEnv_.value(); }
    float modulationValue() const noexcept { return lastMod_; }

private:
    void updateSmoothing();
    void updateDrift();
    float nextRandomBipolar();
    float dcBlock (float x);
    void releaseIfPossible();
    void removeHeldNote (int note);

    double sr_ = 48000.0;

    EngineParams params_;

    Oscillator osc_[3];
    Envelope ampEnv_;
    Envelope filterEnv_;
    LadderFilter ladder_;

    // note stack, last-note priority
    int heldNotes_[kMaxHeldNotes] {};
    int numHeld_ = 0;
    int currentNote_ = -1;
    float currentNoteF_ = 60.0f;
    float targetNoteF_ = 60.0f;
    float velocity_ = 1.0f;
    bool sustain_ = false;
    bool playedSinceReset_ = false;

    float bendSm_ = 0.0f;
    float bendTarget_ = 0.0f;
    float modWheelSm_ = 0.0f;
    float cutoffSm_ = 1200.0f;
    float emphasisSm_ = 0.2f;
    float volumeSm_ = 0.5f;
    float tuneSm_ = 0.0f;
    float levelSm_[3] { 0.8f, 0.6f, 0.0f };
    float noiseSm_ = 0.0f;
    float driveSm_ = 0.2f;

    float glideCoef_ = 1.0f;
    float driftCoef_ = 0.0001f;
    float dcR_ = 0.9993f;

    // slow random drift, one generator per oscillator plus the filter
    float drift_[4] {};
    float driftTarget_[4] {};
    int driftCountdown_ = 0;

    uint32_t rng_ = 0x2545f491u;

    float dcX1_ = 0.0f;
    float dcY1_ = 0.0f;

    float lastMod_ = 0.0f;
};
} // namespace ladderine
