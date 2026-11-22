// Copyright (C) 2026 duddex
// SPDX-License-Identifier: AGPL-3.0-or-later

#include "PluginEditor.h"

#include "PluginProcessor.h"
#include "Parameters.h"
#include "ui/Knob.h"
#include "ui/Scope.h"
#include "ui/SteppedKnob.h"
#include "ui/Wheel.h"

#include <juce_audio_utils/juce_audio_utils.h>

using namespace ladderine::ui;
using namespace ladderine::params;

namespace
{
constexpr int designWidth = 1120;
constexpr int designHeight = 760;

void paintWood (juce::Graphics& g, juce::Rectangle<int> area)
{
    juce::ColourGradient gradient (colour::woodDark, (float) area.getX(), 0.0f,
                                   colour::wood, (float) area.getX(), (float) area.getHeight(),
                                   false);
    g.setGradientFill (gradient);
    g.fillRect (area);

    g.setColour (juce::Colour (0x33000000));

    for (int i = 0; i < 14; ++i)
    {
        const float x = (float) area.getX() + 2.0f + (float) i * 1.05f;
        g.drawVerticalLine ((int) x, 0.0f, (float) area.getHeight());
    }

    g.setColour (juce::Colour (0x22000000));
    g.drawVerticalLine (area.getRight() - 1, 0.0f, (float) area.getHeight());
    g.setColour (colour::ink.withAlpha (0.6f));
    g.drawVerticalLine (area.getX(), 0.0f, (float) area.getHeight());
}
} // namespace

// ===========================================================================
// The whole instrument panel, laid out on a fixed 1120 x 760 canvas.
// ===========================================================================
class RootComponent final : public juce::Component,
                            private juce::Timer
{
public:
    explicit RootComponent (LadderineAudioProcessor& processor);
    ~RootComponent() override;

    void paint (juce::Graphics&) override;
    void resized() override;

private:
    void timerCallback() override;

    void setupKnob (Knob&, const char* id, const char* caption);
    void setupStepped (SteppedKnob&, const char* id, const juce::StringArray& labels,
                       const char* caption);
    void setupToggle (juce::ToggleButton&, const char* id, const char* text,
                      std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment>&);

    void drawPanel (juce::Graphics&, juce::Rectangle<int>, const juce::String& title);
    void drawEnvelope (juce::Graphics&, juce::Rectangle<int>, float sustain);

    void cyclePreset (int delta);
    void savePresetDialog();
    void loadPresetDialog();

    LadderineAudioProcessor& processor_;

    Scope scope_;
    Wheel modWheel_, pitchWheel_;
    std::unique_ptr<juce::MidiKeyboardComponent> keyboard_;

    // oscillators / mixer
    Knob osc1Level_, osc2Level_, osc3Level_, noiseLevel_, drive_;
    Knob osc2Tune_, osc3Tune_;
    SteppedKnob osc1Wave_, osc1Range_, osc2Wave_, osc2Range_, osc3Wave_, osc3Range_;
    juce::ToggleButton osc1On_, osc2On_, osc3On_, osc3Kbd_;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> osc1OnAttach_,
        osc2OnAttach_, osc3OnAttach_, osc3KbdAttach_;

    // filter and contours
    Knob cutoff_, emphasis_, contour_;
    SteppedKnob filterKbd_;
    Knob filterA_, filterD_, filterS_, filterR_;
    Knob ampA_, ampD_, ampS_, ampR_;

    // performance / modulation
    Knob glide_, modMix_, modDepth_;
    SteppedKnob glideMode_, modDest_;
    Knob tune_, drift_, bend_, velAmp_, velFilter_, volume_;

    // preset bar
    juce::ComboBox presetBox_;
    juce::TextButton prevButton_, nextButton_, saveButton_, loadButton_;
    std::unique_ptr<juce::FileChooser> fileChooser_;

    int lastPreset_ = -2;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (RootComponent)
};

