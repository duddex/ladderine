// Copyright (C) 2026 duddex
// SPDX-License-Identifier: AGPL-3.0-or-later

#pragma once

#include <juce_gui_basics/juce_gui_basics.h>

namespace ladderine::ui
{
namespace colour
{
inline const juce::Colour panel      { 0xff141418 };
inline const juce::Colour panelLight { 0xff1c1c21 };
inline const juce::Colour panelEdge  { 0xff2d2d34 };
inline const juce::Colour ink        { 0xff0a0a0c };
inline const juce::Colour cream      { 0xffe8ddbe };
inline const juce::Colour creamDim   { 0xffb4a98a };
inline const juce::Colour creamFaint { 0x66e8ddbe };
inline const juce::Colour red        { 0xffcf4b35 };
inline const juce::Colour gold       { 0xffd7a648 };
inline const juce::Colour wood       { 0xff5a3820 };
inline const juce::Colour woodDark   { 0xff2b1a0e };
inline const juce::Colour screen     { 0xff0c1310 };
} // namespace colour

juce::Font uiFont (float height, bool bold = false);
juce::Font logoFont (float height);

class MonoLookAndFeel : public juce::LookAndFeel_V4
{
public:
    MonoLookAndFeel() = default;
    ~MonoLookAndFeel() override = default;

    void drawRotarySlider (juce::Graphics&, int x, int y, int width, int height,
                           float sliderPos, float rotaryStartAngle, float rotaryEndAngle,
                           juce::Slider&) override;

    void drawComboBox (juce::Graphics&, int width, int height, bool isButtonDown,
                       int buttonX, int buttonY, int buttonW, int buttonH,
                       juce::ComboBox&) override;
    void positionComboBoxText (juce::ComboBox&, juce::Label&) override;

    void drawButtonBackground (juce::Graphics&, juce::Button&, const juce::Colour&,
                               bool shouldDrawButtonAsHighlighted,
                               bool shouldDrawButtonAsDown) override;
    void drawButtonText (juce::Graphics&, juce::TextButton&,
                         bool shouldDrawButtonAsHighlighted,
                         bool shouldDrawButtonAsDown) override;

    void drawToggleButton (juce::Graphics&, juce::ToggleButton&,
                           bool shouldDrawButtonAsHighlighted,
                           bool shouldDrawButtonAsDown) override;
};
} // namespace ladderine::ui
