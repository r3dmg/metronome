#include "PluginEditor.h"
#include "PluginIDs.h"

namespace
{
void styleHeaderLabel (juce::Label& l, const juce::String& text)
{
    l.setText (text, juce::dontSendNotification);
    l.setFont (juce::FontOptions (12.0f).withStyle ("Bold"));
    l.setColour (juce::Label::textColourId, MetronomeTheme::subtext);
    l.setJustificationType (juce::Justification::centredLeft);
}

void styleFieldLabel (juce::Label& l, const juce::String& text)
{
    l.setText (text, juce::dontSendNotification);
    l.setFont (juce::FontOptions (11.0f).withStyle ("Bold"));
    l.setColour (juce::Label::textColourId, MetronomeTheme::subtext);
    l.setJustificationType (juce::Justification::centredLeft);
}
} // namespace

MetronomeVSTAudioProcessorEditor::MetronomeVSTAudioProcessorEditor (MetronomeVSTAudioProcessor& p)
    : AudioProcessorEditor (p),
      audioProcessor (p)
{
    setLookAndFeel (&laf);
    setSize (500, 680);
    setWantsKeyboardFocus (true);

    auto& apvts = audioProcessor.getAPVTS();
    activeProfile = audioProcessor.getCurrentProgram();

    titleLabel.setText ("Metronome", juce::dontSendNotification);
    titleLabel.setFont (juce::FontOptions (24.0f));
    titleLabel.setColour (juce::Label::textColourId, MetronomeTheme::text);
    addAndMakeVisible (titleLabel);

    totalTimeLabel.setText ("Total Time: 00:00", juce::dontSendNotification);
    totalTimeLabel.setFont (juce::FontOptions (14.0f));
    totalTimeLabel.setColour (juce::Label::textColourId, MetronomeTheme::subtext);
    totalTimeLabel.setJustificationType (juce::Justification::centredRight);
    addAndMakeVisible (totalTimeLabel);

    for (int i = 0; i < 5; ++i)
    {
        profileTabs[i].setButtonText (juce::String (i + 1));
        profileTabs[i].setClickingTogglesState (false);
        profileTabs[i].onClick = [this, i, &p]()
        {
            p.setCurrentProgram (i);
            activeProfile = i;
            updateProfileTabs();
        };
        addAndMakeVisible (profileTabs[i]);
    }
    updateProfileTabs();

    styleHeaderLabel (bpmHeader, "BPM");
    addAndMakeVisible (bpmHeader);
    bpmSlider.setSliderStyle (juce::Slider::LinearHorizontal);
    bpmSlider.setTextBoxStyle (juce::Slider::TextBoxAbove, false, 72, 22);
    bpmSlider.setColour (juce::Slider::textBoxTextColourId, MetronomeTheme::text);
    bpmSlider.setColour (juce::Slider::textBoxBackgroundColourId, MetronomeTheme::inset);
    bpmSlider.setColour (juce::Slider::textBoxOutlineColourId, MetronomeTheme::border);
    addAndMakeVisible (bpmSlider);
    bpmAttach = std::make_unique<Attachment> (apvts, ParamIDs::bpm, bpmSlider);

    styleHeaderLabel (timeSigHeader, "TIME SIGNATURE");
    addAndMakeVisible (timeSigHeader);
    for (int i = 1; i <= 12; ++i)
        beatsBox.addItem (juce::String (i), i);
    beatsBox.setSelectedId (4);
    addAndMakeVisible (beatsBox);
    beatsAttach = std::make_unique<ComboAttachment> (apvts, ParamIDs::beatsPerBar, beatsBox);

    slashLabel.setText ("/", juce::dontSendNotification);
    slashLabel.setFont (juce::FontOptions (14.0f).withStyle ("Bold"));
    slashLabel.setColour (juce::Label::textColourId, MetronomeTheme::subtext);
    slashLabel.setJustificationType (juce::Justification::centred);
    addAndMakeVisible (slashLabel);

    unitBox.addItemList ({ "2", "4", "8", "16" }, 1);
    unitBox.setSelectedId (2);
    addAndMakeVisible (unitBox);
    unitAttach = std::make_unique<ComboAttachment> (apvts, ParamIDs::beatUnit, unitBox);

    startBtn.setButtonText ("Start");
    startBtn.getProperties().set ("btnStyle", "primary");
    startBtn.onClick = [this]() { audioProcessor.startMetronome(); updateStartStopButtons(); };
    addAndMakeVisible (startBtn);

    stopBtn.setButtonText ("Stop");
    stopBtn.getProperties().set ("btnStyle", "secondary");
    stopBtn.setEnabled (false);
    stopBtn.onClick = [this]() { audioProcessor.stopMetronome(); updateStartStopButtons(); };
    addAndMakeVisible (stopBtn);

    addAndMakeVisible (beatVisualizer);
    nextStepTimer.setText ("---", juce::dontSendNotification);
    nextStepTimer.setFont (juce::FontOptions (16.0f).withStyle ("Bold"));
    nextStepTimer.setColour (juce::Label::textColourId, MetronomeTheme::subtext);
    nextStepTimer.setJustificationType (juce::Justification::centredRight);
    addAndMakeVisible (nextStepTimer);

    countToggle.setButtonText ("Countdown");
    drumsToggle.setButtonText ("Drums");
    hostTempoToggle.setButtonText ("Sync host BPM");
    addAndMakeVisible (countToggle);
    addAndMakeVisible (drumsToggle);
    addAndMakeVisible (hostTempoToggle);
    countAttach = std::make_unique<ButtonAttachment> (apvts, ParamIDs::countdown, countToggle);
    drumsAttach = std::make_unique<ButtonAttachment> (apvts, ParamIDs::drums, drumsToggle);
    hostTempoAttach = std::make_unique<ButtonAttachment> (apvts, ParamIDs::useHostTempo, hostTempoToggle);

    drumsVolLabel.setText ("Volume", juce::dontSendNotification);
    drumsVolLabel.setColour (juce::Label::textColourId, MetronomeTheme::text);
    addAndMakeVisible (drumsVolLabel);
    drumsVolSlider.setSliderStyle (juce::Slider::LinearHorizontal);
    drumsVolSlider.setTextBoxStyle (juce::Slider::NoTextBox, false, 0, 0);
    addAndMakeVisible (drumsVolSlider);
    drumsVolAttach = std::make_unique<Attachment> (apvts, ParamIDs::drumsVol, drumsVolSlider);

    addAndMakeVisible (autoPanel);
    autoHeader.setText ("Auto-BPM", juce::dontSendNotification);
    autoHeader.setFont (juce::FontOptions (16.0f));
    autoHeader.setColour (juce::Label::textColourId, MetronomeTheme::accent);
    autoPanel.addAndMakeVisible (autoHeader);

    autoBpmToggle.setButtonText ("");
    autoPanel.addAndMakeVisible (autoBpmToggle);
    autoBpmAttach = std::make_unique<ButtonAttachment> (apvts, ParamIDs::autoBpm, autoBpmToggle);
    autoBpmToggle.onClick = [this]() { updateAutoPanelEnabled(); };

    autoLoopToggle.setButtonText ("Loop");
    autoReverseToggle.setButtonText ("Reverse");
    autoResetBtn.setButtonText ("Reset");
    autoResetBtn.getProperties().set ("btnStyle", "small");
    autoPanel.addAndMakeVisible (autoLoopToggle);
    autoPanel.addAndMakeVisible (autoReverseToggle);
    autoPanel.addAndMakeVisible (autoResetBtn);
    autoLoopAttach = std::make_unique<ButtonAttachment> (apvts, ParamIDs::autoLoop, autoLoopToggle);
    autoReverseAttach = std::make_unique<ButtonAttachment> (apvts, ParamIDs::autoReverse, autoReverseToggle);
    autoResetBtn.onClick = [this, &apvts]()
    {
        audioProcessor.getEngine().resetAutoState();
        const float minV = *apvts.getRawParameterValue (ParamIDs::autoMin);
        apvts.getParameter (ParamIDs::bpm)->setValueNotifyingHost (
            apvts.getParameterRange (ParamIDs::bpm).convertTo0to1 (minV));
    };

    styleFieldLabel (autoMinLabel, "MIN BPM");
    styleFieldLabel (autoMaxLabel, "MAX BPM");
    styleFieldLabel (autoStepLabel, "STEP");
    styleFieldLabel (autoEveryLabel, "EVERY");
    for (auto* l : { &autoMinLabel, &autoMaxLabel, &autoStepLabel, &autoEveryLabel })
        autoPanel.addAndMakeVisible (*l);

    for (auto* s : { &autoMinSlider, &autoMaxSlider, &autoStepSlider, &autoEverySlider })
    {
        s->setSliderStyle (juce::Slider::LinearHorizontal);
        s->setTextBoxStyle (juce::Slider::TextBoxBelow, false, 64, 18);
        s->setColour (juce::Slider::textBoxTextColourId, MetronomeTheme::text);
        s->setColour (juce::Slider::textBoxBackgroundColourId, MetronomeTheme::inset);
        s->setColour (juce::Slider::textBoxOutlineColourId, MetronomeTheme::border);
        autoPanel.addAndMakeVisible (*s);
    }
    autoMinAttach = std::make_unique<Attachment> (apvts, ParamIDs::autoMin, autoMinSlider);
    autoMaxAttach = std::make_unique<Attachment> (apvts, ParamIDs::autoMax, autoMaxSlider);
    autoStepAttach = std::make_unique<Attachment> (apvts, ParamIDs::autoStep, autoStepSlider);
    autoEveryAttach = std::make_unique<Attachment> (apvts, ParamIDs::autoEvery, autoEverySlider);

    autoUnitBox.addItem ("Bars", 1);
    autoUnitBox.addItem ("Minutes", 2);
    autoUnitBox.onChange = [this, &apvts]()
    {
        const bool bars = autoUnitBox.getSelectedId() == 1;
        apvts.getParameter (ParamIDs::autoUnitBars)->setValueNotifyingHost (bars ? 1.0f : 0.0f);
    };
    autoPanel.addAndMakeVisible (autoUnitBox);
    if (*apvts.getRawParameterValue (ParamIDs::autoUnitBars) > 0.5f)
        autoUnitBox.setSelectedId (1);
    else
        autoUnitBox.setSelectedId (2);

    updateAutoPanelEnabled();
    startTimerHz (30);
}

