#include "PluginEditor.h"
#include "PluginIDs.h"

namespace
{
void styleHeaderLabel (juce::Label& l, const juce::String& text)
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
    setSize (500, 730);
    setWantsKeyboardFocus (true);

    auto& apvts = audioProcessor.getAPVTS();
    activeProfile = audioProcessor.getCurrentProgram();

    // --- Заголовок ---
    titleLabel.setText ("Metronome", juce::dontSendNotification);
    titleLabel.setFont (juce::FontOptions (22.0f).withStyle ("Bold"));
    titleLabel.setColour (juce::Label::textColourId, MetronomeTheme::text);
    addAndMakeVisible (titleLabel);

    totalTimeLabel.setText ("00:00", juce::dontSendNotification);
    totalTimeLabel.setFont (juce::FontOptions (13.0f));
    totalTimeLabel.setColour (juce::Label::textColourId, MetronomeTheme::subtext);
    totalTimeLabel.setJustificationType (juce::Justification::centredRight);
    addAndMakeVisible (totalTimeLabel);

    // --- Вкладки профилей ---
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

    // --- BPM ---
    styleHeaderLabel (bpmHeader, "BPM");
    addAndMakeVisible (bpmHeader);
    bpmSlider.setSliderStyle (juce::Slider::LinearHorizontal);
    bpmSlider.setTextBoxStyle (juce::Slider::TextBoxAbove, false, 80, 24);
    bpmSlider.setColour (juce::Slider::textBoxTextColourId, MetronomeTheme::text);
    bpmSlider.setColour (juce::Slider::textBoxBackgroundColourId, MetronomeTheme::inset);
    bpmSlider.setColour (juce::Slider::textBoxOutlineColourId, MetronomeTheme::border);
    addAndMakeVisible (bpmSlider);
    bpmAttach = std::make_unique<Attachment> (apvts, ParamIDs::bpm, bpmSlider);

    // --- Размер такта ---
    styleHeaderLabel (timeSigHeader, "TIME SIGNATURE");
    addAndMakeVisible (timeSigHeader);
    for (int i = 1; i <= 12; ++i)
        beatsBox.addItem (juce::String (i), i);
    beatsBox.setSelectedId (4);
    addAndMakeVisible (beatsBox);
    beatsAttach = std::make_unique<ComboAttachment> (apvts, ParamIDs::beatsPerBar, beatsBox);

    slashLabel.setText ("/", juce::dontSendNotification);
    slashLabel.setFont (juce::FontOptions (16.0f).withStyle ("Bold"));
    slashLabel.setColour (juce::Label::textColourId, MetronomeTheme::subtext);
    slashLabel.setJustificationType (juce::Justification::centred);
    addAndMakeVisible (slashLabel);

    unitBox.addItemList ({ "2", "4", "8", "16" }, 1);
    unitBox.setSelectedId (2);
    addAndMakeVisible (unitBox);
    unitAttach = std::make_unique<ComboAttachment> (apvts, ParamIDs::beatUnit, unitBox);

    // --- Кнопки Start / Stop ---
    startBtn.setButtonText ("Start");
    startBtn.getProperties().set ("btnStyle", "primary");
    startBtn.onClick = [this]() { audioProcessor.startMetronome(); updateStartStopButtons(); };
    addAndMakeVisible (startBtn);

    stopBtn.setButtonText ("Stop");
    stopBtn.getProperties().set ("btnStyle", "secondary");
    stopBtn.setEnabled (false);
    stopBtn.onClick = [this]() { audioProcessor.stopMetronome(); updateStartStopButtons(); };
    addAndMakeVisible (stopBtn);

    // --- Визуализатор ---
    addAndMakeVisible (beatVisualizer);
    nextStepTimer.setText ("---", juce::dontSendNotification);
    nextStepTimer.setFont (juce::FontOptions (14.0f).withStyle ("Bold"));
    nextStepTimer.setColour (juce::Label::textColourId, MetronomeTheme::subtext);
    nextStepTimer.setJustificationType (juce::Justification::centredRight);
    addAndMakeVisible (nextStepTimer);

    // --- Опции ---
    countToggle.setButtonText ("Countdown");
    drumsToggle.setButtonText ("Drums");
    hostTempoToggle.setButtonText ("Sync host BPM");
    addAndMakeVisible (countToggle);
    addAndMakeVisible (drumsToggle);
    addAndMakeVisible (hostTempoToggle);
    countAttach    = std::make_unique<ButtonAttachment> (apvts, ParamIDs::countdown,    countToggle);
    drumsAttach    = std::make_unique<ButtonAttachment> (apvts, ParamIDs::drums,        drumsToggle);
    hostTempoAttach= std::make_unique<ButtonAttachment> (apvts, ParamIDs::useHostTempo, hostTempoToggle);

