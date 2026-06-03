#pragma once

#include "MetronomeTheme.h"
#include <JuceHeader.h>

class BeatVisualizer : public juce::Component
{
public:
    void setBeats (int total, int current, float beatProgress)
    {
        numBeats = juce::jmax (1, total);
        currentBeat = juce::jlimit (0, numBeats - 1, current);
        progress = juce::jlimit (0.0f, 1.0f, beatProgress);
        repaint();
    }

    void paint (juce::Graphics& g) override
    {
        auto bounds = getLocalBounds().toFloat();
        g.setColour (MetronomeTheme::inset);
        g.fillRoundedRectangle (bounds, 8.0f);
        g.setColour (MetronomeTheme::border);
        g.drawRoundedRectangle (bounds.reduced (0.5f), 8.0f, 1.0f);

        auto track = bounds.reduced (6.0f);
        g.setColour (MetronomeTheme::panel);
        g.fillRoundedRectangle (track, 6.0f);

        const float step = track.getWidth() / (float) numBeats;
        const float fillW = step * (float) currentBeat + step * progress;

        if (fillW > 0.0f)
        {
            juce::ColourGradient grad (MetronomeTheme::accent.withAlpha (0.35f), track.getX(), track.getCentreY(),
                                       MetronomeTheme::accent.withAlpha (0.6f), track.getX() + fillW, track.getCentreY(), false);
            g.setGradientFill (grad);
            g.fillRoundedRectangle (track.getX(), track.getY(), fillW, track.getHeight(), 6.0f);
        }

        for (int i = 0; i < numBeats; ++i)
        {
            const float x = track.getX() + step * (float) i;
            if (i > 0)
            {
                g.setColour (MetronomeTheme::segOff);
                g.drawLine (x, track.getY(), x, track.getBottom(), 1.0f);
            }
        }
    }

private:
    int numBeats = 4;
    int currentBeat = 0;
    float progress = 0.0f;
};