MetronomeVSTAudioProcessorEditor::~MetronomeVSTAudioProcessorEditor()
{
    setLookAndFeel (nullptr);
}

void MetronomeVSTAudioProcessorEditor::paint (juce::Graphics& g)
{
    g.fillAll (MetronomeTheme::bg);

    auto bounds = getLocalBounds().reduced (12);
    g.setColour (MetronomeTheme::panel);
    g.fillRoundedRectangle (bounds.toFloat(), 12.0f);

    auto drawPanel = [&g] (juce::Rectangle<int> r)
    {
        if (r.isEmpty())
            return;
        g.setColour (MetronomeTheme::inset);
        g.fillRoundedRectangle (r.toFloat(), 10.0f);
        g.setColour (MetronomeTheme::border);
        g.drawRoundedRectangle (r.toFloat().reduced (0.5f), 10.0f, 1.0f);
    };

    drawPanel (vizBounds);
    drawPanel (optionsBounds);
    drawPanel (autoBounds);

    g.setColour (MetronomeTheme::border);
    g.drawHorizontalLine (tabsBottom, (float) bounds.getX() + 12, (float) bounds.getRight() - 12);

    const int cd = audioProcessor.getEngine().getCountdownDisplay();
    if (cd > 0)
    {
        g.setColour (juce::Colour (0xcc000000));
        g.fillRect (getLocalBounds());
        g.setColour (MetronomeTheme::accent);
        g.setFont (juce::FontOptions (120.0f).withStyle ("Bold"));
        g.drawText (juce::String (cd), getLocalBounds(), juce::Justification::centred);
    }
}