    drumsVolLabel.setText ("Volume", juce::dontSendNotification);
    drumsVolLabel.setFont (juce::FontOptions (13.0f));
    drumsVolLabel.setColour (juce::Label::textColourId, MetronomeTheme::subtext);
    addAndMakeVisible (drumsVolLabel);
    drumsVolSlider.setSliderStyle (juce::Slider::LinearHorizontal);
    drumsVolSlider.setTextBoxStyle (juce::Slider::NoTextBox, false, 0, 0);
    addAndMakeVisible (drumsVolSlider);
    drumsVolAttach = std::make_unique<Attachment> (apvts, ParamIDs::drumsVol, drumsVolSlider);

    // --- Auto-BPM ---
    addAndMakeVisible (autoPanel);
    autoHeader.setText ("Auto-BPM", juce::dontSendNotification);
    autoHeader.setFont (juce::FontOptions (15.0f).withStyle ("Bold"));
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
    autoLoopAttach    = std::make_unique<ButtonAttachment> (apvts, ParamIDs::autoLoop,    autoLoopToggle);
    autoReverseAttach = std::make_unique<ButtonAttachment> (apvts, ParamIDs::autoReverse, autoReverseToggle);
    autoResetBtn.onClick = [this, &apvts]()
    {
        audioProcessor.getEngine().resetAutoState();
        const float minV = *apvts.getRawParameterValue (ParamIDs::autoMin);
        apvts.getParameter (ParamIDs::bpm)->setValueNotifyingHost (
            apvts.getParameterRange (ParamIDs::bpm).convertTo0to1 (minV));
    };

    // Подписи Auto-BPM полей (явно, без initializer_list — совместимо с MSVC)
    auto styleAutoLabel = [this] (juce::Label& lbl, const juce::String& txt)
    {
        lbl.setText (txt, juce::dontSendNotification);
        lbl.setFont (juce::FontOptions (10.0f).withStyle ("Bold"));
        lbl.setColour (juce::Label::textColourId, MetronomeTheme::subtext);
        autoPanel.addAndMakeVisible (lbl);
    };
    styleAutoLabel (autoMinLabel,   "MIN BPM");
    styleAutoLabel (autoMaxLabel,   "MAX BPM");
    styleAutoLabel (autoStepLabel,  "STEP");
    styleAutoLabel (autoEveryLabel, "EVERY");

    for (auto* s : { &autoMinSlider, &autoMaxSlider, &autoStepSlider, &autoEverySlider })
    {
        s->setSliderStyle (juce::Slider::LinearHorizontal);
        s->setTextBoxStyle (juce::Slider::TextBoxBelow, false, 60, 18);
        s->setColour (juce::Slider::textBoxTextColourId, MetronomeTheme::text);
        s->setColour (juce::Slider::textBoxBackgroundColourId, MetronomeTheme::inset);
        s->setColour (juce::Slider::textBoxOutlineColourId, MetronomeTheme::border);
        autoPanel.addAndMakeVisible (*s);
    }
    autoMinAttach   = std::make_unique<Attachment> (apvts, ParamIDs::autoMin,   autoMinSlider);
    autoMaxAttach   = std::make_unique<Attachment> (apvts, ParamIDs::autoMax,   autoMaxSlider);
    autoStepAttach  = std::make_unique<Attachment> (apvts, ParamIDs::autoStep,  autoStepSlider);
    autoEveryAttach = std::make_unique<Attachment> (apvts, ParamIDs::autoEvery, autoEverySlider);

    autoUnitBox.addItem ("Bars",    1);
    autoUnitBox.addItem ("Minutes", 2);
    autoUnitBox.onChange = [this, &apvts]()
    {
        const bool bars = autoUnitBox.getSelectedId() == 1;
        apvts.getParameter (ParamIDs::autoUnitBars)->setValueNotifyingHost (bars ? 1.0f : 0.0f);
    };
    autoPanel.addAndMakeVisible (autoUnitBox);
    autoUnitBox.setSelectedId (*apvts.getRawParameterValue (ParamIDs::autoUnitBars) > 0.5f ? 1 : 2);

    updateAutoPanelEnabled();
    startTimerHz (30);
}

MetronomeVSTAudioProcessorEditor::~MetronomeVSTAudioProcessorEditor()
{
    setLookAndFeel (nullptr);
}

