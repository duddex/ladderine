// Copyright (C) 2026 duddex
// SPDX-License-Identifier: AGPL-3.0-or-later

#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_gui_basics/juce_gui_basics.h>

#include "ui/LookAndFeel.h"

namespace ladderine::ui
{
/* A rotary control with the engraved caption underneath. */
class Knob : public juce::Component
{
public:
    Knob() = default;

    void init (juce::AudioProcessorValueTreeState& state,
               const juce::String& parameterID,
               const juce::String& caption)
    {
        slider_.setSliderStyle (juce::Slider::RotaryVerticalDrag);
        slider_.setTextBoxStyle (juce::Slider::NoTextBox, false, 0, 0);
        slider_.setRotaryParameters (juce::MathConstants<float>::pi * 1.2f,
                                     juce::MathConstants<float>::pi * 2.8f,
                                     true);
        slider_.setTooltip (caption);
        slider_.setWantsKeyboardFocus (false);
        addAndMakeVisible (slider_);

        label_.setText (caption, juce::dontSendNotification);
        label_.setJustificationType (juce::Justification::centred);
        label_.setColour (juce::Label::textColourId, colour::creamDim);
        label_.setFont (uiFont (8.5f, true));
        label_.setInterceptsMouseClicks (false, false);
        addAndMakeVisible (label_);

        attachment_ = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (
            state, parameterID, slider_);

        slider_.onValueChange = [this]
        {
            slider_.setTooltip (label_.getText() + ": "
                                + slider_.getTextFromValue (slider_.getValue()));
        };

        slider_.onValueChange();
    }

    juce::Slider& slider() noexcept { return slider_; }
    juce::Label& captionLabel() noexcept { return label_; }

    void resized() override
    {
        auto area = getLocalBounds();
        label_.setBounds (area.removeFromBottom (13));
        slider_.setBounds (area);
    }

private:
    juce::Slider slider_;
    juce::Label label_;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> attachment_;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Knob)
};
} // namespace ladderine::ui