void MetronomeVSTAudioProcessorEditor::resized()
{
    auto area = getLocalBounds().reduced (24);

    auto header = area.removeFromTop (32);
    titleLabel.setBounds (header.removeFromLeft (header.getWidth() / 2));
    totalTimeLabel.setBounds (header);

    auto tabs = area.removeFromTop (36);
    tabsBottom = tabs.getBottom();
    const int tw = tabs.getWidth() / 5;
    for (int i = 0; i < 5; ++i)
        profileTabs[i].setBounds (tabs.removeFromLeft (tw).reduced (2, 6));

    area.removeFromTop (8);
    auto controls = area.removeFromTop (168);
    const int colW = controls.getWidth() / 3;

    auto bpmCol = controls.removeFromLeft (colW).reduced (4, 0);
    bpmHeader.setBounds (bpmCol.removeFromTop (18));
    bpmSlider.setBounds (bpmCol);

    auto sigCol = controls.removeFromLeft (colW).reduced (4, 0);
    timeSigHeader.setBounds (sigCol.removeFromTop (18));
    auto sigRow = sigCol.removeFromTop (36);
    beatsBox.setBounds (sigRow.removeFromLeft (sigRow.getWidth() / 2 - 12));
    slashLabel.setBounds (sigRow.removeFromLeft (24));
    unitBox.setBounds (sigRow);

    auto btnCol = controls.reduced (4, 0);
    btnCol.removeFromTop (18);
    auto btnRow = btnCol.removeFromTop (88);
    startBtn.setBounds (btnRow.removeFromTop (40).reduced (0, 2));
    stopBtn.setBounds (btnRow.reduced (0, 2));

    area.removeFromTop (8);
    vizBounds = area.removeFromTop (72).reduced (0, 4);
    auto vizInner = vizBounds.reduced (14, 12);
    nextStepTimer.setBounds (vizInner.removeFromRight (90));
    beatVisualizer.setBounds (vizInner);

    area.removeFromTop (8);
    optionsBounds = area.removeFromTop (52).reduced (0, 4);
    auto opt = optionsBounds.reduced (12, 10);
    countToggle.setBounds (opt.removeFromLeft (108));
    drumsToggle.setBounds (opt.removeFromLeft (80));
    hostTempoToggle.setBounds (opt.removeFromLeft (120));
    drumsVolLabel.setBounds (opt.removeFromLeft (52));
    drumsVolSlider.setBounds (opt);

    area.removeFromTop (8);
    autoBounds = area.reduced (0, 4);
    autoPanel.setBounds (autoBounds);
    layoutAutoPanel();
}

