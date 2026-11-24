#pragma once

#include <JuceHeader.h>

/**
 * BassCream DSP core.
 *
 * Two characters:
 *  - Slap: transient punch, string clarity, controlled sub, fast dynamics
 *  - Rock: warm even-harmonic cream, body, optical-style glue
 */
class BassEngine
{
public:
    enum class Mode
    {
        Slap = 0,
        Rock = 1
    };

    void prepare (const juce::dsp::ProcessSpec& spec);
    void reset();

    void setMode (Mode newMode) noexcept { mode = newMode; }
    void setCream (float amount01) noexcept { creamTarget = juce::jlimit (0.0f, 1.0f, amount01); }
    void setPunch (float amount01) noexcept { punchTarget = juce::jlimit (0.0f, 1.0f, amount01); }
    void setTone (float amount01) noexcept { toneTarget = juce::jlimit (0.0f, 1.0f, amount01); }
    void setOutputDb (float db) noexcept { outputDbTarget = juce::jlimit (-24.0f, 24.0f, db); }
    void setMix (float amount01) noexcept { mixTarget = juce::jlimit (0.0f, 1.0f, amount01); }

    void process (juce::AudioBuffer<float>& buffer);

private:
    void updateCoefficients();
    float saturateSlap (float x, float drive) const noexcept;
    float saturateRock (float x, float drive) const noexcept;
    float processDynamics (float x, int channel, float punchAmt) noexcept;

    Mode mode { Mode::Rock };

    double sampleRate { 44100.0 };
    int numChannels { 2 };

    // Smoothed user params
    juce::SmoothedValue<float, juce::ValueSmoothingTypes::Linear> cream { 0.45f };
    juce::SmoothedValue<float, juce::ValueSmoothingTypes::Linear> punch { 0.4f };
    juce::SmoothedValue<float, juce::ValueSmoothingTypes::Linear> tone { 0.5f };
    juce::SmoothedValue<float, juce::ValueSmoothingTypes::Linear> outputGain { 1.0f };
    juce::SmoothedValue<float, juce::ValueSmoothingTypes::Linear> mix { 1.0f };

    float creamTarget { 0.45f };
    float punchTarget { 0.4f };
    float toneTarget { 0.5f };
    float outputDbTarget { 0.0f };
    float mixTarget { 1.0f };

    // Filters (stereo-capable arrays)
    static constexpr int maxChannels = 2;

    using Filter = juce::dsp::IIR::Filter<float>;
    using Coeffs = juce::dsp::IIR::Coefficients<float>;

    std::array<Filter, maxChannels> hpf;
    std::array<Filter, maxChannels> bodyPeak;
    std::array<Filter, maxChannels> presencePeak;
    std::array<Filter, maxChannels> airShelf;
    std::array<Filter, maxChannels> mudCut;

    // Simple envelope follower / compressor state per channel
    std::array<float, maxChannels> env { 0.0f, 0.0f };
    std::array<float, maxChannels> gainSmooth { 1.0f, 1.0f };

    Mode lastCoeffMode { Mode::Rock };
    float lastToneForCoeffs { -1.0f };
    int coeffUpdateCounter { 0 };
};
