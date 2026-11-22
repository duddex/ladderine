// Copyright (C) 2026 duddex
// SPDX-License-Identifier: AGPL-3.0-or-later

#include "SteppedKnob.h"

#include <cmath>

namespace ladderine::ui
{
void SteppedKnob::init (juce::RangedAudioParameter& parameter,
                        juce::StringArray labels,
                        juce::String caption)
{
    parameter_ = &parameter;
    labels_ = std::move (labels);
    caption_ = std::move (caption);

    attachment_ = std::make_unique<juce::ParameterAttachment> (
        parameter,
        [this] (float value)
        {
            index_ = juce::roundToInt (value);
            repaint();
        });

    attachment_->sendInitialUpdate();

    setTooltip (caption_);
    setMouseCursor (juce::MouseCursor::UpDownResizeCursor);
    setWantsKeyboardFocus (false);
}

void SteppedKnob::setIndex (int newIndex, bool notify)
{
    if (labels_.isEmpty())
        return;

    newIndex = juce::jlimit (0, labels_.size() - 1, newIndex);

    if (newIndex == index_)
        return;

    index_ = newIndex;
    repaint();

    if (notify && parameter_ != nullptr && attachment_ != nullptr)
        attachment_->setValueAsCompleteGesture ((float) index_);
}

void SteppedKnob::mouseDown (const juce::MouseEvent& e)
{
    dragStartIndex_ = index_;
    dragStartY_ = e.y;
}

void SteppedKnob::mouseDrag (const juce::MouseEvent& e)
{
    const int steps = (dragStartY_ - e.y) / 12;
    setIndex (dragStartIndex_ + steps, true);
}

void SteppedKnob::mouseWheelMove (const juce::MouseEvent&,
                                  const juce::MouseWheelDetails& wheel)
{
    setIndex (index_ + (wheel.deltaY > 0.0f ? 1 : -1), true);
}

void SteppedKnob::paint (juce::Graphics& g)
{
    auto area = getLocalBounds().toFloat();
    auto textArea = area.removeFromBottom (15.0f);
    const auto square = area.reduced (4.0f);

    const float radius = juce::jmin (square.getWidth(), square.getHeight()) * 0.5f;
    const auto centre = square.getCentre();

    constexpr float startAngle = 3.92699082f;  // 225 degrees (bottom left)
    constexpr float sweep = 4.71238898f;       // 270 degrees

    const int count = juce::jmax (2, labels_.size());

    // position ticks
    for (int i = 0; i < count; ++i)
    {
        const float a = startAngle + sweep * (float) i / (float) (count - 1);
        const float sinA = std::sin (a);
        const float cosA = std::cos (a);

        const bool active = i == index_;

        g.setColour (active ? colour::cream : colour::cream.withAlpha (0.28f));
        g.drawLine (centre.x + sinA * (radius + 1.0f),
                    centre.y - cosA * (radius + 1.0f),
                    centre.x + sinA * (radius + 5.0f),
                    centre.y - cosA * (radius + 5.0f),
                    active ? 1.6f : 1.0f);
    }

    const float bodyRadius = radius * 0.72f;
    const auto body = juce::Rectangle<float> (bodyRadius * 2.0f, bodyRadius * 2.0f)
                          .withCentre (centre);

    g.setColour (colour::ink.withAlpha (0.5f));
    g.fillEllipse (body.translated (0.0f, 1.5f));

    juce::ColourGradient gradient (juce::Colour (0xff37373e), centre.x, centre.y - bodyRadius,
                                   juce::Colour (0xff121214), centre.x, centre.y + bodyRadius,
                                   false);
    g.setGradientFill (gradient);
    g.fillEllipse (body);

    g.setColour (colour::cream.withAlpha (0.22f));
    g.drawEllipse (body, 1.0f);

    const float angle = startAngle
                        + sweep * (float) index_ / (float) juce::jmax (1, count - 1);

    const float sinA = std::sin (angle);
    const float cosA = std::cos (angle);

    g.setColour (colour::cream);
    g.drawLine (centre.x + sinA * bodyRadius * 0.25f,
                centre.y - cosA * bodyRadius * 0.25f,
                centre.x + sinA * bodyRadius * 0.86f,
                centre.y - cosA * bodyRadius * 0.86f,
                2.0f);

    g.setColour (juce::Colour (0xff26262c));
    g.fillEllipse (juce::Rectangle<float> (bodyRadius * 0.34f, bodyRadius * 0.34f)
                       .withCentre (centre));

    const auto text = index_ >= 0 && index_ < labels_.size() ? labels_[index_] : juce::String();

    g.setColour (colour::cream);
    g.setFont (uiFont (8.5f, true));
    g.drawFittedText (text, textArea.toNearestInt(), juce::Justification::centred, 1);
}
} // namespace ladderine::ui

