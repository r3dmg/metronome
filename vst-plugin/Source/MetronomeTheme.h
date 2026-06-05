#pragma once

#include <JuceHeader.h>

// Те же CSS-переменные что в styles.css веб-приложения
namespace MetronomeTheme
{
inline const juce::Colour bg        { 0xff0f172a };
inline const juce::Colour panel     { 0xff111827 };
inline const juce::Colour inset     { 0xff0b1220 };
inline const juce::Colour border    { 0xff263146 };
inline const juce::Colour text      { 0xffe5e7eb };
inline const juce::Colour subtext   { 0xff9ca3af };
inline const juce::Colour accent    { 0xff22c55e };
inline const juce::Colour accentDark{ 0xff052e16 };
inline const juce::Colour secondary { 0xff4b5563 };
inline const juce::Colour segOff    { 0xff1f2937 };
} // namespace MetronomeTheme

class MetronomeLookAndFeel : public juce::LookAndFeel_V4
{
public:
    MetronomeLookAndFeel()
    {
        // Базовые цвета
        setColour (juce::ResizableWindow::backgroundColourId, MetronomeTheme::bg);
        setColour (juce::Label::textColourId,                 MetronomeTheme::text);

        // ComboBox
        setColour (juce::ComboBox::backgroundColourId, MetronomeTheme::inset);
        setColour (juce::ComboBox::textColourId,       MetronomeTheme::text);
        setColour (juce::ComboBox::outlineColourId,    MetronomeTheme::border);
        setColour (juce::ComboBox::arrowColourId,      MetronomeTheme::subtext);
        setColour (juce::PopupMenu::backgroundColourId,MetronomeTheme::inset);
        setColour (juce::PopupMenu::textColourId,      MetronomeTheme::text);
        setColour (juce::PopupMenu::highlightedBackgroundColourId, MetronomeTheme::border);
        setColour (juce::PopupMenu::highlightedTextColourId,       MetronomeTheme::text);

        // Slider
        setColour (juce::Slider::backgroundColourId,        MetronomeTheme::border);
        setColour (juce::Slider::trackColourId,             MetronomeTheme::accent);
        setColour (juce::Slider::thumbColourId,             MetronomeTheme::accent);
        setColour (juce::Slider::textBoxTextColourId,       MetronomeTheme::text);
        setColour (juce::Slider::textBoxBackgroundColourId, MetronomeTheme::inset);
        setColour (juce::Slider::textBoxOutlineColourId,    MetronomeTheme::border);

        // TextButton
        setColour (juce::TextButton::buttonColourId,  MetronomeTheme::secondary);
        setColour (juce::TextButton::textColourOffId, MetronomeTheme::text);
        setColour (juce::TextButton::textColourOnId,  MetronomeTheme::text);

        // ToggleButton (чекбоксы в Options и Auto-BPM)
        setColour (juce::ToggleButton::textColourId,        MetronomeTheme::text);
        setColour (juce::ToggleButton::tickColourId,        MetronomeTheme::accent);
        setColour (juce::ToggleButton::tickDisabledColourId,MetronomeTheme::border);
    }

    // ----- Кнопки (Start / Stop / Reset) -----
    void drawButtonBackground (juce::Graphics& g, juce::Button& button,
                               const juce::Colour&, bool, bool) override
    {
        const juce::String style = button.getProperties()
                                         .getWithDefault ("btnStyle", "").toString();
        const bool primary = (style == "primary");
        const bool small   = (style == "small");

        auto col = primary ? MetronomeTheme::accent : MetronomeTheme::secondary;

        if (! button.isEnabled())      col = col.withMultipliedAlpha (0.45f);
        else if (button.isDown())      col = col.darker (0.15f);
        else if (button.isOver())      col = primary ? col.brighter (0.08f) : col.brighter (0.14f);

        g.setColour (col);
        g.fillRoundedRectangle (button.getLocalBounds().toFloat().reduced (0.5f),
                                small ? 6.0f : 8.0f);
    }

    void drawButtonText (juce::Graphics& g, juce::TextButton& button, bool, bool) override
    {
        const juce::String style = button.getProperties()
                                         .getWithDefault ("btnStyle", "").toString();
        const bool primary = (style == "primary");
        const bool small   = (style == "small");

        g.setColour (! button.isEnabled()
                       ? MetronomeTheme::subtext
                       : primary ? MetronomeTheme::accentDark : MetronomeTheme::text);
        g.setFont (small ? 12.0f : 14.0f);
        g.drawText (button.getButtonText(),
                    button.getLocalBounds(), juce::Justification::centred);
    }

    // ----- Вкладки профилей (TextButton без btnStyle) -----
    // Активная вкладка получает цветную нижнюю линию — как border-bottom в веб-версии
    void drawTabButton (juce::TabBarButton&, juce::Graphics&, bool, bool) override {}

    // ----- ComboBox: скруглённые углы, как в веб-версии -----
    void drawComboBox (juce::Graphics& g, int w, int h, bool,
                       int, int, int, int, juce::ComboBox& box) override
    {
        g.setColour (MetronomeTheme::inset);
        g.fillRoundedRectangle (0.f, 0.f, (float)w, (float)h, 8.f);
        g.setColour (box.hasKeyboardFocus (false) ? MetronomeTheme::accent
                                                  : MetronomeTheme::border);
        g.drawRoundedRectangle (0.5f, 0.5f, w - 1.f, h - 1.f, 8.f, 1.f);

        // Стрелка вниз
        const float arrowX = w - 18.f;
        const float arrowY = h * 0.5f;
        juce::Path arrow;
        arrow.addTriangle (arrowX, arrowY - 3.f,
                           arrowX + 7.f, arrowY - 3.f,
                           arrowX + 3.5f, arrowY + 3.f);
        g.setColour (MetronomeTheme::subtext);
        g.fillPath (arrow);
    }

    void positionComboBoxText (juce::ComboBox& box, juce::Label& label) override
    {
        label.setBounds (8, 1, box.getWidth() - 24, box.getHeight() - 2);
        label.setFont (juce::FontOptions (13.0f));
    }

    // ----- Слайдер: трек и ползунок -----
    void drawLinearSlider (juce::Graphics& g, int x, int y, int w, int h,
                           float pos, float, float,
                           juce::Slider::SliderStyle style,
                           juce::Slider& slider) override
    {
        if (style != juce::Slider::LinearHorizontal)
        {
            LookAndFeel_V4::drawLinearSlider (g, x, y, w, h, pos, 0, 0, style, slider);
            return;
        }

        const float trackH  = 4.f;
        const float trackY  = y + (h - trackH) * 0.5f;
        const float thumbR  = 7.f;

        // Фон трека
        g.setColour (MetronomeTheme::border);
        g.fillRoundedRectangle ((float)x, trackY, (float)w, trackH, trackH * 0.5f);

        // Заполнение до ползунка (акцентный цвет)
        if (pos > x)
        {
            g.setColour (MetronomeTheme::accent);
            g.fillRoundedRectangle ((float)x, trackY, pos - x, trackH, trackH * 0.5f);
        }

        // Ползунок
        g.setColour (MetronomeTheme::accent);
        g.fillEllipse (pos - thumbR, y + (h - thumbR * 2) * 0.5f, thumbR * 2, thumbR * 2);
    }
};
