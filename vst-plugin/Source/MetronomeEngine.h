#pragma once

#include "SampleBank.h"
#include <JuceHeader.h>
#include <array>
#include <vector>

/** Audio engine ported from the web metronome (script.js). */
class MetronomeEngine
{
public:
    enum class Phase { stopped, countdown, running };

    struct Params
    {
        float bpm = 120.0f;
        int beatsPerBar = 4;
        int beatUnit = 4;
        bool drumsEnabled = true;
        float drumsVolume = 0.8f;
        float clickVolume = 0.6f;
        bool countdownEnabled = true;
        bool autoBpmEnabled = false;
        float autoMin = 100.0f;
        float autoMax = 140.0f;
        int autoStep = 2;
        int autoEvery = 4;
        bool autoUnitBars = true;
        bool autoLoop = true;
        bool autoReverse = false;
        bool useHostTempo = true;
    };

    void prepare (double sampleRate, int maxBlockSize);
    void reset();
    void setParams (const Params& p);

    void start();
    void stop();
    void resetAutoState();
    bool isRunning() const { return internalPlaying; }

    float process (juce::AudioBuffer<float>& buffer,
                   int numSamples,
                   double hostBpm);

    float getCurrentBpm() const { return currentBpm; }
    int getCurrentBeat() const { return currentBeat; }
    int getCurrentBar() const { return currentBar; }
    Phase getPhase() const { return phase; }
    float getBeatProgress() const;
    int getTotalTimeSec() const { return totalTimeSec; }
    juce::String getStatusTimerText() const;
    /** 0 = нет оверлея; 3, 2, 1 во время отсчёта. */
    int getCountdownDisplay() const;

private:
    struct DrumInstrumentConfig
    {
        std::vector<int> firstBar;
        std::vector<std::vector<int>> inbetween;
        std::vector<int> filler;
        float volume = 1.0f;
    };

    using PatternMap = std::map<juce::String, DrumInstrumentConfig>;
    static const PatternMap& getPattern (int beatUnit, int beatsPerBar);

    struct SampleVoice
    {
        bool active = false;
        SampleBank::Sample sample = SampleBank::Sample::kick;
        double position = 0.0;
        double increment = 1.0;
        float gain = 1.0f;
    };

    struct ScheduledHit
    {
        int64_t atSample = 0;
        SampleBank::Sample sample = SampleBank::Sample::kick;
        float volume = 1.0f;
        bool isClick = false;
        bool accentedClick = false;
    };

    struct ClickVoice
    {
        bool active = false;
        double phase = 0.0;
        double env = 0.0;
        float gain = 0.0f;
        double freq = 1000.0;
        double decay = 0.999;  // мультипликативный коэффициент для экспоненциального затухания
    };

    SampleBank sampleBank;
    Phase phase = Phase::stopped;
    int64_t countdownStartSample = 0;
    int64_t countdownEndSample = 0;

    double sampleRate = 44100.0;
    Params params;
    int64_t transportSamples = 0;

    float currentBpm = 120.0f;
    int currentBeat = 0;
    int currentBar = 1;
    int barsSinceStart = 0;
    int patternAlt = 0;
    int autoBarCounter = 0;
    int autoDir = 1;
    double autoElapsedSec = 0.0;
    int lastBeatIndex = -1;
    bool crashOnNextDownbeat = false;

    double samplesPerBeat = 0.0;
    double sampleCounter = 0.0;
    bool internalPlaying = false;
    int totalTimeSec = 0;
    double totalTimeSamples = 0.0;

    static constexpr int kMaxSampleVoices = 24;
    static constexpr int kMaxClickVoices = 8;
    std::array<SampleVoice, kMaxSampleVoices> sampleVoices;
    std::array<ClickVoice, kMaxClickVoices> clickVoices;
    std::vector<ScheduledHit> scheduled;

    void updateSamplesPerBeat();
    double secondsPerBeat() const;
    void beginCountdownOrRunning();
    void startRunningFromDownbeat();
    void processRunning (int numSamples);
    void fireScheduledHitsUpTo (int64_t endSample);
    void scheduleHit (int64_t atSample, SampleBank::Sample sample, float vol);
    void scheduleClick (int64_t atSample, bool accented);
    void triggerClick (bool accented);
    void triggerDrum (SampleBank::Sample sample, float vol);
    SampleVoice* allocateSampleVoice();
    ClickVoice* allocateClickVoice();
    void renderAudio (float* left, float* right, int numSamples);
    void scheduleDrumsForBar (int64_t barStartSample);
    void maybeAutoAdvance (int beatIndex);
    void bumpBpm();
    // barStartSample — точная позиция начала бара в сэмплах (не начало блока)
    void onDownbeat (int64_t barStartSample);
};
