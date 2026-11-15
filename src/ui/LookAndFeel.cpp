#include "LookAndFeel.h"

#include <cmath>

namespace ladderine::ui
{
juce::Font uiFont (float height, bool bold)
{
    return juce::Font (juce::FontOptions (height, bold ? juce::Font::bold : juce::Font::plain))
        .withExtraKerningFactor (0.02f);
}

juce::Font logoFont (float height)
{
    return juce::Font (juce::FontOptions (juce::Font::getDefaultSerifFontName(), height,
                                          juce::Font::bold))
        .withExtraKerningFactor (0.12f);
}

void MonoLookAndFeel::drawRotarySlider (juce::Graphics& g, int x, int y, int width, int height,
                                        float sliderPos, float rotaryStartAngle,
                                        float rotaryEndAngle, juce::Slider&)
{
    auto bounds = juce::Rectangle<int> (x, y, width, height).toFloat().reduced (3.0f);
    const float radius = juce::jmin (bounds.getWidth(), bounds.getHeight()) * 0.5f;
    const auto centre = bounds.getCentre();
    const float angle = rotaryStartAngle + sliderPos * (rotaryEndAngle - rotaryStartAngle);

    // engraved tick ring
    g.setColour (colour::cream.withAlpha (0.25f));

    for (int i = 0; i <= 10; ++i)
    {
        const float a = rotaryStartAngle + (rotaryEndAngle - rotaryStartAngle) * (float) i / 10.0f;
        const juce::Point<float> p1 (centre.x + std::sin (a) * (radius + 1.5f),
                                     centre.y - std::cos (a) * (radius + 1.5f));
        const juce::Point<float> p2 (centre.x + std::sin (a) * (radius + 5.0f),
                                     centre.y - std::cos (a) * (radius + 5.0f));
        g.drawLine ({ p1, p2 }, 1.0f);
    }

    // shadow
    g.setColour (colour::ink.withAlpha (0.55f));
    g.fillEllipse (juce::Rectangle<float> (radius * 2.0f, radius * 2.0f)
                       .withCentre (centre.translated (0.0f, 2.0f)));

    // body
    const auto body = juce::Rectangle<float> (radius * 2.0f, radius * 2.0f).withCentre (centre);

    juce::ColourGradient gradient (juce::Colour (0xff3b3b42), centre.x, centre.y - radius,
                                   juce::Colour (0xff131315), centre.x, centre.y + radius, false);
    g.setGradientFill (gradient);
    g.fillEllipse (body);

    g.setColour (colour::cream.withAlpha (0.22f));
    g.drawEllipse (body, 1.0f);

    // pointer
    const juce::Point<float> root (centre.x + std::sin (angle) * radius * 0.20f,
                                   centre.y - std::cos (angle) * radius * 0.20f);
    const juce::Point<float> tip (centre.x + std::sin (angle) * radius * 0.78f,
                                  centre.y - std::cos (angle) * radius * 0.78f);
    g.setColour (colour::cream);
    g.drawLine ({ root, tip }, 2.4f);

    // centre cap
    g.setColour (juce::Colour (0xff26262c));
    g.fillEllipse (juce::Rectangle<float> (radius * 0.38f, radius * 0.38f).withCentre (centre));
}

void MonoLookAndFeel::drawComboBox (juce::Graphics& g, int width, int height, bool,
                                    int, int, int, int, juce::ComboBox&)
{
    const auto r = juce::Rectangle<float> (0.0f, 0.0f, (float) width, (float) height).reduced (0.5f);

    g.setColour (colour::ink);
    g.fillRoundedRectangle (r, 3.0f);

    g.setColour (colour::cream.withAlpha (0.30f));
    g.drawRoundedRectangle (r, 3.0f, 1.0f);

    // arrow
    juce::Path arrow;
    const float ax = (float) width - 14.0f;
    const float ay = (float) height * 0.5f;
    arrow.startNewSubPath (ax - 4.0f, ay - 2.0f);
    arrow.lineTo (ax, ay + 2.0f);
    arrow.lineTo (ax + 4.0f, ay - 2.0f);
    g.setColour (colour::creamDim);
    g.strokePath (arrow, juce::PathStrokeType (1.4f));
}

void MonoLookAndFeel::positionComboBoxText (juce::ComboBox& box, juce::Label& label)
{
    label.setBounds (8, 0, box.getWidth() - 26, box.getHeight());
    label.setFont (uiFont (11.0f, true));
    label.setColour (juce::Label::textColourId, colour::cream);
    label.setJustificationType (juce::Justification::centredLeft);
}

void MonoLookAndFeel::drawButtonBackground (juce::Graphics& g, juce::Button& button,
                                            const juce::Colour&, bool highlighted, bool down)
{
    const auto r = button.getLocalBounds().toFloat().reduced (0.5f);
    auto base = colour::panelLight;

    if (down)
        base = base.brighter (0.18f);
    else if (highlighted)
        base = base.brighter (0.08f);

    g.setColour (base);
    g.fillRoundedRectangle (r, 3.0f);
    g.setColour (colour::cream.withAlpha (highlighted ? 0.5f : 0.28f));
    g.drawRoundedRectangle (r, 3.0f, 1.0f);
}

void MonoLookAndFeel::drawButtonText (juce::Graphics& g, juce::TextButton& button,
                                      bool, bool)
{
    g.setColour (button.getToggleState() ? colour::ink : colour::cream);
    g.setFont (uiFont (10.0f, true));
    g.drawFittedText (button.getButtonText(), button.getLocalBounds().reduced (4, 0),
                      juce::Justification::centred, 1);
}

void MonoLookAndFeel::drawToggleButton (juce::Graphics& g, juce::ToggleButton& button,
                                        bool highlighted, bool down)
{
    const bool on = button.getToggleState();
    auto r = button.getLocalBounds().toFloat().reduced (0.5f);

    juce::Colour fill = on ? colour::cream : colour::panelLight;

    if (down)
        fill = fill.brighter (0.12f);
    else if (highlighted)
        fill = fill.brighter (0.05f);

    g.setColour (fill);
    g.fillRoundedRectangle (r, 3.0f);

    g.setColour (on ? colour::cream : colour::cream.withAlpha (0.28f));
    g.drawRoundedRectangle (r, 3.0f, 1.0f);

    // LED
    auto ledArea = r.removeFromRight (20.0f);
    const auto led = ledArea.withSizeKeepingCentre (7.0f, 7.0f);
    g.setColour (on ? colour::red : juce::Colour (0xff2a2a2f));
    g.fillEllipse (led);
    g.setColour (colour::ink.withAlpha (0.5f));
    g.drawEllipse (led, 0.8f);

    g.setColour (on ? colour::ink : colour::creamDim);
    g.setFont (uiFont (9.5f, true));
    g.drawFittedText (button.getButtonText(), r.reduced (6.0f, 0.0f).toNearestInt(),
                      juce::Justification::centredLeft, 1);
}
} // namespace ladderine::ui