void MetronomeVSTAudioProcessorEditor::layoutAutoPanel()
{
    auto r = autoPanel.getLocalBounds().reduced (14, 12);
    auto head = r.removeFromTop (28);
    autoHeader.setBounds (head.removeFromLeft (100));
    autoBpmToggle.setBounds (head.removeFromLeft (48).withSizeKeepingCentre (46, 22));

    auto ctrl = r.removeFromTop (28);
    autoLoopToggle.setBounds (ctrl.removeFromLeft (70));
    autoReverseToggle.setBounds (ctrl.removeFromLeft (90));
    autoResetBtn.setBounds (ctrl.removeFromRight (72).withHeight (26));

    r.removeFromTop (6);
    const int colW = r.getWidth() / 2;

    auto layoutField = [] (juce::Rectangle<int> col, juce::Label& lbl, juce::Component& field)
    {
        lbl.setBounds (col.removeFromTop (16));
        field.setBounds (col.removeFromTop (44));
    };

    auto c1 = r.removeFromLeft (colW).reduced (4, 0);
    auto c2 = r.reduced (4, 0);

    auto minCol = c1.removeFromTop (64);
    auto stepCol = c1.removeFromTop (64);
    auto maxCol = c2.removeFromTop (64);
    auto everyCol = c2.removeFromTop (64);

    layoutField (minCol, autoMinLabel, autoMinSlider);
    layoutField (stepCol, autoStepLabel, autoStepSlider);
    layoutField (maxCol, autoMaxLabel, autoMaxSlider);
    layoutField (everyCol, autoEveryLabel, autoEverySlider);

    auto unitArea = everyCol.withHeight (28).withWidth (100);
    juce::ignoreUnused (unitArea);
    autoUnitBox.setBounds (autoEverySlider.getBounds().withY (autoEverySlider.getBottom() + 2).withHeight (26).withWidth (100));
}

void MetronomeVSTAudioProcessorEditor::updateStartStopButtons()
{
    const bool running = audioProcessor.isMetronomeRunning();
    startBtn.setEnabled (! running);
    stopBtn.setEnabled (running);
}

void MetronomeVSTAudioProcessorEditor::updateProfileTabs()
{
    for (int i = 0; i < 5; ++i)
    {
        profileTabs[i].setColour (juce::TextButton::textColourOffId,
                                  i == activeProfile ? MetronomeTheme::accent : MetronomeTheme::subtext);
        profileTabs[i].setColour (juce::TextButton::textColourOnId, MetronomeTheme::accent);
    }
}

void MetronomeVSTAudioProcessorEditor::updateAutoPanelEnabled()
{
    const bool en = autoBpmToggle.getToggleState();
    autoPanel.setAlpha (en ? 1.0f : 0.5f);
    autoMinSlider.setEnabled (en);
    autoMaxSlider.setEnabled (en);
    autoStepSlider.setEnabled (en);
    autoEverySlider.setEnabled (en);
    autoLoopToggle.setEnabled (en);
    autoReverseToggle.setEnabled (en);
    autoResetBtn.setEnabled (en);
    autoUnitBox.setEnabled (en);
}

void MetronomeVSTAudioProcessorEditor::timerCallback()
{
    updateStartStopButtons();

    const auto& eng = audioProcessor.getEngine();
    const int beats = (int) *audioProcessor.getAPVTS().getRawParameterValue (ParamIDs::beatsPerBar) + 1;
    beatVisualizer.setBeats (beats, eng.getCurrentBeat(), eng.getBeatProgress());

    const int t = eng.getTotalTimeSec();
    totalTimeLabel.setText ("Total Time: " + juce::String (t / 60).paddedLeft ('0', 2) + ":"
                            + juce::String (t % 60).paddedLeft ('0', 2),
                            juce::dontSendNotification);

    nextStepTimer.setText (eng.getStatusTimerText(), juce::dontSendNotification);
    repaint();
}

bool MetronomeVSTAudioProcessorEditor::keyPressed (const juce::KeyPress& key)
{
    if (key == juce::KeyPress::spaceKey)
    {
        if (audioProcessor.isMetronomeRunning())
            audioProcessor.stopMetronome();
        else
            audioProcessor.startMetronome();
        updateStartStopButtons();
        return true;
    }
    return false;
}
