#include "PluginProcessor.h"

#include "PluginEditor.h"
#include "Parameters.h"
#include "presets/FactoryPresets.h"

#include <cmath>

LadderineAudioProcessor::LadderineAudioProcessor()
    : AudioProcessor (BusesProperties().withOutput ("Output", juce::AudioChannelSet::stereo(), true)),
      apvts (*this, nullptr, "LadderineState", ladderine::createParameterLayout())
{
    for (const auto& preset : ladderine::factoryPresets())
        presetNames.add (preset.name);

    currentPreset.store (0);
}

void LadderineAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    engine.prepare (sampleRate);
    monoBuffer.setSize (1, juce::jmax (1, samplesPerBlock));

    scopeWrite.store (0, std::memory_order_relaxed);
    outputLevel.store (0.0f, std::memory_order_relaxed);
}

void LadderineAudioProcessor::releaseResources()
{
    engine.reset();
}

bool LadderineAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
    const auto& out = layouts.getMainOutputChannelSet();

    return out == juce::AudioChannelSet::mono() || out == juce::AudioChannelSet::stereo();
}

void LadderineAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midi)
{
    juce::ScopedNoDenormals noDenormals;

    const int numSamples = buffer.getNumSamples();
    const int numChannels = buffer.getNumChannels();

    buffer.clear();

    if (numSamples <= 0)
        return;

    if (monoBuffer.getNumSamples() < numSamples)
        monoBuffer.setSize (1, numSamples, false, false, true);

    float* mono = monoBuffer.getWritePointer (0);

    // Merge the on-screen keyboard into the host's MIDI.
    keyboardState.processNextMidiBuffer (midi, 0, numSamples, true);

    engine.setParams (readParams());
    engine.pitchBend (pitchBendValue.load());

    int pos = 0;

    for (const auto metadata : midi)
    {
        const int eventPos = juce::jlimit (0, numSamples, metadata.samplePosition);

        if (eventPos > pos)
        {
            engine.render (mono + pos, eventPos - pos);
            pos = eventPos;
        }

        const auto message = metadata.getMessage();

        if (message.isNoteOn())
        {
            engine.noteOn (message.getNoteNumber(), message.getFloatVelocity());
        }
        else if (message.isNoteOff())
        {
            engine.noteOff (message.getNoteNumber());
        }
        else if (message.isPitchWheel())
        {
            pitchBendValue.store ((message.getPitchWheelValue() - 8192) / 8192.0f);
        }
        else if (message.isController())
        {
            switch (message.getControllerNumber())
            {
                case 1:   // modulation wheel
                    modWheelValue.store (message.getControllerValue() / 127.0f);
                    break;

                case 64:  // sustain
                    engine.sustainPedal (message.getControllerValue() >= 64);
                    break;

                case 120: // all sound off
                case 123: // all notes off
                    engine.allNotesOff (message.getControllerNumber() == 120);
                    break;

                default:
                    break;
            }
        }
        else if (message.isAllNotesOff())
        {
            engine.allNotesOff (false);
        }
        else if (message.isAllSoundOff())
        {
            engine.allNotesOff (true);
        }
    }

    if (pos < numSamples)
        engine.render (mono + pos, numSamples - pos);

    float peak = 0.0f;

    for (int i = 0; i < numSamples; ++i)
    {
        const float s = mono[i];
        peak = juce::jmax (peak, std::abs (s));

        scopeBuffer[(size_t) ((scopeWrite.load (std::memory_order_relaxed) + i) & (kScopeSize - 1))] = s;

        for (int ch = 0; ch < numChannels; ++ch)
            buffer.setSample (ch, i, s);
    }

    scopeWrite.fetch_add (numSamples, std::memory_order_relaxed);
    outputLevel.store (peak, std::memory_order_relaxed);
}

