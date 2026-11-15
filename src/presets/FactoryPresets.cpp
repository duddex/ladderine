#include "FactoryPresets.h"

#include "../Parameters.h"

namespace ladderine
{
namespace
{
using namespace params;

#define P(id, value) PresetValue { id, value }

std::vector<FactoryPreset> build()
{
    std::vector<FactoryPreset> presets;

    presets.push_back ({ "Init", {} });

    presets.push_back ({ "Fat Bass", {
        P (osc1Wave, 2), P (osc1Range, 2), P (osc1Level, 9.0f), P (osc1On, 1),
        P (osc2Wave, 2), P (osc2Range, 1), P (osc2Tune, 0.12f), P (osc2Level, 7.5f), P (osc2On, 1),
        P (osc3On, 0),
        P (noiseLevel, 0), P (drive, 0.35f),
        P (filterCutoff, 380), P (filterEmphasis, 4.2f), P (filterContour, 6.5f), P (filterKbd, 1),
        P (filterAttack, 0.002f), P (filterDecay, 0.28f), P (filterSustain, 0.05f), P (filterRelease, 0.12f),
        P (ampAttack, 0.002f), P (ampDecay, 0.60f), P (ampSustain, 0.75f), P (ampRelease, 0.14f),
        P (glideTime, 0.03f), P (glideMode, 1),
        P (modMix, 0.0f), P (modDest, 0), P (modDepth, 1.0f),
        P (volume, -5.0f), P (drift, 0.20f), P (bendRange, 2), P (velAmp, 0), P (velFilter, 0) } });

    presets.push_back ({ "Rubber Bass", {
        P (osc1Wave, 3), P (osc1Range, 1), P (osc1Level, 8.5f), P (osc1On, 1),
        P (osc2Wave, 2), P (osc2Range, 2), P (osc2Tune, -4.5f), P (osc2Level, 5.0f), P (osc2On, 1),
        P (osc3On, 0),
        P (drive, 0.45f),
        P (filterCutoff, 240), P (filterEmphasis, 6.0f), P (filterContour, 7.5f), P (filterKbd, 0),
        P (filterAttack, 0.001f), P (filterDecay, 0.16f), P (filterSustain, 0.0f), P (filterRelease, 0.10f),
        P (ampAttack, 0.002f), P (ampDecay, 0.35f), P (ampSustain, 0.55f), P (ampRelease, 0.08f),
        P (glideTime, 0.02f), P (glideMode, 1),
        P (volume, -5.0f), P (drift, 0.15f) } });

    presets.push_back ({ "Screaming Lead", {
        P (osc1Wave, 2), P (osc1Range, 2), P (osc1Level, 8.0f), P (osc1On, 1),
        P (osc2Wave, 2), P (osc2Range, 2), P (osc2Tune, 0.20f), P (osc2Level, 6.5f), P (osc2On, 1),
        P (osc3Wave, 2), P (osc3Range, 4), P (osc3Level, 4.0f), P (osc3On, 1), P (osc3Kbd, 1),
        P (drive, 0.25f),
        P (filterCutoff, 2400), P (filterEmphasis, 5.0f), P (filterContour, 4.0f), P (filterKbd, 2),
        P (filterAttack, 0.010f), P (filterDecay, 0.50f), P (filterSustain, 0.30f), P (filterRelease, 0.30f),
        P (ampAttack, 0.005f), P (ampDecay, 0.40f), P (ampSustain, 0.85f), P (ampRelease, 0.25f),
        P (glideTime, 0.08f), P (glideMode, 1),
        P (volume, -8.0f), P (drift, 0.25f), P (bendRange, 2) } });

    presets.push_back ({ "Emerson Lead", {
        P (osc1Wave, 2), P (osc1Range, 2), P (osc1Level, 9.0f), P (osc1On, 1),
        P (osc2Wave, 3), P (osc2Range, 2), P (osc2Tune, 0.05f), P (osc2Level, 7.0f), P (osc2On, 1),
        P (osc3Wave, 2), P (osc3Range, 2), P (osc3Level, 3.0f), P (osc3On, 1), P (osc3Kbd, 1),
        P (drive, 0.55f),
        P (filterCutoff, 1600), P (filterEmphasis, 5.5f), P (filterContour, 5.0f), P (filterKbd, 2),
        P (filterAttack, 0.010f), P (filterDecay, 0.30f), P (filterSustain, 0.40f), P (filterRelease, 0.20f),
        P (ampAttack, 0.005f), P (ampDecay, 0.30f), P (ampSustain, 0.85f), P (ampRelease, 0.20f),
        P (glideTime, 0.04f), P (glideMode, 1),
        P (volume, -7.0f), P (drift, 0.30f) } });

    presets.push_back ({ "Brass Section", {
        P (osc1Wave, 2), P (osc1Range, 2), P (osc1Level, 8.0f), P (osc1On, 1),
        P (osc2Wave, 2), P (osc2Range, 2), P (osc2Tune, 0.10f), P (osc2Level, 6.5f), P (osc2On, 1),
        P (osc3Wave, 2), P (osc3Range, 2), P (osc3Level, 5.0f), P (osc3On, 1), P (osc3Kbd, 1),
        P (drive, 0.20f),
        P (filterCutoff, 900), P (filterEmphasis, 2.5f), P (filterContour, 6.0f), P (filterKbd, 1),
        P (filterAttack, 0.060f), P (filterDecay, 0.60f), P (filterSustain, 0.35f), P (filterRelease, 0.30f),
        P (ampAttack, 0.040f), P (ampDecay, 0.40f), P (ampSustain, 0.90f), P (ampRelease, 0.25f),
        P (glideTime, 0.0f), P (glideMode, 0),
        P (volume, -8.0f), P (drift, 0.20f), P (velAmp, 0.25f), P (velFilter, 0.35f) } });

    presets.push_back ({ "Lucky Man", {
        P (osc1Wave, 0), P (osc1Range, 2), P (osc1Level, 8.0f), P (osc1On, 1),
        P (osc2Wave, 2), P (osc2Range, 2), P (osc2Tune, 0.15f), P (osc2Level, 6.0f), P (osc2On, 1),
        P (osc3Wave, 2), P (osc3Range, 3), P (osc3Level, 2.0f), P (osc3On, 1), P (osc3Kbd, 1),
        P (drive, 0.15f),
        P (filterCutoff, 1400), P (filterEmphasis, 3.0f), P (filterContour, 3.5f), P (filterKbd, 1),
        P (filterAttack, 0.010f), P (filterDecay, 0.50f), P (filterSustain, 0.40f), P (filterRelease, 0.30f),
        P (ampAttack, 0.010f), P (ampDecay, 0.50f), P (ampSustain, 0.90f), P (ampRelease, 0.35f),
        P (glideTime, 0.12f), P (glideMode, 1),
        P (modMix, 0.10f), P (modDest, 1), P (modDepth, 0.8f),
        P (volume, -8.0f), P (drift, 0.25f), P (velAmp, 0.1f) } });

    presets.push_back ({ "Whistle", {
        P (osc1Wave, 0), P (osc1Range, 3), P (osc1Level, 8.0f), P (osc1On, 1),
        P (osc2Wave, 0), P (osc2Range, 2), P (osc2Tune, 0.25f), P (osc2Level, 5.0f), P (osc2On, 1),
        P (osc3On, 0),
        P (filterCutoff, 3000), P (filterEmphasis, 4.5f), P (filterContour, 2.0f), P (filterKbd, 3),
        P (filterAttack, 0.020f), P (filterDecay, 0.60f), P (filterSustain, 0.50f), P (filterRelease, 0.40f),
        P (ampAttack, 0.080f), P (ampDecay, 0.50f), P (ampSustain, 0.90f), P (ampRelease, 0.40f),
        P (glideTime, 0.25f), P (glideMode, 1),
        P (volume, -9.0f), P (drift, 0.30f), P (velAmp, 0.2f), P (bendRange, 2) } });

    presets.push_back ({ "Filter Sweep", {
        P (osc1Wave, 2), P (osc1Range, 2), P (osc1Level, 8.0f), P (osc1On, 1),
        P (osc2Wave, 2), P (osc2Range, 1), P (osc2Level, 6.0f), P (osc2On, 1),
        P (osc3On, 0),
        P (filterCutoff, 180), P (filterEmphasis, 6.5f), P (filterContour, 9.0f), P (filterKbd, 0),
        P (filterAttack, 1.5f), P (filterDecay, 3.0f), P (filterSustain, 0.25f), P (filterRelease, 1.2f),
        P (ampAttack, 0.40f), P (ampDecay, 1.0f), P (ampSustain, 1.0f), P (ampRelease, 1.5f),
        P (glideTime, 0.0f), P (glideMode, 0),
        P (volume, -8.0f), P (drift, 0.2f) } });

    presets.push_back ({ "Noise Sweep", {
        P (osc1On, 0), P (osc2On, 0), P (osc3On, 0),
        P (noiseLevel, 8.0f),
        P (filterCutoff, 300), P (filterEmphasis, 7.0f), P (filterContour, 9.0f), P (filterKbd, 0),
        P (filterAttack, 2.0f), P (filterDecay, 4.0f), P (filterSustain, 0.10f), P (filterRelease, 1.5f),
        P (ampAttack, 0.020f), P (ampDecay, 1.0f), P (ampSustain, 1.0f), P (ampRelease, 1.2f),
        P (volume, -8.0f), P (drift, 0.0f) } });

    presets.push_back ({ "Percussive Bass", {
        P (osc1Wave, 3), P (osc1Range, 1), P (osc1Level, 9.0f), P (osc1On, 1),
        P (osc2Wave, 2), P (osc2Range, 2), P (osc2Tune, -0.10f), P (osc2Level, 5.0f), P (osc2On, 1),
        P (osc3On, 0),
        P (drive, 0.30f),
        P (filterCutoff, 320), P (filterEmphasis, 5.5f), P (filterContour, 8.0f), P (filterKbd, 0),
        P (filterAttack, 0.001f), P (filterDecay, 0.12f), P (filterSustain, 0.0f), P (filterRelease, 0.08f),
        P (ampAttack, 0.001f), P (ampDecay, 0.28f), P (ampSustain, 0.35f), P (ampRelease, 0.06f),
        P (glideTime, 0.0f), P (glideMode, 0),
        P (volume, -5.0f), P (drift, 0.1f) } });

    presets.push_back ({ "Resonant Drone", {
        P (osc1Wave, 2), P (osc1Range, 2), P (osc1Level, 6.0f), P (osc1On, 1),
        P (osc2Wave, 0), P (osc2Range, 1), P (osc2Level, 5.0f), P (osc2On, 1),
        P (osc3Wave, 0), P (osc3Range, 0), P (osc3Level, 0.0f), P (osc3On, 0),
        P (filterCutoff, 600), P (filterEmphasis, 8.0f), P (filterContour, 6.0f), P (filterKbd, 1),
        P (filterAttack, 1.2f), P (filterDecay, 2.0f), P (filterSustain, 0.60f), P (filterRelease, 2.0f),
        P (ampAttack, 1.0f), P (ampDecay, 2.0f), P (ampSustain, 0.80f), P (ampRelease, 2.0f),
        P (glideTime, 0.30f), P (glideMode, 1),
        P (modMix, 0.0f), P (modDest, 0), P (modDepth, 1.0f),
        P (volume, -9.0f), P (drift, 0.5f) } });

    return presets;
}

#undef P
} // namespace

const std::vector<FactoryPreset>& factoryPresets()
{
    static const std::vector<FactoryPreset> presets = build();
    return presets;
}
} // namespace ladderine
