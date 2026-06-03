#include "PluginProcessor.h"
#include "PluginEditor.h"
#include "PluginIDs.h"

juce::AudioProcessorValueTreeState::ParameterLayout MetronomeVSTAudioProcessor::createParameterLayout()
{
    std::vector<std::unique_ptr<juce::RangedAudioParameter>> params;

    params.push_back (std::make_unique<juce::AudioParameterFloat> (
        juce::ParameterID { ParamIDs::bpm, 1 }, "BPM",
        juce::NormalisableRange<float> (20.0f, 300.0f, 1.0f), 120.0f));

    {
        juce::StringArray beatChoices;
        for (int i = 1; i <= 12; ++i)
            beatChoices.add (juce::String (i));
        params.push_back (std::make_unique<juce::AudioParameterChoice> (
            juce::ParameterID { ParamIDs::beatsPerBar, 1 }, "Beats / Bar", beatChoices, 3));
    }

    params.push_back (std::make_unique<juce::AudioParameterChoice> (
        juce::ParameterID { ParamIDs::beatUnit, 1 }, "Beat Unit",
        juce::StringArray { "2", "4", "8", "16" }, 1));

    params.push_back (std::make_unique<juce::AudioParameterBool> (
        juce::ParameterID { ParamIDs::drums, 1 }, "Drums", true));

    params.push_back (std::make_unique<juce::AudioParameterFloat> (
        juce::ParameterID { ParamIDs::drumsVol, 1 }, "Drums Volume",
        juce::NormalisableRange<float> (0.0f, 1.0f, 0.01f), 0.8f));

    params.push_back (std::make_unique<juce::AudioParameterFloat> (
        juce::ParameterID { ParamIDs::clickVol, 1 }, "Click Volume",
        juce::NormalisableRange<float> (0.0f, 1.0f, 0.01f), 0.6f));

    params.push_back (std::make_unique<juce::AudioParameterBool> (
        juce::ParameterID { ParamIDs::useHostTempo, 1 }, "Sync Host Tempo", true));

    params.push_back (std::make_unique<juce::AudioParameterBool> (
        juce::ParameterID { ParamIDs::countdown, 1 }, "Countdown", true));

    params.push_back (std::make_unique<juce::AudioParameterBool> (
        juce::ParameterID { ParamIDs::autoBpm, 1 }, "Auto BPM", false));

    params.push_back (std::make_unique<juce::AudioParameterFloat> (
        juce::ParameterID { ParamIDs::autoMin, 1 }, "Auto Min",
        juce::NormalisableRange<float> (20.0f, 300.0f, 1.0f), 100.0f));

    params.push_back (std::make_unique<juce::AudioParameterFloat> (
        juce::ParameterID { ParamIDs::autoMax, 1 }, "Auto Max",
        juce::NormalisableRange<float> (20.0f, 300.0f, 1.0f), 140.0f));

    params.push_back (std::make_unique<juce::AudioParameterInt> (
        juce::ParameterID { ParamIDs::autoStep, 1 }, "Auto Step", 1, 50, 2));

    params.push_back (std::make_unique<juce::AudioParameterInt> (
        juce::ParameterID { ParamIDs::autoEvery, 1 }, "Auto Every", 1, 256, 4));

    params.push_back (std::make_unique<juce::AudioParameterBool> (
        juce::ParameterID { ParamIDs::autoUnitBars, 1 }, "Unit: Bars", true));

    params.push_back (std::make_unique<juce::AudioParameterBool> (
        juce::ParameterID { ParamIDs::autoLoop, 1 }, "Auto Loop", true));

    params.push_back (std::make_unique<juce::AudioParameterBool> (
        juce::ParameterID { ParamIDs::autoReverse, 1 }, "Auto Reverse", false));

    return { params.begin(), params.end() };
}

MetronomeVSTAudioProcessor::MetronomeVSTAudioProcessor()
    : AudioProcessor (BusesProperties()
                          .withOutput ("Output", juce::AudioChannelSet::stereo(), true)),
      apvts (*this, nullptr, "MetronomeState", createParameterLayout())
{
}

void MetronomeVSTAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    engine.prepare (sampleRate, samplesPerBlock);
    syncEngineFromParams();
}

void MetronomeVSTAudioProcessor::releaseResources() {}

bool MetronomeVSTAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono()
        && layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;
    return true;
}

void MetronomeVSTAudioProcessor::syncEngineFromParams()
{
    MetronomeEngine::Params p;
    p.bpm = *apvts.getRawParameterValue (ParamIDs::bpm);
    p.beatsPerBar = (int) *apvts.getRawParameterValue (ParamIDs::beatsPerBar) + 1;
    const int unitIdx = (int) *apvts.getRawParameterValue (ParamIDs::beatUnit);
    p.beatUnit = unitIdx == 0 ? 2 : (unitIdx == 1 ? 4 : (unitIdx == 2 ? 8 : 16));
    p.drumsEnabled = *apvts.getRawParameterValue (ParamIDs::drums) > 0.5f;
    p.drumsVolume = *apvts.getRawParameterValue (ParamIDs::drumsVol);
    p.clickVolume = *apvts.getRawParameterValue (ParamIDs::clickVol);
    p.useHostTempo = *apvts.getRawParameterValue (ParamIDs::useHostTempo) > 0.5f;
    p.countdownEnabled = *apvts.getRawParameterValue (ParamIDs::countdown) > 0.5f;
    p.autoBpmEnabled = *apvts.getRawParameterValue (ParamIDs::autoBpm) > 0.5f;
    p.autoMin = *apvts.getRawParameterValue (ParamIDs::autoMin);
    p.autoMax = *apvts.getRawParameterValue (ParamIDs::autoMax);
    p.autoStep = (int) *apvts.getRawParameterValue (ParamIDs::autoStep);
    p.autoEvery = (int) *apvts.getRawParameterValue (ParamIDs::autoEvery);
    p.autoUnitBars = *apvts.getRawParameterValue (ParamIDs::autoUnitBars) > 0.5f;
    p.autoLoop = *apvts.getRawParameterValue (ParamIDs::autoLoop) > 0.5f;
    p.autoReverse = *apvts.getRawParameterValue (ParamIDs::autoReverse) > 0.5f;
    engine.setParams (p);
}

void MetronomeVSTAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer&)
{
    juce::ScopedNoDenormals noDenormals;
    syncEngineFromParams();

    bool playing = false;
    if (auto* ph = getPlayHead())
        if (auto pos = ph->getPosition())
            playing = pos->getIsPlaying();

    double hostBpm = 120.0;
    int64_t ppq = 0;
    if (auto* ph = getPlayHead())
    {
        if (auto pos = ph->getPosition())
        {
            if (pos->getBpm().hasValue())
                hostBpm = *pos->getBpm();
            if (pos->getPpqPosition().hasValue())
                ppq = (int64_t) *pos->getPpqPosition();
        }
    }

    engine.process (buffer, buffer.getNumSamples(), playing, hostBpm, ppq);
}

juce::AudioProcessorEditor* MetronomeVSTAudioProcessor::createEditor()
{
    return new MetronomeVSTAudioProcessorEditor (*this);
}

void MetronomeVSTAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    saveCurrentProgram();
    auto state = apvts.copyState();
    state.setProperty ("currentProgram", currentProgram, nullptr);
    juce::MemoryOutputStream stream (destData, true);
    state.writeToStream (stream);
}

void MetronomeVSTAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    auto tree = juce::ValueTree::readFromData (data, (size_t) sizeInBytes);
    if (tree.isValid())
    {
        apvts.replaceState (tree);
        currentProgram = tree.getProperty ("currentProgram", 0);
        loadProgram (currentProgram);
    }
}

