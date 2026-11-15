#include "Parameters.h"

namespace ladderine
{
juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout()
{
    using namespace params;

    std::vector<std::unique_ptr<juce::RangedAudioParameter>> list;

    auto info = [] (const char* label)
    {
        return juce::AudioParameterFloatAttributes().withLabel (label);
    };

    auto addFloat = [&list] (const char* id, const char* name,
                             juce::NormalisableRange<float> range, float def,
                             juce::AudioParameterFloatAttributes attributes = {})
    {
        list.push_back (std::make_unique<juce::AudioParameterFloat> (
            juce::ParameterID { id, 1 }, name, range, def, attributes));
    };

    auto addChoice = [&list] (const char* id, const char* name,
                              const juce::StringArray& choices, int def)
    {
        list.push_back (std::make_unique<juce::AudioParameterChoice> (
            juce::ParameterID { id, 1 }, name, choices, def));
    };

    auto addBool = [&list] (const char* id, const char* name, bool def)
    {
        list.push_back (std::make_unique<juce::AudioParameterBool> (
            juce::ParameterID { id, 1 }, name, def));
    };

    auto percent = [] (float lo, float hi, float def)
    {
        juce::ignoreUnused (def);
        return juce::NormalisableRange<float> (lo, hi, 0.0f);
    };

    auto timeRange = []
    {
        juce::NormalisableRange<float> r (0.001f, 10.0f, 0.0f);
        r.setSkewForCentre (0.25f);
        return r;
    };

    // ------------------------------------------------------------ oscillators
    addChoice (osc1Wave, "Osc 1 Wave", waveNames, 2);
    addChoice (osc1Range, "Osc 1 Range", rangeNames, 2);
    addFloat (osc1Level, "Osc 1 Level", percent (0.0f, 10.0f, 8.0f), 8.0f);
    addBool (osc1On, "Osc 1 On", true);

    addChoice (osc2Wave, "Osc 2 Wave", waveNames, 2);
    addChoice (osc2Range, "Osc 2 Range", rangeNames, 2);
    addFloat (osc2Tune, "Osc 2 Frequency", juce::NormalisableRange<float> (-7.0f, 7.0f, 0.0f), 0.0f, info ("st"));
    addFloat (osc2Level, "Osc 2 Level", percent (0.0f, 10.0f, 6.0f), 6.0f);
    addBool (osc2On, "Osc 2 On", true);

    addChoice (osc3Wave, "Osc 3 Wave", waveNames, 0);
    addChoice (osc3Range, "Osc 3 Range", osc3Ranges, 0);
    addFloat (osc3Tune, "Osc 3 Frequency", juce::NormalisableRange<float> (-7.0f, 7.0f, 0.0f), 0.0f, info ("st"));
    addFloat (osc3Level, "Osc 3 Level", percent (0.0f, 10.0f, 0.0f), 0.0f);
    addBool (osc3On, "Osc 3 On", false);
    addBool (osc3Kbd, "Osc 3 Keyboard Control", false);

    addFloat (noiseLevel, "Noise Level", percent (0.0f, 10.0f, 0.0f), 0.0f);
    addFloat (drive, "Mixer Overload", percent (0.0f, 1.0f, 0.15f), 0.15f);

    // ---------------------------------------------------------------- filter
    juce::NormalisableRange<float> cutoff (20.0f, 20000.0f, 0.0f);
    cutoff.setSkewForCentre (700.0f);
    addFloat (filterCutoff, "Cutoff", cutoff, 1200.0f, info ("Hz"));
    addFloat (filterEmphasis, "Emphasis", percent (0.0f, 10.0f, 2.0f), 2.0f);
    addFloat (filterContour, "Contour", percent (0.0f, 10.0f, 4.0f), 4.0f);
    addChoice (filterKbd, "Filter Keyboard Tracking", kbdNames, 1);
    addFloat (filterAttack, "Filter Attack", timeRange(), 0.005f, info ("s"));
    addFloat (filterDecay, "Filter Decay", timeRange(), 0.40f, info ("s"));
    addFloat (filterSustain, "Filter Sustain", percent (0.0f, 1.0f, 0.20f), 0.20f);
    addFloat (filterRelease, "Filter Release", timeRange(), 0.20f, info ("s"));

    // -------------------------------------------------------------- loudness
    addFloat (ampAttack, "Loudness Attack", timeRange(), 0.002f, info ("s"));
    addFloat (ampDecay, "Loudness Decay", timeRange(), 0.50f, info ("s"));
    addFloat (ampSustain, "Loudness Sustain", percent (0.0f, 1.0f, 0.80f), 0.80f);
    addFloat (ampRelease, "Loudness Release", timeRange(), 0.15f, info ("s"));

    // ------------------------------------------------------------ performance
    juce::NormalisableRange<float> glide (0.0f, 2.0f, 0.0f);
    glide.setSkewForCentre (0.3f);
    addFloat (glideTime, "Glide", glide, 0.0f, info ("s"));
    addChoice (glideMode, "Glide Mode", glideNames, 1);

    addFloat (modMix, "Modulation Mix", percent (0.0f, 1.0f, 0.0f), 0.0f);
    addChoice (modDest, "Modulation Destination", destNames, 0);
    addFloat (modDepth, "Modulation Depth", percent (0.0f, 1.0f, 1.0f), 1.0f);

    juce::NormalisableRange<float> volumeRange (-60.0f, 6.0f, 0.0f);
    volumeRange.setSkewForCentre (-12.0f);
    addFloat (params::volume, "Master Volume", volumeRange, -6.0f, info ("dB"));
    addFloat (tune, "Master Tune", juce::NormalisableRange<float> (-1.0f, 1.0f, 0.0f), 0.0f, info ("st"));
    addFloat (drift, "Analog Drift", percent (0.0f, 1.0f, 0.15f), 0.15f);
    addFloat (bendRange, "Pitch Bend Range", juce::NormalisableRange<float> (1.0f, 12.0f, 1.0f), 2.0f, info ("st"));
    addFloat (velAmp, "Velocity to Loudness", percent (0.0f, 1.0f, 0.0f), 0.0f);
    addFloat (velFilter, "Velocity to Filter", percent (0.0f, 1.0f, 0.0f), 0.0f);

    return { list.begin(), list.end() };
}
} // namespace ladderine
