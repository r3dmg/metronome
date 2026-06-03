#pragma once

#include "PluginProcessor.h"
#include "BeatVisualizer.h"
#include "MetronomeTheme.h"
#include <JuceHeader.h>

class MetronomeVSTAudioProcessorEditor : public juce::AudioProcessorEditor,
                                         private juce::Timer
{
public:
    explicit MetronomeVSTAudioProcessorEditor (MetronomeVSTAudioProcessor&);
    ~MetronomeVSTAudioProcessorEditor() override;

    void paint (juce::Graphics&) override;
    void resized() override;
    bool keyPressed (const juce::KeyPress& key) override;

private:
    MetronomeVSTAudioProcessor& audioProcessor;
    MetronomeLookAndFeel laf;

    juce::Label titleLabel;
    juce::Label totalTimeLabel;
    juce::TextButton profileTabs[5];
    int activeProfile = 0;

    juce::Label bpmHeader;
    juce::Slider bpmSlider;
    juce::Label timeSigHeader;
    juce::ComboBox beatsBox;
    juce::Label slashLabel;
    juce::ComboBox unitBox;
    juce::TextButton startBtn;
    juce::TextButton stopBtn;

    BeatVisualizer beatVisualizer;
    juce::Label nextStepTimer;

    juce::ToggleButton countToggle;
    juce::ToggleButton drumsToggle;
    juce::Label drumsVolLabel;
    juce::Slider drumsVolSlider;
    juce::ToggleButton hostTempoToggle;

    juce::Component autoPanel;
    juce::Label autoHeader;
    juce::ToggleButton autoBpmToggle;
    juce::ToggleButton autoLoopToggle;
    juce::ToggleButton autoReverseToggle;
    juce::TextButton autoResetBtn;
    juce::Label autoMinLabel, autoMaxLabel, autoStepLabel, autoEveryLabel;
    juce::Slider autoMinSlider, autoMaxSlider, autoStepSlider, autoEverySlider;
    juce::ComboBox autoUnitBox;

    juce::Rectangle<int> vizBounds, optionsBounds, autoBounds;
    int tabsBottom = 0;

    using Attachment = juce::AudioProcessorValueTreeState::SliderAttachment;
    using ButtonAttachment = juce::AudioProcessorValueTreeState::ButtonAttachment;
    using ComboAttachment = juce::AudioProcessorValueTreeState::ComboBoxAttachment;

    std::unique_ptr<Attachment> bpmAttach, drumsVolAttach;
    std::unique_ptr<Attachment> autoMinAttach, autoMaxAttach, autoStepAttach, autoEveryAttach;
    std::unique_ptr<ButtonAttachment> countAttach, drumsAttach, hostTempoAttach, autoBpmAttach;
    std::unique_ptr<ButtonAttachment> autoLoopAttach, autoReverseAttach;
    std::unique_ptr<ComboAttachment> beatsAttach, unitAttach;

    void layoutAutoPanel();
    void updateStartStopButtons();
    void updateProfileTabs();
    void updateAutoPanelEnabled();
    void timerCallback() override;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MetronomeVSTAudioProcessorEditor)
};
