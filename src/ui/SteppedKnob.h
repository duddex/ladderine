#pragma once

#include <juce_audio_processors/juce_audio_processors.h>

#include "ui/LookAndFeel.h"

namespace ladderine::ui
{
class SteppedKnob : public juce::Component,
                    public juce::SettableTooltipClient
{
public:
    SteppedKnob() = default;

    void init (juce::RangedAudioParameter& parameter,
               juce::StringArray labels,
               juce::String caption);

    void paint (juce::Graphics&) override;
    void mouseDown (const juce::MouseEvent&) override;
    void mouseDrag (const juce::MouseEvent&) override;
    void mouseWheelMove (const juce::MouseEvent&, const juce::MouseWheelDetails&) override;

    void setIndex (int newIndex, bool notify);
    int getIndex() const noexcept { return index_; }

private:
    juce::RangedAudioParameter* parameter_ = nullptr;
    juce::StringArray labels_;
    juce::String caption_;
    std::unique_ptr<juce::ParameterAttachment> attachment_;

    int index_ = 0;
    int dragStartIndex_ = 0;
    int dragStartY_ = 0;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SteppedKnob)
};
} // namespace ladderine::ui
