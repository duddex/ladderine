// Copyright (C) 2026 duddex
// SPDX-License-Identifier: AGPL-3.0-or-later

#pragma once

#include <juce_gui_basics/juce_gui_basics.h>

class LadderineAudioProcessor;

namespace ladderine::ui
{
class Scope : public juce::Component
{
public:
    Scope() = default;

    void setProcessor (LadderineAudioProcessor* processor) noexcept { processor_ = processor; }
    void paint (juce::Graphics&) override;

private:
    LadderineAudioProcessor* processor_ = nullptr;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Scope)
};
} // namespace ladderine::ui
