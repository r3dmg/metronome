#pragma once

#include <JuceHeader.h>
#include <array>

/** Embedded drum WAVs (Accelonome samples from the web app). */
class SampleBank
{
public:
    enum class Sample
    {
        kick,
        snare,
        hatClosed,
        hatOpen,
        crash,
        tom,
        clickNormal,
        clickAccent,
        count
    };

    void prepare (double hostSampleRate);
    bool isLoaded() const { return loaded; }

    const juce::AudioBuffer<float>* getBuffer (Sample s) const;
    double getSourceSampleRate (Sample s) const;

private:
    bool loaded = false;
    double hostRate = 44100.0;
    std::array<std::unique_ptr<juce::AudioBuffer<float>>, (size_t) Sample::count> buffers;
    std::array<double, (size_t) Sample::count> sourceRates {};

    bool loadFromMemory (Sample s, const void* data, int dataSize);
};
