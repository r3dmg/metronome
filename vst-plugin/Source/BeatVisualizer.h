#pragma once

#include <JuceHeader.h>

class BeatVisualizer : public juce::Component
{
public:
    void setBeats (int total, int current)
    {
        numBeats = juce::jmax (1, total);
        currentBeat = current;
        repaint();
    }

    void paint (juce::Graphics& g) override
    {
        auto bounds = getLocalBounds().toFloat();
        const float w = bounds.getWidth() / (float) numBeats;

        for (int i = 0; i < numBeats; ++i)
        {
            auto seg = bounds.withX (bounds.getX() + w * (float) i).withWidth (w - 2.0f);
            const bool on = i == currentBeat;
            g.setColour (on ? juce::Colour (0xff22c55e) : juce::Colour (0xff334155));
            g.fillRoundedRectangle (seg.reduced (1.0f), 4.0f);
        }
    }

private:
    int numBeats = 4;
    int currentBeat = 0;
};
