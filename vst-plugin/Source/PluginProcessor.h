#pragma once

#include <JuceHeader.h>
#include "MetronomeEngine.h"

class MetronomeVSTAudioProcessor : public juce::AudioProcessor
{
public:
    MetronomeVSTAudioProcessor();
    ~MetronomeVSTAudioProcessor() override = default;

    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override { return true; }

    const juce::String getName() const override { return JucePlugin_Name; }
    bool acceptsMidi() const override { return false; }
    bool producesMidi() const override { return false; }
    bool isMidiEffect() const override { return false; }
    double getTailLengthSeconds() const override { return 0.0; }

    int getNumPrograms() override { return 5; }
    int getCurrentProgram() override { return currentProgram; }
    void setCurrentProgram (int index) override;
    const juce::String getProgramName (int index) override;
    void changeProgramName (int, const juce::String&) override {}

    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;

    juce::AudioProcessorValueTreeState& getAPVTS() { return apvts; }

    MetronomeEngine& getEngine() { return engine; }

    void startMetronome() { engine.start(); }
    void stopMetronome()  { engine.stop(); }
    bool isMetronomeRunning() const { return engine.isRunning(); }

    static juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();

private:
    MetronomeEngine engine;
    juce::AudioProcessorValueTreeState apvts;
    int currentProgram = 0;

    struct ProgramState
    {
        float bpm = 120.0f;
        int beatsPerBar = 4;  // numerator
        int beatUnit = 1;     // choice index: 0=2, 1=4, 2=8, 3=16
        bool drums = true;
        float drumsVol = 0.8f;
        bool countdown = true;
        bool autoBpm = false;
        float autoMin = 100.0f;
        float autoMax = 140.0f;
        int autoStep = 2;
        int autoEvery = 4;
        bool autoUnitBars = true;
        bool autoLoop = true;
        bool autoReverse = false;
    };

    std::array<ProgramState, 5> programs;
    void syncEngineFromParams();
    void saveCurrentProgram();
    void loadProgram (int index);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MetronomeVSTAudioProcessor)
};
