#include "MetronomeEngine.h"
#include <algorithm>

const MetronomeEngine::PatternMap& MetronomeEngine::getPattern (int beatUnit, int beatsPerBar)
{
    static std::map<int, std::map<int, PatternMap>> patterns;
    if (patterns.empty())
    {
        {
            PatternMap p;
            p["open_hihat"] = { {}, { { 15 } }, {}, 0.3f };
            p["closed_hithat"] = { {}, { { 3, 7, 11, 15 }, { 3, 7, 11 } }, {}, 0.05f };
            p["crash_cymbal"] = { { 1 }, {}, {}, 0.5f };
            p["kick"] = { {}, { { 1, 9, 11 } }, {}, 0.95f };
            p["snare"] = { {}, { { 5, 13 } }, { 5, 13, 15, 16 }, 1.0f };
            patterns[4][4] = std::move (p);
        }
        {
            PatternMap p;
            p["closed_hithat"] = { {}, { { 1, 5, 9, 13, 17 } }, {}, 0.05f };
            p["crash_cymbal"] = { { 1 }, {}, {}, 0.5f };
            p["kick"] = { {}, { { 1 } }, {}, 0.95f };
            p["snare"] = { {}, { { 13 } }, {}, 0.8f };
            patterns[4][5] = std::move (p);
        }
        {
            PatternMap p;
            p["open_hihat"] = { {}, { {}, { 11 } }, {}, 0.3f };
            p["closed_hithat"] = { {}, { { 1, 3, 5, 7, 9, 11, 13, 15, 17, 19 },
                                       { 1, 3, 5, 7, 9, 11, 13, 15, 17 } }, {}, 0.05f };
            p["crash_cymbal"] = { { 1 }, {}, {}, 0.5f };
            p["kick"] = { {}, { { 1 } }, {}, 0.95f };
            p["snare"] = { {}, { { 7 } }, { 7, 9, 10 }, 0.8f };
            p["tom"] = { {}, {}, { 11 }, 1.0f };
            patterns[4][6] = std::move (p);
            patterns[8][6] = patterns[4][6];
        }
    }

    static PatternMap empty;
    auto denIt = patterns.find (beatUnit);
    if (denIt == patterns.end())
        return empty;
    auto numIt = denIt->second.find (beatsPerBar);
    if (numIt == denIt->second.end())
        return empty;
    return numIt->second;
}

void MetronomeEngine::prepare (double sr, int)
{
    sampleRate = sr;
    sampleBank.prepare (sr);
    reset();
}

void MetronomeEngine::reset()
{
    phase = Phase::stopped;
    countdownStartSample = 0;
    countdownEndSample = 0;
    transportSamples = 0;
    currentBeat = 0;
    currentBar = 1;
    barsSinceStart = 0;
    patternAlt = 0;
    autoBarCounter = 0;
    autoDir = 1;
    autoElapsedSec = 0.0;
    lastBeatIndex = -1;
    crashOnNextDownbeat = false;
    sampleCounter = 0.0;
    internalPlaying = false;
    totalTimeSec = 0;
    totalTimeSamples = 0.0;
    scheduled.clear();
    for (auto& v : sampleVoices)
        v.active = false;
    for (auto& v : clickVoices)
        v.active = false;
    currentBpm = params.bpm;
    updateSamplesPerBeat();
}

void MetronomeEngine::setParams (const Params& p)
{
    params = p;
}

void MetronomeEngine::start()
{
    if (internalPlaying)
        return;
    internalPlaying = true;
    beginCountdownOrRunning();
}

void MetronomeEngine::stop()
{
    internalPlaying = false;
    reset();
}

void MetronomeEngine::resetAutoState()
{
    autoBarCounter = 0;
    autoDir = 1;
    autoElapsedSec = 0.0;
    lastBeatIndex = -1;
    crashOnNextDownbeat = false;
    const float minV = juce::jmin (params.autoMin, params.autoMax);
    currentBpm = juce::jlimit (20.0f, 300.0f, minV);
    updateSamplesPerBeat();
}

float MetronomeEngine::getBeatProgress() const
{
    if (samplesPerBeat <= 0.0)
        return 0.0f;
    return (float) juce::jlimit (0.0, 1.0, sampleCounter / samplesPerBeat);
}