RootComponent::RootComponent (LadderineAudioProcessor& processor)
    : processor_ (processor)
{
    scope_.setProcessor (&processor);
    addAndMakeVisible (scope_);

    // ---------------------------------------------------------- oscillators
    setupKnob (osc1Level_, osc1Level, "OSC 1");
    setupKnob (osc2Level_, osc2Level, "OSC 2");
    setupKnob (osc3Level_, osc3Level, "OSC 3");
    setupKnob (noiseLevel_, noiseLevel, "NOISE");
    setupKnob (drive_, drive, "DRIVE");

    setupKnob (osc2Tune_, osc2Tune, "FREQ");
    setupKnob (osc3Tune_, osc3Tune, "FREQ");

    setupStepped (osc1Wave_, osc1Wave, waveNames, "WAVE");
    setupStepped (osc1Range_, osc1Range, rangeNames, "RANGE");
    setupStepped (osc2Wave_, osc2Wave, waveNames, "WAVE");
    setupStepped (osc2Range_, osc2Range, rangeNames, "RANGE");
    setupStepped (osc3Wave_, osc3Wave, waveNames, "WAVE");
    setupStepped (osc3Range_, osc3Range, osc3Ranges, "RANGE");

    setupToggle (osc1On_, osc1On, "ON", osc1OnAttach_);
    setupToggle (osc2On_, osc2On, "ON", osc2OnAttach_);
    setupToggle (osc3On_, osc3On, "ON", osc3OnAttach_);
    setupToggle (osc3Kbd_, osc3Kbd, "KBD", osc3KbdAttach_);

    // --------------------------------------------------------------- filter
    setupKnob (cutoff_, filterCutoff, "CUTOFF");
    setupKnob (emphasis_, filterEmphasis, "EMPHASIS");
    setupKnob (contour_, filterContour, "CONTOUR");
    setupStepped (filterKbd_, filterKbd, kbdNames, "KBD");

    setupKnob (filterA_, filterAttack, "A");
    setupKnob (filterD_, filterDecay, "D");
    setupKnob (filterS_, filterSustain, "S");
    setupKnob (filterR_, filterRelease, "R");

    // ------------------------------------------------------------- loudness
    setupKnob (ampA_, ampAttack, "A");
    setupKnob (ampD_, ampDecay, "D");
    setupKnob (ampS_, ampSustain, "S");
    setupKnob (ampR_, ampRelease, "R");

    // ---------------------------------------------------------- performance
    setupKnob (glide_, glideTime, "GLIDE");
    setupStepped (glideMode_, glideMode, glideNames, "MODE");
    setupKnob (tune_, tune, "TUNE");
    setupKnob (drift_, drift, "DRIFT");
    setupKnob (bend_, bendRange, "BEND");
    setupKnob (velAmp_, velAmp, "VEL AMP");
    setupKnob (velFilter_, velFilter, "VEL FILT");
    setupKnob (volume_, volume, "VOLUME");

    // ------------------------------------------------------------ modulation
    setupKnob (modMix_, modMix, "SOURCE");
    setupStepped (modDest_, modDest, destNames, "DEST");
    setupKnob (modDepth_, modDepth, "DEPTH");

    // ----------------------------------------------------------- preset bar
    presetBox_.addItemList (processor_.getPresetNames(), 1);
    presetBox_.setSelectedId (processor_.getCurrentPreset() + 1, juce::dontSendNotification);
    presetBox_.onChange = [this]
    {
        const int id = presetBox_.getSelectedId();

        if (id > 0)
            processor_.loadPreset (id - 1);
    };
    addAndMakeVisible (presetBox_);

    prevButton_.setButtonText ("<");
    nextButton_.setButtonText (">");
    saveButton_.setButtonText ("SAVE");
    loadButton_.setButtonText ("LOAD");

    prevButton_.onClick = [this] { cyclePreset (-1); };
    nextButton_.onClick = [this] { cyclePreset (1); };
    saveButton_.onClick = [this] { savePresetDialog(); };
    loadButton_.onClick = [this] { loadPresetDialog(); };

    addAndMakeVisible (prevButton_);
    addAndMakeVisible (nextButton_);
    addAndMakeVisible (saveButton_);
    addAndMakeVisible (loadButton_);

    // -------------------------------------------------------------- keyboard
    keyboard_ = std::make_unique<juce::MidiKeyboardComponent> (
        processor_.keyboardState, juce::MidiKeyboardComponent::horizontalKeyboard);
    keyboard_->setAvailableRange (36, 96);
    keyboard_->setLowestVisibleKey (36);
    keyboard_->setKeyWidth (26.5f);
    keyboard_->setScrollButtonsVisible (false);
    keyboard_->setVelocity (0.9f, false);
    keyboard_->setColour (juce::MidiKeyboardComponent::whiteNoteColourId, colour::cream);
    keyboard_->setColour (juce::MidiKeyboardComponent::blackNoteColourId, colour::ink);
    keyboard_->setColour (juce::MidiKeyboardComponent::keySeparatorLineColourId,
                          colour::ink.withAlpha (0.65f));
    keyboard_->setColour (juce::MidiKeyboardComponent::mouseOverKeyOverlayColourId,
                          colour::gold.withAlpha (0.25f));
    keyboard_->setColour (juce::MidiKeyboardComponent::keyDownOverlayColourId,
                          colour::red.withAlpha (0.7f));
    keyboard_->setColour (juce::MidiKeyboardComponent::shadowColourId,
                          colour::ink.withAlpha (0.5f));
    addAndMakeVisible (*keyboard_);

    modWheel_.init ("MOD", 0.0f, 1.0f, false, [this] (float v) { processor_.setModWheel (v); });
    pitchWheel_.init ("PITCH", -1.0f, 1.0f, true, [this] (float v) { processor_.setPitchBend (v); });
    addAndMakeVisible (modWheel_);
    addAndMakeVisible (pitchWheel_);

    lastPreset_ = processor_.getCurrentPreset();
    startTimerHz (30);
}

