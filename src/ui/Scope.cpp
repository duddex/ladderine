#include "Scope.h"

#include "PluginProcessor.h"
#include "LookAndFeel.h"

#include <cmath>

namespace ladderine::ui
{
void Scope::paint (juce::Graphics& g)
{
    const auto bounds = getLocalBounds().toFloat();

    g.setColour (colour::screen);
    g.fillRoundedRectangle (bounds, 4.0f);

    g.setColour (colour::cream.withAlpha (0.09f));
    g.drawRoundedRectangle (bounds.reduced (0.5f), 4.0f, 1.0f);

    auto screen = bounds.reduced (6.0f, 5.0f);
    auto meter = screen.removeFromBottom (10.0f);

    // grid
    g.setColour (colour::cream.withAlpha (0.06f));

    for (int i = 1; i < 4; ++i)
    {
        const float y = screen.getY() + screen.getHeight() * (float) i / 4.0f;
        g.drawHorizontalLine (juce::roundToInt (y), screen.getX(), screen.getRight());
    }

    for (int i = 1; i < 8; ++i)
    {
        const float x = screen.getX() + screen.getWidth() * (float) i / 8.0f;
        g.drawVerticalLine (juce::roundToInt (x), screen.getY(), screen.getBottom());
    }

    // level meter
    {
        auto labelArea = meter.removeFromLeft (26.0f);

        g.setColour (colour::ink);
        g.fillRect (meter);
        g.setColour (colour::cream.withAlpha (0.25f));
        g.drawRect (meter, 1.0f);

        const float level = processor_ != nullptr ? processor_->getOutputLevel() : 0.0f;
        const float db = juce::Decibels::gainToDecibels (level, -60.0f);
        const float norm = juce::jlimit (0.0f, 1.0f, (db + 60.0f) / 60.0f);

        g.setColour (db > -1.0f ? colour::red : colour::cream.withAlpha (0.65f));
        g.fillRect (meter.withWidth (meter.getWidth() * norm).reduced (1.0f));

        g.setColour (colour::creamDim);
        g.setFont (uiFont (7.5f, true));
        g.drawFittedText ("OUT", labelArea.toNearestInt(), juce::Justification::centredLeft, 1);
    }

    if (processor_ == nullptr)
        return;

    const float* data = processor_->getScopeData();
    const int write = processor_->getScopeWriteIndex();
    constexpr int ringSize = LadderineAudioProcessor::kScopeSize;

    // Trigger on the most recent rising zero crossing for a stable picture.
    int anchor = write - 1;

    for (int i = 1; i < 1800; ++i)
    {
        const int idx = (write - i) & (ringSize - 1);
        const int prev = (idx - 1) & (ringSize - 1);

        if (data[prev] <= 0.0f && data[idx] > 0.0f)
        {
            anchor = idx;
            break;
        }
    }

    const int span = 1400;
    const int columns = juce::jmax (1, (int) screen.getWidth());
    const float centreY = screen.getCentreY();
    const float halfHeight = screen.getHeight() * 0.45f;

    auto sampleToY = [centreY, halfHeight] (float s)
    {
        return centreY - juce::jlimit (-1.3f, 1.3f, s) * halfHeight;
    };

    for (int x = 0; x < columns; ++x)
    {
        const int from = anchor + x * span / columns;
        const int to = juce::jmax (from + 1, anchor + (x + 1) * span / columns);

        float lo = 1.0f;
        float hi = -1.0f;

        for (int i = from; i < to; ++i)
        {
            const float s = data[i & (ringSize - 1)];
            lo = juce::jmin (lo, s);
            hi = juce::jmax (hi, s);
        }

        g.setColour (colour::cream.withAlpha (0.16f));
        g.drawVerticalLine ((int) screen.getX() + x, sampleToY (hi) - 1.0f, sampleToY (lo) + 1.0f);

        g.setColour (colour::cream.withAlpha (0.85f));
        g.drawVerticalLine ((int) screen.getX() + x, sampleToY (hi), sampleToY (lo));
    }
}
} // namespace ladderine::ui