juce::String MetronomeEngine::getStatusTimerText() const
{
    if (! params.autoBpmEnabled)
        return "---";

    if (params.autoUnitBars)
    {
        const int total = juce::jmax (1, params.autoEvery);
        const int cur = juce::jmin (total, autoBarCounter + 1);
        return juce::String (cur) + "/" + juce::String (total);
    }

    const double need = (double) params.autoEvery * 60.0;
    const double remain = juce::jmax (0.0, need - autoElapsedSec);
    const int m = (int) (remain / 60.0);
    const int s = (int) std::fmod (remain, 60.0);
    return juce::String (m) + ":" + juce::String (s).paddedLeft ('0', 2);
}

void MetronomeEngine::updateSamplesPerBeat()
{
    const double bpm = juce::jlimit (20.0, 300.0, (double) currentBpm);
    const double den = (double) juce::jmax (1, params.beatUnit);
    samplesPerBeat = sampleRate * (60.0 / bpm) * (4.0 / den);
}

double MetronomeEngine::secondsPerBeat() const
{
    const double bpm = juce::jlimit (20.0, 300.0, (double) currentBpm);
    const double den = (double) juce::jmax (1, params.beatUnit);
    return (60.0 / bpm) * (4.0 / den);
}

MetronomeEngine::SampleVoice* MetronomeEngine::allocateSampleVoice()
{
    for (auto& v : sampleVoices)
        if (! v.active)
            return &v;
    return nullptr;
}

MetronomeEngine::ClickVoice* MetronomeEngine::allocateClickVoice()
{
    for (auto& v : clickVoices)
        if (! v.active)
            return &v;
    return nullptr;
}

void MetronomeEngine::triggerClick (bool accented)
{
    if (auto* v = allocateClickVoice())
    {
        v->active = true;
        v->phase = 0.0;
        v->env = 1.0;
        v->freq = accented ? 1600.0 : 1000.0;
        v->gain = (accented ? 0.9f : 0.6f) * params.clickVolume;
        v->decay = 1.0 / (sampleRate * 0.06);
    }
}

void MetronomeEngine::triggerDrum (SampleBank::Sample sample, float vol)
{
    if (! sampleBank.isLoaded())
        return;

    const auto* buf = sampleBank.getBuffer (sample);
    if (buf == nullptr || buf->getNumSamples() == 0)
        return;

    if (auto* v = allocateSampleVoice())
    {
        const double srcRate = sampleBank.getSourceSampleRate (sample);
        v->active = true;
        v->sample = sample;
        v->position = 0.0;
        v->increment = srcRate > 0.0 ? srcRate / sampleRate : 1.0;
        v->gain = vol * params.drumsVolume;
    }
}

void MetronomeEngine::scheduleHit (int64_t atSample, SampleBank::Sample sample, float vol)
{
    ScheduledHit h;
    h.atSample = atSample;
    h.sample = sample;
    h.volume = vol;
    scheduled.push_back (h);
}

void MetronomeEngine::scheduleClick (int64_t atSample, bool accented)
{
    ScheduledHit h;
    h.atSample = atSample;
    h.isClick = true;
    h.accentedClick = accented;
    scheduled.push_back (h);
}

void MetronomeEngine::fireScheduledHitsUpTo (int64_t endSample)
{
    for (const auto& h : scheduled)
    {
        if (h.atSample > endSample)
            continue;

        if (h.isClick)
            triggerClick (h.accentedClick);
        else
            triggerDrum (h.sample, h.volume);
    }

    scheduled.erase (std::remove_if (scheduled.begin(), scheduled.end(),
                                     [endSample] (const ScheduledHit& h)
                                     {
                                         return h.atSample <= endSample;
                                     }),
                     scheduled.end());
}

void MetronomeEngine::renderAudio (float* left, float* right, int numSamples)
{
    for (int i = 0; i < numSamples; ++i)
    {
        float s = 0.0f;

        for (auto& v : clickVoices)
        {
            if (! v.active)
                continue;

            // ФИКС 1: убрано условие `high &&` — теперь оба вида клика (акцент и обычный)
            // генерируют звук. Ранее неакцентированные биты (freq <= 1200) всегда давали out=0.
            const float out = (v.phase < 0.002 * sampleRate) ? 1.0f : 0.0f;
            s += out * (float) v.env * v.gain;
            v.phase += 1.0;
            v.env -= v.decay;
            if (v.phase > sampleRate * 0.07 || v.env <= 0.0)
                v.active = false;
        }

        for (auto& v : sampleVoices)
        {
            if (! v.active)
                continue;

            const auto* buf = sampleBank.getBuffer (v.sample);
            if (buf == nullptr)
            {
                v.active = false;
                continue;
            }

            const int idx = (int) v.position;
            if (idx >= buf->getNumSamples())
            {
                v.active = false;
                continue;
            }

            const float out = buf->getSample (0, idx);
            s += out * v.gain;
            v.position += v.increment;
        }

        left[i] += s;
        right[i] += s;
    }
}