RootComponent::~RootComponent()
{
    stopTimer();
    setLookAndFeel (nullptr);
}

void RootComponent::setupKnob (Knob& knob, const char* id, const char* caption)
{
    knob.init (processor_.apvts, id, caption);
    addAndMakeVisible (knob);
}

void RootComponent::setupStepped (SteppedKnob& knob, const char* id,
                                  const juce::StringArray& labels, const char* caption)
{
    if (auto* parameter = processor_.apvts.getParameter (id))
        knob.init (*parameter, labels, caption);

    addAndMakeVisible (knob);
}

void RootComponent::setupToggle (
    juce::ToggleButton& button, const char* id, const char* text,
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment>& attachment)
{
    button.setButtonText (text);
    addAndMakeVisible (button);
    attachment = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment> (
        processor_.apvts, id, button);
}

void RootComponent::cyclePreset (int delta)
{
    const int count = processor_.getPresetNames().size();

    if (count <= 0)
        return;

    int current = processor_.getCurrentPreset();

    if (current < 0)
        current = 0;
    else
        current = (current + delta + count) % count;

    processor_.loadPreset (current);
    presetBox_.setSelectedId (current + 1, juce::dontSendNotification);
    lastPreset_ = current;
}

void RootComponent::savePresetDialog()
{
    fileChooser_ = std::make_unique<juce::FileChooser> (
        "Save Ladderine preset",
        juce::File::getSpecialLocation (juce::File::userDocumentsDirectory)
            .getChildFile ("Ladderine.ladderinepreset"),
        "*.ladderinepreset");

    fileChooser_->launchAsync (juce::FileBrowserComponent::saveMode
                                   | juce::FileBrowserComponent::canSelectFiles
                                   | juce::FileBrowserComponent::warnAboutOverwriting,
                               [this] (const juce::FileChooser& chooser)
                               {
                                   const auto file = chooser.getResult();

                                   if (file != juce::File())
                                       processor_.savePresetToFile (file.withFileExtension (".ladderinepreset"));
                               });
}