void MetronomeVSTAudioProcessorEditor::paint (juce::Graphics& g)
{
    // Фон окна
    g.fillAll (MetronomeTheme::bg);

    // Основная карточка (как .container в веб-приложении)
    auto card = getLocalBounds().reduced (10);
    g.setColour (MetronomeTheme::panel);
    g.fillRoundedRectangle (card.toFloat(), 12.0f);

    // Линия под вкладками (как border-bottom у .tabs)
    g.setColour (MetronomeTheme::border);
    g.drawHorizontalLine (tabsBottom,
                          (float) card.getX() + 10,
                          (float) card.getRight() - 10);

    // Секции с тёмным инсетом (visualizer, options, auto-bpm)
    auto drawSection = [&g] (juce::Rectangle<int> r)
    {
        if (r.isEmpty()) return;
        g.setColour (MetronomeTheme::inset);
        g.fillRoundedRectangle (r.toFloat(), 10.0f);
        g.setColour (MetronomeTheme::border);
        g.drawRoundedRectangle (r.toFloat().reduced (0.5f), 10.0f, 1.0f);
    };

    drawSection (vizBounds);
    drawSection (optionsBounds);
    drawSection (autoBounds);

    // Оверлей отсчёта
    const int cd = audioProcessor.getEngine().getCountdownDisplay();
    if (cd > 0)
    {
        g.setColour (juce::Colour (0xdd0f172a));
        g.fillRect (getLocalBounds());
        g.setColour (MetronomeTheme::accent);
        g.setFont (juce::FontOptions (120.0f).withStyle ("Bold"));
        g.drawText (juce::String (cd), getLocalBounds(), juce::Justification::centred);
    }
}

void MetronomeVSTAudioProcessorEditor::resized()
{
    const int pad = 20;
    auto area = getLocalBounds().reduced (pad);

    // ----- Header -----
    auto header = area.removeFromTop (28);
    titleLabel.setBounds (header.removeFromLeft (200));
    totalTimeLabel.setBounds (header);

    area.removeFromTop (12);

    // ----- Вкладки -----
    auto tabRow = area.removeFromTop (36);
    tabsBottom = tabRow.getBottom() + 4;
    const int tw = tabRow.getWidth() / 5;
    for (int i = 0; i < 5; ++i)
        profileTabs[i].setBounds (tabRow.removeFromLeft (tw).reduced (2, 5));

    area.removeFromTop (14);

    // ----- Блок управления: 3 колонки -----
    auto controls = area.removeFromTop (130);
    const int colW = controls.getWidth() / 3;

    // Колонка 1: BPM (label + slider с TextBoxAbove)
    auto bpmCol = controls.removeFromLeft (colW).reduced (4, 0);
    bpmHeader.setBounds (bpmCol.removeFromTop (16));
    bpmSlider.setBounds (bpmCol);

    // Колонка 2: Размер такта (label + два комбобокса)
    auto sigCol = controls.removeFromLeft (colW).reduced (4, 0);
    timeSigHeader.setBounds (sigCol.removeFromTop (16));
    auto sigRow = sigCol.removeFromTop (34);
    const int halfW = (sigRow.getWidth() - 20) / 2;
    beatsBox.setBounds (sigRow.removeFromLeft (halfW));
    slashLabel.setBounds (sigRow.removeFromLeft (20));
    unitBox.setBounds (sigRow);

    // Колонка 3: Кнопки Start / Stop
    auto btnCol = controls.reduced (4, 0);
    btnCol.removeFromTop (16);
    startBtn.setBounds (btnCol.removeFromTop (44).reduced (0, 2));
    stopBtn.setBounds  (btnCol.removeFromTop (44).reduced (0, 2));

    area.removeFromTop (12);

    // ----- Визуализатор -----
    vizBounds = area.removeFromTop (60).reduced (0, 2);
    auto vizInner = vizBounds.reduced (12, 10);
    nextStepTimer.setBounds (vizInner.removeFromRight (80));
    beatVisualizer.setBounds (vizInner);

    area.removeFromTop (10);

    // ----- Опции -----
    optionsBounds = area.removeFromTop (46).reduced (0, 2);
    auto opt = optionsBounds.reduced (12, 8);
    countToggle.setBounds    (opt.removeFromLeft (110));
    drumsToggle.setBounds    (opt.removeFromLeft (80));
    hostTempoToggle.setBounds(opt.removeFromLeft (130));
    drumsVolLabel.setBounds  (opt.removeFromLeft (52));
    drumsVolSlider.setBounds (opt);

    area.removeFromTop (10);

    // ----- Auto-BPM -----
    autoBounds = area.reduced (0, 2);
    autoPanel.setBounds (autoBounds);
    layoutAutoPanel();
}

