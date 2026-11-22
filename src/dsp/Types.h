// Copyright (C) 2026 duddex
// SPDX-License-Identifier: AGPL-3.0-or-later

/*
    Ladderine - a Model D inspired monophonic synthesizer.
    DSP core types. Deliberately free of JUCE: the same code builds as a
    console test and inside the plugin.
*/
#pragma once

#include <algorithm>
#include <cmath>
#include <cstdint>

namespace ladderine
{
enum Wave
{
    WaveTri = 0,
    WaveTriSaw,
    WaveSaw,
    WaveSquare,
    WavePulse,
    WaveNarrow,
    NumWaves
};

enum Range
{
    Range32 = 0,
    Range16,
    Range8,
    Range4,
    Range2,
    NumRanges
};

// Oscillator 3 adds the LO position in front of the footage settings.
enum Osc3Range
{
    Osc3Lo = 0,
    Osc3Range32,
    Osc3Range16,
    Osc3Range8,
    Osc3Range4,
    Osc3Range2,
    NumOsc3Ranges
};

enum GlideMode
{
    GlideOff = 0,
    GlideOn,
    GlideAuto,
    NumGlideModes
};

enum ModDest
{
    ModFilter = 0,
    ModOsc,
    ModBoth,
    NumModDests
};

enum KbdTrack
{
    KbdTrackOff = 0,
    KbdTrackThird,
    KbdTrackTwoThirds,
    KbdTrackFull,
    NumKbdTracks
};

struct EnvParams
{
    float attack = 0.01f;
    float decay = 0.3f;
    float sustain = 0.75f;
    float release = 0.25f;
};

struct EngineParams
{
    bool  oscOn[3]    { true, true, false };
    int   oscWave[3]  { WaveSaw, WaveSaw, WaveTri };
    int   oscRange[3] { Range8, Range8, Osc3Lo }; // osc 3 uses Osc3Range
    float oscLevel[3] { 0.80f, 0.60f, 0.0f };     // 0..1
    float oscTune[3]  { 0.0f, 0.0f, 0.0f };       // semitones (osc 2 / 3)
    bool  osc3Kbd = false;

    float noiseLevel = 0.0f;                       // 0..1
    float drive = 0.20f;                           // 0..1 mixer overload

    float cutoffHz = 1200.0f;
    float emphasis = 0.20f;                        // 0..1 (0..10 on the panel)
    float contour = 0.40f;                         // 0..1
    int   kbdTrack = KbdTrackThird;
    EnvParams filterEnv { 0.005f, 0.40f, 0.20f, 0.20f };

    EnvParams ampEnv { 0.002f, 0.50f, 0.80f, 0.15f };

    float glideSeconds = 0.0f;
    int   glideMode = GlideOn;

    float modMix = 0.0f;                           // 0 = osc 3, 1 = noise
    int   modDest = ModFilter;
    float modDepth = 1.0f;
    float modWheel = 0.0f;                         // 0..1, smoothed by the engine

    float volumeDb = -6.0f;
    float tuneSemis = 0.0f;                        // -1..+1
    float drift = 0.15f;                           // 0..1 analog instability
    int   bendRange = 2;                           // semitones, -bendRange..+bendRange
    float velToAmp = 0.0f;
    float velToFilter = 0.0f;
};

inline float clampf (float v, float lo, float hi)
{
    return v < lo ? lo : (v > hi ? hi : v);
}

inline float lerpf (float a, float b, float t)
{
    return a + (b - a) * t;
}

inline double noteToHz (double midiNote)
{
    return 440.0 * std::pow (2.0, (midiNote - 69.0) / 12.0);
}

inline float dbToGain (float db)
{
    return db <= -60.0f ? 0.0f : std::pow (10.0f, db * 0.05f);
}
} // namespace ladderine