void RootComponent::loadPresetDialog()
{
    fileChooser_ = std::make_unique<juce::FileChooser> (
        "Load Ladderine preset",
        juce::File::getSpecialLocation (juce::File::userDocumentsDirectory),
        "*.ladderinepreset");

    fileChooser_->launchAsync (juce::FileBrowserComponent::openMode
                                   | juce::FileBrowserComponent::canSelectFiles,
                               [this] (const juce::FileChooser& chooser)
                               {
                                   const auto file = chooser.getResult();

                                   if (file.existsAsFile() && processor_.loadPresetFromFile (file))
                                       lastPreset_ = -2;
                               });
}

void RootComponent::timerCallback()
{
    scope_.repaint();

    modWheel_.setDisplayValue (processor_.getModWheel());
    pitchWheel_.setDisplayValue (processor_.getPitchBend());

    const int current = processor_.getCurrentPreset();

    if (current != lastPreset_)
    {
        if (current >= 0)
            presetBox_.setSelectedId (current + 1, juce::dontSendNotification);
        else
            presetBox_.setText ("USER", juce::dontSendNotification);

        lastPreset_ = current;
    }
}

void RootComponent::paint (juce::Graphics& g)
{
    g.fillAll (colour::ink);

    paintWood (g, { 0, 0, 16, designHeight });
    paintWood (g, { designWidth - 16, 0, 16, designHeight });

    juce::ColourGradient body (colour::panel, 0.0f, 0.0f,
                               juce::Colour (0xff0f0f12), 0.0f, (float) designHeight, false);
    g.setGradientFill (body);
    g.fillRect (16, 0, designWidth - 32, designHeight);

    g.setColour (colour::cream.withAlpha (0.10f));
    g.drawVerticalLine (16, 0.0f, (float) designHeight);
    g.drawVerticalLine (designWidth - 17, 0.0f, (float) designHeight);

    // ---------------------------------------------------------------- header
    g.setColour (colour::cream);
    g.setFont (logoFont (29.0f));
    g.drawText ("LADDERINE", 30, 8, 235, 36, juce::Justification::centredLeft);

    g.setColour (colour::red);
    g.setFont (uiFont (12.0f, true));
    g.drawText ("MONO", 246, 13, 80, 22, juce::Justification::centredLeft);

    g.setColour (colour::creamDim);
    g.setFont (uiFont (8.5f, true));
    g.drawText ("MONOPHONIC VIRTUAL-ANALOGUE SYNTHESIZER", 33, 42, 430, 14,
                juce::Justification::centredLeft);

    g.setColour (colour::cream.withAlpha (0.10f));
    g.drawHorizontalLine (64, 28.0f, (float) designWidth - 28.0f);

    g.setColour (colour::creamDim);
    g.setFont (uiFont (8.5f, true));
    g.drawText ("PRESET", 596, 24, 60, 14, juce::Justification::centredRight);

    // a small power lamp
    g.setColour (colour::red);
    g.fillEllipse (1088.0f, 10.0f, 6.0f, 6.0f);
    g.setColour (colour::ink.withAlpha (0.6f));
    g.drawEllipse (1088.0f, 10.0f, 6.0f, 6.0f, 0.8f);

    // ---------------------------------------------------------------- panels
    drawPanel (g, { 24, 70, 252, 212 }, "OSCILLATOR 1");
    drawPanel (g, { 284, 70, 252, 212 }, "OSCILLATOR 2");
    drawPanel (g, { 544, 70, 252, 212 }, "OSCILLATOR 3");
    drawPanel (g, { 804, 70, 292, 212 }, "MIXER");

    drawPanel (g, { 24, 292, 184, 220 }, "MODULATION");
    drawPanel (g, { 216, 292, 330, 220 }, "FILTER");
    drawPanel (g, { 554, 292, 258, 220 }, "FILTER CONTOUR");
    drawPanel (g, { 820, 292, 276, 220 }, "LOUDNESS CONTOUR");

    drawPanel (g, { 24, 522, 560, 128 }, "PERFORMANCE");
    drawPanel (g, { 596, 522, 500, 128 }, "OUTPUT");

    g.setColour (colour::creamDim);
    g.setFont (uiFont (8.0f, true));
    g.drawText ("24 dB / OCT", 234, 486, 120, 12, juce::Justification::centredLeft);

    g.setColour (colour::cream.withAlpha (0.22f));
    g.setFont (uiFont (8.0f));
    g.drawText ("OSC 3 / NOISE", 108, 482, 78, 12, juce::Justification::centredLeft);

    drawEnvelope (g, { 570, 438, 226, 40 }, 0.25f);
    drawEnvelope (g, { 836, 438, 244, 40 }, 0.8f);

    // -------------------------------------------------------------- keyboard
    g.setColour (colour::cream.withAlpha (0.12f));
    g.drawHorizontalLine (654, 28.0f, (float) designWidth - 28.0f);

    g.setColour (colour::creamDim);
}