void MetronomeVSTAudioProcessorEditor::layoutAutoPanel()
{
    auto r = autoPanel.getLocalBounds().reduced (14, 12);

    // Строка заголовка: "Auto-BPM" + переключатель
    auto headRow = r.removeFromTop (26);
    autoHeader.setBounds (headRow.removeFromLeft (100));
    autoBpmToggle.setBounds (headRow.removeFromLeft (46).withSizeKeepingCentre (44, 22));

    r.removeFromTop (8);

    // Строка управления: Loop, Reverse, Reset
    auto ctrlRow = r.removeFromTop (26);
    autoLoopToggle.setBounds    (ctrlRow.removeFromLeft (70));
    autoReverseToggle.setBounds (ctrlRow.removeFromLeft (90));
    autoResetBtn.setBounds      (ctrlRow.removeFromRight (64).withHeight (24));

    r.removeFromTop (10);

    // Сетка 2×2: Min, Max, Step, Every
    const int colW = r.getWidth() / 2;
    auto left  = r.removeFromLeft (colW).reduced (4, 0);
    auto right = r.reduced (4, 0);

    auto layoutField = [] (juce::Rectangle<int> col, juce::Label& lbl, juce::Component& fld)
    {
        lbl.setBounds (col.removeFromTop (14));
        fld.setBounds (col.removeFromTop (48));
    };

    layoutField (left.removeFromTop  (64), autoMinLabel,   autoMinSlider);
    layoutField (left.removeFromTop  (64), autoStepLabel,  autoStepSlider);
    layoutField (right.removeFromTop (64), autoMaxLabel,   autoMaxSlider);

    // "Every" + unit selector
    auto everyTop = right.removeFromTop (64);
    autoEveryLabel.setBounds (everyTop.removeFromTop (14));
    autoEverySlider.setBounds (everyTop.removeFromTop (48));
    autoUnitBox.setBounds (autoEverySlider.getBounds()
                               .withY      (autoEverySlider.getBottom() + 4)
                               .withHeight (24)
                               .withWidth  (100));
}

void MetronomeVSTAudioProcessorEditor::updateStartStopButtons()
{
    const bool running = audioProcessor.isMetronomeRunning();
    startBtn.setEnabled (! running);
    stopBtn.setEnabled  (running);
}

void MetronomeVSTAudioProcessorEditor::updateProfileTabs()
{
    for (int i = 0; i < 5; ++i)
    {
        profileTabs[i].setColour (juce::TextButton::textColourOffId,
                                  i == activeProfile ? MetronomeTheme::accent
                                                     : MetronomeTheme::subtext);
        profileTabs[i].setColour (juce::TextButton::textColourOnId, MetronomeTheme::accent);
    }
}

void MetronomeVSTAudioProcessorEditor::updateAutoPanelEnabled()
{
    const bool en = autoBpmToggle.getToggleState();
    autoPanel.setAlpha (en ? 1.0f : 0.5f);
    // Явное перечисление — MSVC не выводит тип из смешанного initializer_list
    autoMinSlider   .setEnabled (en);
    autoMaxSlider   .setEnabled (en);
    autoStepSlider  .setEnabled (en);
    autoEverySlider .setEnabled (en);
    autoLoopToggle  .setEnabled (en);
    autoReverseToggle.setEnabled (en);
    autoResetBtn    .setEnabled (en);
    autoUnitBox     .setEnabled (en);
}

void MetronomeVSTAudioProcessorEditor::timerCallback()
{
    updateStartStopButtons();

    const auto& eng   = audioProcessor.getEngine();
    const int beats = (int) *audioProcessor.getAPVTS().getRawParameterValue (ParamIDs::beatsPerBar) + 1;
// currentBeat — это СЛЕДУЮЩИЙ бит для планировщика.
// Для visualizer нужен ТЕКУЩИЙ (тот что сейчас звучит) — он на 1 меньше.
const int displayBeat = (eng.getCurrentBeat() > 0)
                            ? eng.getCurrentBeat() - 1
                            : beats - 1;
beatVisualizer.setBeats (beats, displayBeat, eng.getBeatProgress());

    const int t = eng.getTotalTimeSec();
    totalTimeLabel.setText (juce::String (t / 60).paddedLeft ('0', 2) + ":"
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
