// Copyright (C) 2026 duddex
// SPDX-License-Identifier: AGPL-3.0-or-later

/*
    Ladderine - parameter identifiers and layout.

    The IDs are stable: presets, host automation and saved sessions depend
    on them.
*/
#pragma once

#include <juce_audio_processors/juce_audio_processors.h>

namespace ladderine
{
namespace params
{
inline constexpr const char* osc1Wave   = "osc1_wave";
inline constexpr const char* osc1Range  = "osc1_range";
inline constexpr const char* osc1Level  = "osc1_level";
inline constexpr const char* osc1On     = "osc1_on";

inline constexpr const char* osc2Wave   = "osc2_wave";
inline constexpr const char* osc2Range  = "osc2_range";
inline constexpr const char* osc2Tune   = "osc2_tune";
inline constexpr const char* osc2Level  = "osc2_level";
inline constexpr const char* osc2On     = "osc2_on";

inline constexpr const char* osc3Wave   = "osc3_wave";
inline constexpr const char* osc3Range  = "osc3_range";
inline constexpr const char* osc3Tune   = "osc3_tune";
inline constexpr const char* osc3Level  = "osc3_level";
inline constexpr const char* osc3On     = "osc3_on";
inline constexpr const char* osc3Kbd    = "osc3_kbd";

inline constexpr const char* noiseLevel = "noise_level";
inline constexpr const char* drive      = "mixer_drive";

inline constexpr const char* filterCutoff   = "filter_cutoff";
inline constexpr const char* filterEmphasis = "filter_emphasis";
inline constexpr const char* filterContour  = "filter_contour";
inline constexpr const char* filterKbd      = "filter_kbd";
inline constexpr const char* filterAttack   = "filter_attack";
inline constexpr const char* filterDecay    = "filter_decay";
inline constexpr const char* filterSustain  = "filter_sustain";
inline constexpr const char* filterRelease  = "filter_release";

inline constexpr const char* ampAttack  = "amp_attack";
inline constexpr const char* ampDecay   = "amp_decay";
inline constexpr const char* ampSustain = "amp_sustain";
inline constexpr const char* ampRelease = "amp_release";

inline constexpr const char* glideTime = "glide_time";
inline constexpr const char* glideMode = "glide_mode";

inline constexpr const char* modMix   = "mod_mix";
inline constexpr const char* modDest  = "mod_dest";
inline constexpr const char* modDepth = "mod_depth";

inline constexpr const char* volume    = "master_volume";
inline constexpr const char* tune      = "master_tune";
inline constexpr const char* drift     = "drift";
inline constexpr const char* bendRange = "bend_range";
inline constexpr const char* velAmp    = "vel_amp";
inline constexpr const char* velFilter = "vel_filter";

inline const juce::StringArray waveNames   { "TRI", "TRI-SAW", "SAW", "SQUARE", "PULSE", "NARROW" };
inline const juce::StringArray rangeNames  { "32'", "16'", "8'", "4'", "2'" };
inline const juce::StringArray osc3Ranges  { "LO", "32'", "16'", "8'", "4'", "2'" };
inline const juce::StringArray kbdNames    { "OFF", "1/3", "2/3", "FULL" };
inline const juce::StringArray glideNames  { "OFF", "ON", "AUTO" };
inline const juce::StringArray destNames   { "FILTER", "OSC", "BOTH" };
} // namespace params

juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();
} // namespace ladderine