void RootComponent::drawPanel (juce::Graphics& g, juce::Rectangle<int> area,
                               const juce::String& title)
{
    const auto r = area.toFloat();

    g.setColour (colour::panelLight);
    g.fillRoundedRectangle (r, 5.0f);

    g.setColour (colour::cream.withAlpha (0.08f));
    g.drawRoundedRectangle (r.reduced (0.5f), 5.0f, 1.0f);

    g.setColour (colour::cream.withAlpha (0.62f));
    g.setFont (uiFont (9.0f, true).withExtraKerningFactor (0.14f));
    g.drawText (title, area.reduced (12, 0).withTrimmedTop (7).withHeight (14),
                juce::Justification::centredLeft);

    g.setColour (colour::cream.withAlpha (0.12f));
    g.drawHorizontalLine (area.getY() + 26, (float) area.getX() + 12.0f,
                          (float) area.getRight() - 12.0f);
}

void RootComponent::drawEnvelope (juce::Graphics& g, juce::Rectangle<int> area, float sustain)
{
    const float x0 = (float) area.getX();
    const float x1 = (float) area.getRight();
    const float bottom = (float) area.getBottom();
    const float top = (float) area.getY();
    const float width = x1 - x0;

    const float sustainY = bottom - (bottom - top) * juce::jlimit (0.08f, 0.92f, sustain);

    juce::Path path;
    path.startNewSubPath (x0, bottom);
    path.lineTo (x0 + width * 0.26f, top);
    path.lineTo (x0 + width * 0.47f, sustainY);
    path.lineTo (x0 + width * 0.74f, sustainY);
    path.lineTo (x1, bottom);

    g.setColour (colour::cream.withAlpha (0.22f));
    g.strokePath (path, juce::PathStrokeType (1.1f));
}