void MetronomeEngine::beginCountdownOrRunning()
{
    currentBeat = 0;
    currentBar = 1;
    barsSinceStart = 0;
    patternAlt = 0;
    autoBarCounter = 0;
    autoDir = 1;
    autoElapsedSec = 0.0;
    lastBeatIndex = -1;
    crashOnNextDownbeat = false;
    sampleCounter = 0.0;
    scheduled.clear();

    const auto cfg = params;
    if (cfg.autoBpmEnabled && (currentBpm < cfg.autoMin || currentBpm > cfg.autoMax))
        currentBpm = juce::jmin (cfg.autoMin, cfg.autoMax);

    updateSamplesPerBeat();

    if (params.countdownEnabled)
    {
        phase = Phase::countdown;
        countdownStartSample = transportSamples;
        countdownEndSample = transportSamples + (int64_t) (3.0 * sampleRate);
        scheduleClick (countdownStartSample, true);
        scheduleClick (countdownStartSample + (int64_t) sampleRate, true);
        scheduleClick (countdownStartSample + (int64_t) (2.0 * sampleRate), true);
    }
    else
    {
        startRunningFromDownbeat();
    }
}

int MetronomeEngine::getCountdownDisplay() const
{
    if (phase != Phase::countdown)
        return 0;

    const auto elapsed = transportSamples - countdownStartSample;
    if (elapsed < (int64_t) sampleRate)
        return 3;
    if (elapsed < (int64_t) (2.0 * sampleRate))
        return 2;
    if (elapsed < countdownEndSample)
        return 1;
    return 0;
}

void MetronomeEngine::startRunningFromDownbeat()
{
    phase = Phase::running;
    scheduleClick (transportSamples, true);
    onDownbeat();

    // ФИКС 2: бит 0 (даунбит) уже отыгран выше — выставляем currentBeat=1,
    // чтобы processRunning не вызвал onDownbeat() повторно через samplesPerBeat,
    // что приводило к дублированию клика и барабанов первого бара.
    currentBeat = 1;
}

void MetronomeEngine::bumpBpm()
{
    const float minV = juce::jmin (params.autoMin, params.autoMax);
    const float maxV = juce::jmax (params.autoMin, params.autoMax);
    float next = currentBpm;

    if (params.autoReverse)
    {
        next = currentBpm + (float) params.autoStep * (float) autoDir;
        if (next > maxV)
        {
            const float over = next - maxV;
            autoDir = -1;
            next = juce::jmax (minV, maxV - over);
        }
        else if (next < minV)
        {
            const float over = minV - next;
            autoDir = 1;
            next = juce::jmin (maxV, minV + over);
        }
    }
    else
    {
        next = currentBpm + (float) params.autoStep;
        if (next > maxV)
            next = params.autoLoop ? minV : maxV;
        if (next < minV)
            next = minV;
    }

    currentBpm = juce::jlimit (20.0f, 300.0f, next);
    updateSamplesPerBeat();
    crashOnNextDownbeat = true;
}

void MetronomeEngine::maybeAutoAdvance (int beatIndex)
{
    if (! params.autoBpmEnabled)
    {
        lastBeatIndex = beatIndex;
        return;
    }

    if (! params.autoUnitBars)
    {
        autoElapsedSec += secondsPerBeat();
        const double need = (double) params.autoEvery * 60.0;
        if (autoElapsedSec + 1e-6 >= need)
        {
            autoElapsedSec = 0.0;
            bumpBpm();
        }
    }
    else
    {
        if (lastBeatIndex < 0)
        {
            lastBeatIndex = beatIndex;
            return;
        }
        if (beatIndex == 0 && lastBeatIndex != 0)
        {
            ++autoBarCounter;
            if (autoBarCounter >= juce::jmax (1, params.autoEvery))
            {
                autoBarCounter = 0;
                bumpBpm();
            }
        }
    }
    lastBeatIndex = beatIndex;
}

void MetronomeEngine::onDownbeat()
{
    scheduleDrumsForBar (transportSamples);

    if (crashOnNextDownbeat && params.drumsEnabled)
    {
        scheduleHit (transportSamples, SampleBank::Sample::crash, 0.5f);
        crashOnNextDownbeat = false;
    }

    ++barsSinceStart;
    patternAlt = 1 - patternAlt;
    ++currentBar;
}

