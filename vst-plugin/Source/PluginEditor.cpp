#include "PluginEditor.h"
#include "PluginIDs.h"

MetronomeVSTAudioProcessorEditor::MetronomeVSTAudioProcessorEditor (MetronomeVSTAudioProcessor& p)
    : AudioProcessorEditor (p),
      processor (p)
{
    setSize (480, 548);

    auto& apvts = processor.getAPVTS();

    bpmLabel.setText ("BPM", juce::dontSendNotification);
    bpmLabel.setJustificationType (juce::Justification::centred);
    addAndMakeVisible (bpmLabel);

    bpmSlider.setSliderStyle (juce::Slider::LinearHorizontal);
    bpmSlider.setTextBoxStyle (juce::Slider::TextBoxBelow, false, 60, 18);
    addAndMakeVisible (bpmSlider);
    bpmAttach = std::make_unique<Attachment> (apvts, ParamIDs::bpm, bpmSlider);

    for (int i = 1; i <= 12; ++i)
        beatsBox.addItem (juce::String (i), i);
    beatsBox.setSelectedId (4);
    addAndMakeVisible (beatsBox);
    beatsAttach = std::make_unique<ComboAttachment> (apvts, ParamIDs::beatsPerBar, beatsBox);

    unitBox.addItemList ({ "2", "4", "8", "16" }, 1);
    unitBox.setSelectedId (2);
    addAndMakeVisible (unitBox);
    unitAttach = std::make_unique<ComboAttachment> (apvts, ParamIDs::beatUnit, unitBox);

    drumsToggle.setButtonText ("Drums");
    addAndMakeVisible (drumsToggle);
    drumsAttach = std::make_unique<ButtonAttachment> (apvts, ParamIDs::drums, drumsToggle);

    drumsVolSlider.setSliderStyle (juce::Slider::LinearHorizontal);
    drumsVolSlider.setTextBoxStyle (juce::Slider::TextBoxRight, false, 50, 18);
    addAndMakeVisible (drumsVolSlider);
    drumsVolAttach = std::make_unique<Attachment> (apvts, ParamIDs::drumsVol, drumsVolSlider);

    hostTempoToggle.setButtonText ("Sync FL tempo");
    addAndMakeVisible (hostTempoToggle);
    hostTempoAttach = std::make_unique<ButtonAttachment> (apvts, ParamIDs::useHostTempo, hostTempoToggle);

    countdownToggle.setButtonText ("Countdown 3s");
    addAndMakeVisible (countdownToggle);
    countdownAttach = std::make_unique<ButtonAttachment> (apvts, ParamIDs::countdown, countdownToggle);

    autoBpmToggle.setButtonText ("Auto-BPM");
    addAndMakeVisible (autoBpmToggle);
    autoBpmAttach = std::make_unique<ButtonAttachment> (apvts, ParamIDs::autoBpm, autoBpmToggle);

    for (auto* s : { &autoMinSlider, &autoMaxSlider, &autoStepSlider, &autoEverySlider })
    {
        s->setSliderStyle (juce::Slider::LinearHorizontal);
        s->setTextBoxStyle (juce::Slider::TextBoxRight, false, 50, 18);
        addAndMakeVisible (*s);
    }
    autoMinAttach = std::make_unique<Attachment> (apvts, ParamIDs::autoMin, autoMinSlider);
    autoMaxAttach = std::make_unique<Attachment> (apvts, ParamIDs::autoMax, autoMaxSlider);
    autoStepAttach = std::make_unique<Attachment> (apvts, ParamIDs::autoStep, autoStepSlider);
    autoEveryAttach = std::make_unique<Attachment> (apvts, ParamIDs::autoEvery, autoEverySlider);

    autoLoopToggle.setButtonText ("Loop");
    autoReverseToggle.setButtonText ("Reverse");
    autoUnitBarsToggle.setButtonText ("Every N bars");
    addAndMakeVisible (autoLoopToggle);
    addAndMakeVisible (autoReverseToggle);
    addAndMakeVisible (autoUnitBarsToggle);
    autoLoopAttach = std::make_unique<ButtonAttachment> (apvts, ParamIDs::autoLoop, autoLoopToggle);
    autoReverseAttach = std::make_unique<ButtonAttachment> (apvts, ParamIDs::autoReverse, autoReverseToggle);
    autoUnitBarsAttach = std::make_unique<ButtonAttachment> (apvts, ParamIDs::autoUnitBars, autoUnitBarsToggle);

    for (int i = 0; i < 5; ++i)
    {
        profileButtons[i].setButtonText (juce::String (i + 1));
        profileButtons[i].onClick = [this, i, &p]()
        {
            p.setCurrentProgram (i);
        };
        addAndMakeVisible (profileButtons[i]);
    }

    addAndMakeVisible (beatVisualizer);
    startTimerHz (30);
}