void RootComponent::resized()
{
    // oscillators
    osc1Wave_.setBounds (42, 108, 84, 84);
    osc1Range_.setBounds (132, 108, 84, 84);
    osc1On_.setBounds (42, 228, 100, 28);

    osc2Wave_.setBounds (302, 108, 84, 84);
    osc2Range_.setBounds (392, 108, 84, 84);
    osc2Tune_.setBounds (480, 108, 44, 72);
    osc2On_.setBounds (302, 228, 100, 28);

    osc3Wave_.setBounds (562, 108, 84, 84);
    osc3Range_.setBounds (652, 108, 84, 84);
    osc3Tune_.setBounds (740, 108, 44, 72);
    osc3On_.setBounds (562, 228, 100, 28);
    osc3Kbd_.setBounds (670, 228, 100, 28);

    // mixer
    osc1Level_.setBounds (816, 110, 52, 76);
    osc2Level_.setBounds (872, 110, 52, 76);
    osc3Level_.setBounds (928, 110, 52, 76);
    noiseLevel_.setBounds (984, 110, 52, 76);
    drive_.setBounds (1040, 110, 52, 76);

    // modulation
    modMix_.setBounds (42, 332, 64, 84);
    modDest_.setBounds (126, 332, 64, 84);
    modDepth_.setBounds (46, 428, 56, 76);

    // filter
    cutoff_.setBounds (234, 332, 84, 104);
    emphasis_.setBounds (332, 344, 60, 84);
    contour_.setBounds (402, 344, 60, 84);
    filterKbd_.setBounds (470, 344, 58, 84);

    // contours
    filterA_.setBounds (570, 346, 48, 70);
    filterD_.setBounds (632, 346, 48, 70);
    filterS_.setBounds (694, 346, 48, 70);
    filterR_.setBounds (756, 346, 48, 70);

    ampA_.setBounds (836, 346, 48, 70);
    ampD_.setBounds (904, 346, 48, 70);
    ampS_.setBounds (972, 346, 48, 70);
    ampR_.setBounds (1040, 346, 48, 70);

    // performance
    glide_.setBounds (38, 554, 54, 82);
    glideMode_.setBounds (105, 554, 54, 82);
    tune_.setBounds (172, 554, 54, 82);
    drift_.setBounds (239, 554, 54, 82);
    bend_.setBounds (306, 554, 54, 82);
    velAmp_.setBounds (373, 554, 54, 82);
    velFilter_.setBounds (440, 554, 54, 82);
    volume_.setBounds (507, 554, 54, 82);

    // header
    prevButton_.setBounds (664, 20, 26, 26);
    presetBox_.setBounds (696, 20, 240, 26);
    nextButton_.setBounds (942, 20, 26, 26);
    saveButton_.setBounds (976, 20, 52, 26);
    loadButton_.setBounds (1034, 20, 52, 26);

    // output scope
    scope_.setBounds (610, 556, 472, 80);

    // keyboard and wheels
    modWheel_.setBounds (30, 658, 44, 90);
    pitchWheel_.setBounds (80, 658, 44, 90);

    if (keyboard_ != nullptr)
        keyboard_->setBounds (134, 658, 962, 90);
}

// ===========================================================================
// Editor
// ===========================================================================
LadderineAudioProcessorEditor::LadderineAudioProcessorEditor (LadderineAudioProcessor& processor)
    : AudioProcessorEditor (&processor), processor_ (processor)
{
    root_ = std::make_unique<RootComponent> (processor_);
    root_->setLookAndFeel (&lookAndFeel_);
    addAndMakeVisible (*root_);

    setSize (designWidth, designHeight);
    setResizable (true, true);
    setResizeLimits (784, 532, 1680, 1140);

    if (auto* ratioConstrainer = getConstrainer())
        ratioConstrainer->setFixedAspectRatio ((double) designWidth / (double) designHeight);

    setWantsKeyboardFocus (false);
}

LadderineAudioProcessorEditor::~LadderineAudioProcessorEditor()
{
    root_->setLookAndFeel (nullptr);
}

void LadderineAudioProcessorEditor::paint (juce::Graphics& g)
{
    g.fillAll (colour::ink);
}

void LadderineAudioProcessorEditor::resized()
{
    const float scale = juce::jmin ((float) getWidth() / (float) designWidth,
                                    (float) getHeight() / (float) designHeight);

    const float offsetX = ((float) getWidth() - (float) designWidth * scale) * 0.5f;
    const float offsetY = ((float) getHeight() - (float) designHeight * scale) * 0.5f;

    root_->setSize (designWidth, designHeight);
    root_->setTransform (juce::AffineTransform::scale (scale).translated (offsetX, offsetY));
}



