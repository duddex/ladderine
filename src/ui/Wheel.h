// Copyright (C) 2026 duddex
// SPDX-License-Identifier: AGPL-3.0-or-later

#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include <functional>

namespace ladderine::ui
{
/* A performance wheel: modulation (stays where it is left) or pitch bend
   (springs back to centre). */
class Wheel : public juce::Component
{
public:
    Wheel() = default;
    void init (juce::String label, float minimum, float maximum, bool sprung,
               std::function<void (float)> onChange);

    void paint (juce::Graphics&) override;
    void mouseDown (const juce::MouseEvent&) override;
    void mouseDrag (const juce::MouseEvent&) override;
    void mouseUp (const juce::MouseEvent&) override;

    void setDisplayValue (float value);
    float getValue() const noexcept { return value_; }

private:
    void setValue (float value, bool notify);

    juce::String label_;
    float minimum_ = 0.0f;
    float maximum_ = 1.0f;
    float value_ = 0.0f;
    bool sprung_ = false;
    bool dragging_ = false;
    float dragStartValue_ = 0.0f;
    int dragStartY_ = 0;
    std::function<void (float)> callback_;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Wheel)
};
} // namespace ladderine::ui