void MetronomeVSTAudioProcessorEditor::paint (juce::Graphics& g)
{
    g.fillAll (juce::Colour (0xff1a1f2e));
    g.setColour (juce::Colours::white);
    g.setFont (18.0f);
    g.drawText ("Metronome", getLocalBounds().removeFromTop (36), juce::Justification::centred);

    g.setFont (12.0f);
    g.drawText ("Time signature", 16, 88, 120, 20, juce::Justification::centredLeft);
    g.drawText ("Auto min / max / step / every", 16, 368, 280, 20, juce::Justification::centredLeft);

    const int cd = processor.getEngine().getCountdownDisplay();
    if (cd > 0)
    {
        g.setColour (juce::Colour (0xcc000000));
        g.fillRect (getLocalBounds());
        g.setColour (juce::Colours::white);
        g.setFont (96.0f);
        g.drawText (juce::String (cd), getLocalBounds(), juce::Justification::centred);
    }
}

void MetronomeVSTAudioProcessorEditor::resized()
{
    auto area = getLocalBounds().reduced (16);
    area.removeFromTop (40);

    auto profiles = area.removeFromTop (32);
    const int pw = profiles.getWidth() / 5;
    for (int i = 0; i < 5; ++i)
        profileButtons[i].setBounds (profiles.removeFromLeft (pw).reduced (2));

    bpmLabel.setBounds (area.removeFromTop (20));
    bpmSlider.setBounds (area.removeFromTop (48));
    area.removeFromTop (8);

    auto sigRow = area.removeFromTop (32);
    beatsBox.setBounds (sigRow.removeFromLeft (80));
    sigRow.removeFromLeft (8);
    unitBox.setBounds (sigRow.removeFromLeft (80));

    beatVisualizer.setBounds (area.removeFromTop (48).reduced (0, 8));
    area.removeFromTop (8);

    drumsToggle.setBounds (area.removeFromTop (28));
    drumsVolSlider.setBounds (area.removeFromTop (32));
    hostTempoToggle.setBounds (area.removeFromTop (28));
    countdownToggle.setBounds (area.removeFromTop (28));
    autoBpmToggle.setBounds (area.removeFromTop (28));

    autoMinSlider.setBounds (area.removeFromTop (32));
    autoMaxSlider.setBounds (area.removeFromTop (32));
    autoStepSlider.setBounds (area.removeFromTop (32));
    autoEverySlider.setBounds (area.removeFromTop (32));

    auto row = area.removeFromTop (28);
    autoLoopToggle.setBounds (row.removeFromLeft (80));
    autoReverseToggle.setBounds (row.removeFromLeft (100));
    autoUnitBarsToggle.setBounds (row);
}

void MetronomeVSTAudioProcessorEditor::timerCallback()
{
    const int beats = (int) *processor.getAPVTS().getRawParameterValue (ParamIDs::beatsPerBar) + 1;
    beatVisualizer.setBeats (beats, processor.getEngine().getCurrentBeat());
    repaint();
}