void MetronomeVSTAudioProcessor::saveCurrentProgram()
{
    auto& p = programs[(size_t) currentProgram];
    p.bpm = *apvts.getRawParameterValue (ParamIDs::bpm);
    p.beatsPerBar = (int) *apvts.getRawParameterValue (ParamIDs::beatsPerBar) + 1;
    p.beatUnit = (int) *apvts.getRawParameterValue (ParamIDs::beatUnit);
    p.drums = *apvts.getRawParameterValue (ParamIDs::drums) > 0.5f;
    p.drumsVol = *apvts.getRawParameterValue (ParamIDs::drumsVol);
    p.countdown = *apvts.getRawParameterValue (ParamIDs::countdown) > 0.5f;
    p.autoBpm = *apvts.getRawParameterValue (ParamIDs::autoBpm) > 0.5f;
    p.autoMin = *apvts.getRawParameterValue (ParamIDs::autoMin);
    p.autoMax = *apvts.getRawParameterValue (ParamIDs::autoMax);
    p.autoStep = (int) *apvts.getRawParameterValue (ParamIDs::autoStep);
    p.autoEvery = (int) *apvts.getRawParameterValue (ParamIDs::autoEvery);
    p.autoUnitBars = *apvts.getRawParameterValue (ParamIDs::autoUnitBars) > 0.5f;
    p.autoLoop = *apvts.getRawParameterValue (ParamIDs::autoLoop) > 0.5f;
    p.autoReverse = *apvts.getRawParameterValue (ParamIDs::autoReverse) > 0.5f;
}

void MetronomeVSTAudioProcessor::loadProgram (int index)
{
    index = juce::jlimit (0, 4, index);
    const auto& p = programs[(size_t) index];
    apvts.getParameter (ParamIDs::bpm)->setValueNotifyingHost (
        apvts.getParameterRange (ParamIDs::bpm).convertTo0to1 (p.bpm));
    apvts.getParameter (ParamIDs::beatsPerBar)->setValueNotifyingHost (
        apvts.getParameterRange (ParamIDs::beatsPerBar).convertTo0to1 ((float) juce::jmax (0, p.beatsPerBar - 1)));
    apvts.getParameter (ParamIDs::beatUnit)->setValueNotifyingHost (
        apvts.getParameterRange (ParamIDs::beatUnit).convertTo0to1 ((float) p.beatUnit));
    apvts.getParameter (ParamIDs::drums)->setValueNotifyingHost (p.drums ? 1.0f : 0.0f);
    apvts.getParameter (ParamIDs::drumsVol)->setValueNotifyingHost (
        apvts.getParameterRange (ParamIDs::drumsVol).convertTo0to1 (p.drumsVol));
    apvts.getParameter (ParamIDs::countdown)->setValueNotifyingHost (p.countdown ? 1.0f : 0.0f);
    apvts.getParameter (ParamIDs::autoBpm)->setValueNotifyingHost (p.autoBpm ? 1.0f : 0.0f);
    apvts.getParameter (ParamIDs::autoMin)->setValueNotifyingHost (
        apvts.getParameterRange (ParamIDs::autoMin).convertTo0to1 (p.autoMin));
    apvts.getParameter (ParamIDs::autoMax)->setValueNotifyingHost (
        apvts.getParameterRange (ParamIDs::autoMax).convertTo0to1 (p.autoMax));
    apvts.getParameter (ParamIDs::autoStep)->setValueNotifyingHost (
        apvts.getParameterRange (ParamIDs::autoStep).convertTo0to1 ((float) p.autoStep));
    apvts.getParameter (ParamIDs::autoEvery)->setValueNotifyingHost (
        apvts.getParameterRange (ParamIDs::autoEvery).convertTo0to1 ((float) p.autoEvery));
    apvts.getParameter (ParamIDs::autoUnitBars)->setValueNotifyingHost (p.autoUnitBars ? 1.0f : 0.0f);
    apvts.getParameter (ParamIDs::autoLoop)->setValueNotifyingHost (p.autoLoop ? 1.0f : 0.0f);
    apvts.getParameter (ParamIDs::autoReverse)->setValueNotifyingHost (p.autoReverse ? 1.0f : 0.0f);
}

void MetronomeVSTAudioProcessor::setCurrentProgram (int index)
{
    saveCurrentProgram();
    currentProgram = juce::jlimit (0, 4, index);
    loadProgram (currentProgram);
}

const juce::String MetronomeVSTAudioProcessor::getProgramName (int index)
{
    return "Profile " + juce::String (index + 1);
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new MetronomeVSTAudioProcessor();
}
