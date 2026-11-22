// Copyright (C) 2026 duddex
// SPDX-License-Identifier: AGPL-3.0-or-later

#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_audio_utils/juce_audio_utils.h>

#include <array>
#include <atomic>

#include "engine/SynthEngine.h"

class LadderineAudioProcessor : public juce::AudioProcessor
{
public:
    static constexpr int kScopeSize = 4096;

    LadderineAudioProcessor();
    ~LadderineAudioProcessor() override = default;

    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override { return true; }

    const juce::String getName() const override { return "Ladderine"; }
    bool acceptsMidi() const override { return true; }
    bool producesMidi() const override { return false; }
    bool isMidiEffect() const override { return false; }
    double getTailLengthSeconds() const override { return 10.0; }

    int getNumPrograms() override;
    int getCurrentProgram() override { return juce::jmax (0, currentPreset.load()); }
    void setCurrentProgram (int index) override;
    const juce::String getProgramName (int index) override;
    void changeProgramName (int, const juce::String&) override {}

    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;

    juce::AudioProcessorValueTreeState apvts;
    juce::MidiKeyboardState keyboardState;

    // Performance controls (not automatable parameters: they follow the
    // hardware wheels and the host's MIDI).
    void setModWheel (float value) noexcept { modWheelValue.store (juce::jlimit (0.0f, 1.0f, value)); }
    float getModWheel() const noexcept { return modWheelValue.load(); }
    void setPitchBend (float value) noexcept { pitchBendValue.store (juce::jlimit (-1.0f, 1.0f, value)); }
    float getPitchBend() const noexcept { return pitchBendValue.load(); }

    int getCurrentPreset() const noexcept { return currentPreset.load(); }
    const juce::StringArray& getPresetNames() const noexcept { return presetNames; }
    void loadPreset (int index);

    bool savePresetToFile (const juce::File& file);
    bool loadPresetFromFile (const juce::File& file);

    const float* getScopeData() const noexcept { return scopeBuffer.data(); }
    int getScopeWriteIndex() const noexcept { return scopeWrite.load (std::memory_order_relaxed); }
    float getOutputLevel() const noexcept { return outputLevel.load (std::memory_order_relaxed); }

private:
    ladderine::EngineParams readParams() const;
    void resetToDefaults();

    ladderine::SynthEngine engine;

    juce::AudioBuffer<float> monoBuffer;
    std::array<float, kScopeSize> scopeBuffer {};
    std::atomic<int> scopeWrite { 0 };
    std::atomic<float> outputLevel { 0.0f };

    std::atomic<float> modWheelValue { 0.0f };
    std::atomic<float> pitchBendValue { 0.0f };
    std::atomic<int> currentPreset { 0 };

    juce::StringArray presetNames;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (LadderineAudioProcessor)
};

