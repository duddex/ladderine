// Copyright (C) 2026 duddex
// SPDX-License-Identifier: AGPL-3.0-or-later

#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_gui_basics/juce_gui_basics.h>

#include <memory>

#include "ui/LookAndFeel.h"

class LadderineAudioProcessor;
class RootComponent;

class LadderineAudioProcessorEditor : public juce::AudioProcessorEditor
{
public:
    explicit LadderineAudioProcessorEditor (LadderineAudioProcessor&);
    ~LadderineAudioProcessorEditor() override;

    void paint (juce::Graphics&) override;
    void resized() override;

private:
    LadderineAudioProcessor& processor_;
    ladderine::ui::MonoLookAndFeel lookAndFeel_;
    std::unique_ptr<RootComponent> root_;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (LadderineAudioProcessorEditor)
};
