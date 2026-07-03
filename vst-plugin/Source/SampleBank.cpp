#include "SampleBank.h"
#include "BinaryData.h"

void SampleBank::prepare (double hostSampleRate)
{
    hostRate = hostSampleRate;

    if (loaded)
        return;

    loaded = loadFromMemory (Sample::kick, BinaryData::kick_wav, BinaryData::kick_wavSize)
          && loadFromMemory (Sample::snare, BinaryData::snare_wav, BinaryData::snare_wavSize)
          && loadFromMemory (Sample::hatClosed, BinaryData::hihat_closed_wav, BinaryData::hihat_closed_wavSize)
          && loadFromMemory (Sample::hatOpen, BinaryData::hihat_open_wav, BinaryData::hihat_open_wavSize)
          && loadFromMemory (Sample::crash, BinaryData::crash_wav, BinaryData::crash_wavSize)
          && loadFromMemory (Sample::tom, BinaryData::tom_wav, BinaryData::tom_wavSize)
          && loadFromMemory (Sample::clickNormal, BinaryData::metronome_beat_1_wav, BinaryData::metronome_beat_1_wavSize)
          && loadFromMemory (Sample::clickAccent, BinaryData::metronome_beat_1_accent_wav, BinaryData::metronome_beat_1_accent_wavSize);
}

bool SampleBank::loadFromMemory (Sample s, const void* data, int dataSize)
{
    auto stream = std::make_unique<juce::MemoryInputStream> (data, (size_t) dataSize, false);
    juce::AudioFormatManager formatManager;
    formatManager.registerBasicFormats();

    if (auto* reader = formatManager.createReaderFor (std::move (stream)))
    {
        auto buf = std::make_unique<juce::AudioBuffer<float>> ((int) reader->numChannels,
                                                               (int) reader->lengthInSamples);
        reader->read (buf.get(), 0, (int) reader->lengthInSamples, 0, true, true);
        sourceRates[(size_t) s] = reader->sampleRate;
        buffers[(size_t) s] = std::move (buf);
        delete reader;
        return true;
    }

    return false;
}

const juce::AudioBuffer<float>* SampleBank::getBuffer (Sample s) const
{
    const auto idx = (size_t) s;
    if (idx < buffers.size() && buffers[idx] != nullptr)
        return buffers[idx].get();
    return nullptr;
}

double SampleBank::getSourceSampleRate (Sample s) const
{
    return sourceRates[(size_t) s];
}