ladderine::EngineParams LadderineAudioProcessor::readParams() const
{
    using namespace ladderine::params;

    auto v = [this] (const char* id)
    {
        const auto* value = apvts.getRawParameterValue (id);
        return value != nullptr ? value->load() : 0.0f;
    };

    ladderine::EngineParams p;

    p.oscWave[0] = juce::roundToInt (v (osc1Wave));
    p.oscRange[0] = juce::roundToInt (v (osc1Range));
    p.oscLevel[0] = v (osc1Level) * 0.1f;
    p.oscOn[0] = v (osc1On) > 0.5f;

    p.oscWave[1] = juce::roundToInt (v (osc2Wave));
    p.oscRange[1] = juce::roundToInt (v (osc2Range));
    p.oscTune[1] = v (osc2Tune);
    p.oscLevel[1] = v (osc2Level) * 0.1f;
    p.oscOn[1] = v (osc2On) > 0.5f;

    p.oscWave[2] = juce::roundToInt (v (osc3Wave));
    p.oscRange[2] = juce::roundToInt (v (osc3Range));
    p.oscTune[2] = v (osc3Tune);
    p.oscLevel[2] = v (osc3Level) * 0.1f;
    p.oscOn[2] = v (osc3On) > 0.5f;
    p.osc3Kbd = v (osc3Kbd) > 0.5f;

    p.noiseLevel = v (noiseLevel) * 0.1f;
    p.drive = v (drive);

    p.cutoffHz = v (filterCutoff);
    p.emphasis = v (filterEmphasis) * 0.1f;
    p.contour = v (filterContour) * 0.1f;
    p.kbdTrack = juce::roundToInt (v (filterKbd));

    p.filterEnv.attack = v (filterAttack);
    p.filterEnv.decay = v (filterDecay);
    p.filterEnv.sustain = v (filterSustain);
    p.filterEnv.release = v (filterRelease);

    p.ampEnv.attack = v (ampAttack);
    p.ampEnv.decay = v (ampDecay);
    p.ampEnv.sustain = v (ampSustain);
    p.ampEnv.release = v (ampRelease);

    p.glideSeconds = v (glideTime);
    p.glideMode = juce::roundToInt (v (glideMode));

    p.modMix = v (modMix);
    p.modDest = juce::roundToInt (v (modDest));
    p.modDepth = v (modDepth);
    p.modWheel = modWheelValue.load();

    p.volumeDb = v (volume);
    p.tuneSemis = v (tune);
    p.drift = v (::ladderine::params::drift);
    p.bendRange = juce::roundToInt (v (bendRange));
    p.velToAmp = v (velAmp);
    p.velToFilter = v (velFilter);

    return p;
}

int LadderineAudioProcessor::getNumPrograms()
{
    return juce::jmax (1, (int) ladderine::factoryPresets().size());
}

void LadderineAudioProcessor::setCurrentProgram (int index)
{
    loadPreset (index);
}

const juce::String LadderineAudioProcessor::getProgramName (int index)
{
    const auto& presets = ladderine::factoryPresets();

    if (index >= 0 && index < (int) presets.size())
        return presets[(size_t) index].name;

    return {};
}

void LadderineAudioProcessor::resetToDefaults()
{
    for (auto* parameter : getParameters())
        parameter->setValueNotifyingHost (parameter->getDefaultValue());
}

void LadderineAudioProcessor::loadPreset (int index)
{
    const auto& presets = ladderine::factoryPresets();

    if (index < 0 || index >= (int) presets.size())
        return;

    resetToDefaults();

    for (const auto& value : presets[(size_t) index].values)
    {
        if (auto* parameter = apvts.getParameter (value.id))
            parameter->setValueNotifyingHost (parameter->convertTo0to1 (value.value));
    }

    currentPreset.store (index);
}

bool LadderineAudioProcessor::savePresetToFile (const juce::File& file)
{
    auto state = apvts.copyState();

    if (const int preset = currentPreset.load(); preset >= 0
        && preset < (int) ladderine::factoryPresets().size())
        state.setProperty ("presetName", getProgramName (preset), nullptr);

    if (auto xml = state.createXml())
        return xml->writeTo (file);

    return false;
}

bool LadderineAudioProcessor::loadPresetFromFile (const juce::File& file)
{
    auto xml = juce::XmlDocument::parse (file);

    if (xml == nullptr)
        return false;

    auto tree = juce::ValueTree::fromXml (*xml);

    if (! tree.isValid() || ! tree.hasType (apvts.state.getType()))
        return false;

    apvts.replaceState (tree);
    currentPreset.store (-1);
    return true;
}

void LadderineAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    auto state = apvts.copyState();
    state.setProperty ("presetIndex", currentPreset.load(), nullptr);

    if (auto xml = state.createXml())
        copyXmlToBinary (*xml, destData);
}

void LadderineAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    if (auto xml = getXmlFromBinary (data, sizeInBytes))
    {
        auto tree = juce::ValueTree::fromXml (*xml);

        if (tree.isValid() && tree.hasType (apvts.state.getType()))
        {
            apvts.replaceState (tree);

            const int preset = (int) tree.getProperty ("presetIndex", 0);
            currentPreset.store (preset >= 0 && preset < (int) ladderine::factoryPresets().size() ? preset
                                                                                               : -1);
        }
    }
}

juce::AudioProcessorEditor* LadderineAudioProcessor::createEditor()
{
    return new LadderineAudioProcessorEditor (*this);
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new LadderineAudioProcessor();
}
