#pragma once

#include <JuceHeader.h>

namespace MetronomeTheme
{
inline const juce::Colour bg       { 0xff0f172a };
inline const juce::Colour panel    { 0xff111827 };
inline const juce::Colour inset    { 0xff0b1220 };
inline const juce::Colour border   { 0xff263146 };
inline const juce::Colour text     { 0xffe5e7eb };
inline const juce::Colour subtext  { 0xff9ca3af };
inline const juce::Colour accent   { 0xff22c55e };
inline const juce::Colour accentDark { 0xff052e16 };
inline const juce::Colour secondaryBtn { 0xff4b5563 };
inline const juce::Colour segOff   { 0xff1f2937 };
} // namespace MetronomeTheme

class MetronomeLookAndFeel : public juce::LookAndFeel_V4
{
public:
    MetronomeLookAndFeel()
    {
        setColour (juce::ResizableWindow::backgroundColourId, MetronomeTheme::bg);
        setColour (juce::Label::textColourId, MetronomeTheme::text);
        setColour (juce::ComboBox::backgroundColourId, MetronomeTheme::inset);
        setColour (juce::ComboBox::textColourId, MetronomeTheme::text);
        setColour (juce::ComboBox::outlineColourId, MetronomeTheme::border);
        setColour (juce::PopupMenu::backgroundColourId, MetronomeTheme::inset);
        setColour (juce::PopupMenu::textColourId, MetronomeTheme::text);
        setColour (juce::Slider::backgroundColourId, MetronomeTheme::inset);
        setColour (juce::Slider::trackColourId, MetronomeTheme::border);
        setColour (juce::Slider::thumbColourId, MetronomeTheme::accent);
        setColour (juce::TextButton::buttonColourId, MetronomeTheme::secondaryBtn);
        setColour (juce::TextButton::textColourOffId, MetronomeTheme::text);
        setColour (juce::ToggleButton::textColourId, MetronomeTheme::text);
        setColour (juce::ToggleButton::tickColourId, MetronomeTheme::accent);
        setColour (juce::ToggleButton::tickDisabledColourId, MetronomeTheme::border);
    }

    void drawButtonBackground (juce::Graphics& g, juce::Button& button,
                               const juce::Colour&,
                               bool, bool) override
    {
        auto bounds = button.getLocalBounds().toFloat().reduced (0.5f);
        const bool primary = button.getProperties().getWithDefault ("btnStyle", "") == "primary";
        const bool small = button.getProperties().getWithDefault ("btnStyle", "") == "small";

        auto col = primary ? MetronomeTheme::accent
                           : MetronomeTheme::secondaryBtn;
        if (! button.isEnabled())
            col = col.withMultipliedAlpha (0.45f);
        else if (button.isDown())
            col = col.darker (0.15f);
        else if (button.isOver())
            col = primary ? col.brighter (0.08f) : col.brighter (0.12f);

        g.setColour (col);
        g.fillRoundedRectangle (bounds, small ? 6.0f : 8.0f);
    }

    void drawButtonText (juce::Graphics& g, juce::TextButton& button,
                         bool, bool) override
    {
        const bool primary = button.getProperties().getWithDefault ("btnStyle", "") == "primary";
        g.setColour (primary ? MetronomeTheme::accentDark : MetronomeTheme::text);
        g.setFont (button.getProperties().getWithDefault ("btnStyle", "") == "small" ? 12.0f : 14.0f);
        g.drawText (button.getButtonText(), button.getLocalBounds(), juce::Justification::centred);
    }

};
