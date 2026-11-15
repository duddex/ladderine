#pragma once

#include <juce_core/juce_core.h>
#include <vector>

namespace ladderine
{
struct PresetValue
{
    const char* id;
    float value;
};

struct FactoryPreset
{
    juce::String name;
    std::vector<PresetValue> values;
};

const std::vector<FactoryPreset>& factoryPresets();
} // namespace ladderine
