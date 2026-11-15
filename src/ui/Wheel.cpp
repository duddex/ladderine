#include "Wheel.h"

#include "LookAndFeel.h"

#include <cmath>

namespace ladderine::ui
{
void Wheel::init (juce::String label, float minimum, float maximum, bool sprung,
                  std::function<void (float)> onChange)
{
    label_ = std::move (label);
    minimum_ = minimum;
    maximum_ = maximum;
    sprung_ = sprung;
    callback_ = std::move (onChange);
    value_ = sprung_ ? 0.0f : minimum_;
    setMouseCursor (juce::MouseCursor::UpDownResizeCursor);
}

void Wheel::setValue (float value, bool notify)
{
    value = juce::jlimit (minimum_, maximum_, value);

    if (std::abs (value - value_) < 1.0e-6f)
        return;

    value_ = value;
    repaint();

    if (notify && callback_)
        callback_ (value_);
}

void Wheel::setDisplayValue (float value)
{
    if (dragging_)
        return;

    value = juce::jlimit (minimum_, maximum_, value);

    if (std::abs (value - value_) >= 1.0e-6f)
    {
        value_ = value;
        repaint();
    }
}

void Wheel::mouseDown (const juce::MouseEvent& e)
{
    dragging_ = true;
    dragStartY_ = e.y;
    dragStartValue_ = value_;
}

void Wheel::mouseDrag (const juce::MouseEvent& e)
{
    const float span = maximum_ - minimum_;
    const float delta = (float) (dragStartY_ - e.y) * span / 70.0f;
    setValue (dragStartValue_ + delta, true);
}

void Wheel::mouseUp (const juce::MouseEvent&)
{
    dragging_ = false;

    if (sprung_)
        setValue (0.0f, true);
}

void Wheel::paint (juce::Graphics& g)
{
    auto area = getLocalBounds().toFloat();
    auto textArea = area.removeFromBottom (14.0f);
    auto slot = area.reduced (0.0f, 3.0f);

    const float wheelWidth = juce::jmin (26.0f, slot.getWidth());
    juce::Rectangle<float> wheel (wheelWidth, slot.getHeight());
    wheel.setCentre (slot.getCentre());

    // slot
    g.setColour (colour::ink);
    g.fillRoundedRectangle (wheel.expanded (5.0f, 2.0f), 5.0f);
    g.setColour (colour::cream.withAlpha (0.12f));
    g.drawRoundedRectangle (wheel.expanded (5.0f, 2.0f), 5.0f, 1.0f);

    // wheel body
    juce::ColourGradient grad (juce::Colour (0xff3a3a41), wheel.getX(), 0.0f,
                               juce::Colour (0xff101012), wheel.getRight(), 0.0f, false);
    g.setGradientFill (grad);
    g.fillRoundedRectangle (wheel, 7.0f);

    // ridges
    g.setColour (juce::Colour (0x33000000));

    for (float y = wheel.getY() + 7.0f; y < wheel.getBottom() - 3.0f; y += 5.0f)
        g.drawHorizontalLine (juce::roundToInt (y), wheel.getX() + 2.0f, wheel.getRight() - 2.0f);

    // position indicator
    const float norm = (value_ - minimum_) / juce::jmax (1.0e-6f, maximum_ - minimum_);
    const float y = wheel.getBottom() - 6.0f - norm * (wheel.getHeight() - 12.0f);

    g.setColour (colour::cream);
    g.fillRoundedRectangle (juce::Rectangle<float> (wheel.getX() + 2.0f, y - 1.5f,
                                                    wheel.getWidth() - 4.0f, 3.0f),
                            1.5f);

    if (sprung_)
    {
        g.setColour (colour::cream.withAlpha (0.25f));
        g.drawHorizontalLine (juce::roundToInt (wheel.getCentreY() + 1.0f),
                              wheel.getX() + 1.0f, wheel.getRight() - 1.0f);
    }

    g.setColour (colour::creamDim);
    g.setFont (uiFont (8.5f, true));
    g.drawFittedText (label_, textArea.toNearestInt(), juce::Justification::centred, 1);
}
} // namespace ladderine::ui