void MetronomeEngine::scheduleDrumsForBar (int64_t barStartSample)
{
    if (! params.drumsEnabled || ! sampleBank.isLoaded())
        return;

    const auto& pat = getPattern (params.beatUnit, params.beatsPerBar);
    if (pat.empty())
        return;

    bool shouldPlayFiller = false;
    if (params.autoBpmEnabled && currentBar > 1)
    {
        if (params.autoUnitBars)
            shouldPlayFiller = autoBarCounter == juce::jmax (1, params.autoEvery) - 1;
        else
        {
            const double need = (double) params.autoEvery * 60.0;
            const double remaining = need - autoElapsedSec;
            shouldPlayFiller = remaining <= (double) params.beatsPerBar * secondsPerBeat() + 1e-6;
        }
    }

    const double sixteenthSamples = sampleRate * 60.0 / (currentBpm * 4.0);
    const int altIdx = juce::jmin (patternAlt, 1);

    for (const auto& [name, cfg] : pat)
    {
        const std::vector<int>* beatList = nullptr;

        if (name == "crash_cymbal")
        {
            if (barsSinceStart == 0)
                beatList = &cfg.firstBar;
        }
        else
        {
            if (barsSinceStart == 0 && ! cfg.firstBar.empty())
                beatList = &cfg.firstBar;
            else if (shouldPlayFiller && ! cfg.filler.empty())
                beatList = &cfg.filler;
            else if (! cfg.inbetween.empty())
            {
                const int idx = juce::jmin (altIdx, (int) cfg.inbetween.size() - 1);
                beatList = &cfg.inbetween[(size_t) idx];
            }
        }

        if (beatList == nullptr)
            continue;

        SampleBank::Sample sample = SampleBank::Sample::kick;
        if (name == "kick")
            sample = SampleBank::Sample::kick;
        else if (name == "snare")
            sample = SampleBank::Sample::snare;
        else if (name == "closed_hithat" || name == "closed_hihat")
            sample = SampleBank::Sample::hatClosed;
        else if (name == "open_hihat")
            sample = SampleBank::Sample::hatOpen;
        else if (name == "crash_cymbal")
            sample = SampleBank::Sample::crash;
        else if (name == "tom")
            sample = SampleBank::Sample::tom;
        else
            continue;

        for (int sixteenth : *beatList)
        {
            const int64_t at = barStartSample + (int64_t) ((sixteenth - 1) * sixteenthSamples);
            scheduleHit (at, sample, cfg.volume);
        }
    }
}

void MetronomeEngine::processRunning (int numSamples)
{
    const int64_t blockStart = transportSamples;
    const int64_t blockEnd = blockStart + numSamples;

    for (int i = 0; i < numSamples; ++i)
    {
        const int64_t t = blockStart + i;

        if (sampleCounter >= samplesPerBeat)
        {
            sampleCounter -= samplesPerBeat;
            maybeAutoAdvance (currentBeat);

            const bool accented = currentBeat == 0;
            scheduleClick (t, accented);

            if (currentBeat == 0)
                onDownbeat();

            currentBeat = (currentBeat + 1) % juce::jmax (1, params.beatsPerBar);
        }

        sampleCounter += 1.0;
    }

    fireScheduledHitsUpTo (blockEnd);
    transportSamples = blockEnd;
}

float MetronomeEngine::process (juce::AudioBuffer<float>& buffer,
                                int numSamples,
                                double hostBpm)
{
    buffer.clear();

    if (! internalPlaying)
        return currentBpm;

    if (params.useHostTempo && hostBpm > 20.0 && hostBpm < 300.0)
        currentBpm = (float) hostBpm;
    else
        currentBpm = params.bpm;

    updateSamplesPerBeat();

    totalTimeSamples += (double) numSamples;
    while (totalTimeSamples >= sampleRate)
    {
        totalTimeSamples -= sampleRate;
        ++totalTimeSec;
    }

    const int64_t blockEnd = transportSamples + numSamples;

    if (phase == Phase::countdown)
    {
        fireScheduledHitsUpTo (blockEnd);
        if (blockEnd >= countdownEndSample)
            startRunningFromDownbeat();
        transportSamples = blockEnd;
    }
    else if (phase == Phase::running)
    {
        processRunning (numSamples);
    }

    auto* left = buffer.getWritePointer (0);
    auto* right = buffer.getNumChannels() > 1 ? buffer.getWritePointer (1) : left;
    renderAudio (left, right, numSamples);

    return currentBpm;
}
