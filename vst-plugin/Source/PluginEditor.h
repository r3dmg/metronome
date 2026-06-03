#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"
#include "BeatVisualizer.h"

class MetronomeVSTAudioProcessorEditor : public juce::AudioProcessorEditor,
                                         private juce::Timer
{
public:
    explicit MetronomeVSTAudioProcessorEditor (MetronomeVSTAudioProcessor&);
    ~MetronomeVSTAudioProcessorEditor() override = default;

    void paint (juce::Graphics&) override;
    void resized() override;

private:
    MetronomeVSTAudioProcessor& processor;

    juce::Slider bpmSlider;
    juce::Label bpmLabel;
    juce::ComboBox beatsBox;
    juce::ComboBox unitBox;
    juce::ToggleButton drumsToggle;
    juce::Slider drumsVolSlider;
    juce::ToggleButton hostTempoToggle;
    juce::ToggleButton countdownToggle;
    juce::ToggleButton autoBpmToggle;
    juce::Slider autoMinSlider, autoMaxSlider, autoStepSlider, autoEverySlider;
    juce::ToggleButton autoLoopToggle, autoReverseToggle, autoUnitBarsToggle;
    juce::TextButton profileButtons[5];
    BeatVisualizer beatVisualizer;

    using Attachment = juce::AudioProcessorValueTreeState::SliderAttachment;
    using ButtonAttachment = juce::AudioProcessorValueTreeState::ButtonAttachment;
    using ComboAttachment = juce::AudioProcessorValueTreeState::ComboBoxAttachment;

    std::unique_ptr<Attachment> bpmAttach, drumsVolAttach;
    std::unique_ptr<Attachment> autoMinAttach, autoMaxAttach, autoStepAttach, autoEveryAttach;
    std::unique_ptr<ButtonAttachment> drumsAttach, hostTempoAttach, countdownAttach, autoBpmAttach;
    std::unique_ptr<ButtonAttachment> autoLoopAttach, autoReverseAttach, autoUnitBarsAttach;
    std::unique_ptr<ComboAttachment> beatsAttach, unitAttach;

    void timerCallback() override;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MetronomeVSTAudioProcessorEditor)
};
