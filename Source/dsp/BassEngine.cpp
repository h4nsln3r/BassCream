#include "BassEngine.h"

void BassEngine::prepare (const juce::dsp::ProcessSpec& spec)
{
    sampleRate = spec.sampleRate;
    numChannels = (int) juce::jmin (spec.numChannels, (juce::uint32) maxChannels);

    const double rampSec = 0.05;
    cream.reset (sampleRate, rampSec);
    punch.reset (sampleRate, rampSec);
    tone.reset (sampleRate, rampSec);
    outputGain.reset (sampleRate, rampSec);
    mix.reset (sampleRate, rampSec);

    cream.setCurrentAndTargetValue (creamTarget);
    punch.setCurrentAndTargetValue (punchTarget);
    tone.setCurrentAndTargetValue (toneTarget);
    outputGain.setCurrentAndTargetValue (juce::Decibels::decibelsToGain (outputDbTarget));
    mix.setCurrentAndTargetValue (mixTarget);

    juce::dsp::ProcessSpec monoSpec = spec;
    monoSpec.numChannels = 1;

    for (int ch = 0; ch < maxChannels; ++ch)
    {
        hpf[ch].prepare (monoSpec);
        bodyPeak[ch].prepare (monoSpec);
        presencePeak[ch].prepare (monoSpec);
        airShelf[ch].prepare (monoSpec);
        mudCut[ch].prepare (monoSpec);
        env[ch] = 0.0f;
        gainSmooth[ch] = 1.0f;
    }

    lastToneForCoeffs = -1.0f;
    updateCoefficients();
    reset();
}

void BassEngine::reset()
{
    for (int ch = 0; ch < maxChannels; ++ch)
    {
        hpf[ch].reset();
        bodyPeak[ch].reset();
        presencePeak[ch].reset();
        airShelf[ch].reset();
        mudCut[ch].reset();
        env[ch] = 0.0f;
        gainSmooth[ch] = 1.0f;
    }
}

void BassEngine::updateCoefficients()
{
    const float t = tone.getCurrentValue();
    lastCoeffMode = mode;
    lastToneForCoeffs = t;

    // Tone: 0 = darker, 1 = brighter — morphs presence / air
    const float darkBright = t;

    for (int ch = 0; ch < maxChannels; ++ch)
    {
        *hpf[ch].coefficients = *Coeffs::makeHighPass (sampleRate, 28.0, 0.707);

        if (mode == Mode::Slap)
        {
            // Clean sub, cut mud, lift string definition + click
            const float mudGain = juce::Decibels::decibelsToGain (-2.5f - (1.0f - darkBright) * 1.5f);
            const float bodyGain = juce::Decibels::decibelsToGain (-0.5f + darkBright * 0.5f);
            const float presenceGain = juce::Decibels::decibelsToGain (2.0f + darkBright * 3.5f);
            const float airGain = juce::Decibels::decibelsToGain (-1.0f + darkBright * 3.0f);

            *mudCut[ch].coefficients = *Coeffs::makePeakFilter (sampleRate, 220.0, 0.9, mudGain);
            *bodyPeak[ch].coefficients = *Coeffs::makePeakFilter (sampleRate, 90.0, 0.7, bodyGain);
            *presencePeak[ch].coefficients = *Coeffs::makePeakFilter (sampleRate, 900.0 + darkBright * 400.0, 1.1, presenceGain);
            *airShelf[ch].coefficients = *Coeffs::makeHighShelf (sampleRate, 2800.0, 0.7, airGain);
        }
        else
        {
            // Rock cream: thick low-mid body, soft top
            const float mudGain = juce::Decibels::decibelsToGain (-1.0f + (1.0f - darkBright) * 0.5f);
            const float bodyGain = juce::Decibels::decibelsToGain (2.5f + (1.0f - darkBright) * 2.0f);
            const float presenceGain = juce::Decibels::decibelsToGain (0.5f + darkBright * 1.5f);
            const float airGain = juce::Decibels::decibelsToGain (-2.5f + darkBright * 2.5f);

            *mudCut[ch].coefficients = *Coeffs::makePeakFilter (sampleRate, 280.0, 0.8, mudGain);
            *bodyPeak[ch].coefficients = *Coeffs::makePeakFilter (sampleRate, 110.0, 0.6, bodyGain);
            *presencePeak[ch].coefficients = *Coeffs::makePeakFilter (sampleRate, 650.0, 0.9, presenceGain);
            *airShelf[ch].coefficients = *Coeffs::makeHighShelf (sampleRate, 4500.0, 0.7, airGain);
        }
    }
}

float BassEngine::saturateSlap (float x, float drive) const noexcept
{
    // Brighter, slightly asymmetric — favours string attack harmonics
    const float d = 1.0f + drive * 4.5f;
    const float driven = x * d;
    const float shaped = std::tanh (driven * 1.15f) + 0.08f * driven / (1.0f + driven * driven);
    return shaped / (1.0f + drive * 0.55f);
}

float BassEngine::saturateRock (float x, float drive) const noexcept
{
    // Soft tube-ish: even + gentle odd harmonics, warm fold
    const float d = 1.0f + drive * 3.2f;
    const float y = x * d;
    const float y2 = y * y;
    const float tube = y - 0.22f * y2 * y + 0.04f * y2 * y2 * y;
    const float soft = std::tanh (tube * 0.95f);
    return soft / (1.0f + drive * 0.4f);
}

float BassEngine::processDynamics (float x, int channel, float punchAmt) noexcept
{
    const float absX = std::abs (x);

    // Envelope follower
    const float attackCoeff = (mode == Mode::Slap)
        ? std::exp (-1.0f / (float) (sampleRate * 0.002))   // ~2 ms
        : std::exp (-1.0f / (float) (sampleRate * 0.015));  // ~15 ms

    const float releaseCoeff = (mode == Mode::Slap)
        ? std::exp (-1.0f / (float) (sampleRate * 0.080))
        : std::exp (-1.0f / (float) (sampleRate * 0.220));

    if (absX > env[channel])
        env[channel] = attackCoeff * env[channel] + (1.0f - attackCoeff) * absX;
    else
        env[channel] = releaseCoeff * env[channel] + (1.0f - releaseCoeff) * absX;

    // Threshold / ratio morph with punch
    const float thresh = (mode == Mode::Slap)
        ? juce::Decibels::decibelsToGain (-18.0f + punchAmt * 6.0f)
        : juce::Decibels::decibelsToGain (-22.0f + punchAmt * 8.0f);

    const float ratio = (mode == Mode::Slap)
        ? 2.5f + punchAmt * 4.0f
        : 1.6f + punchAmt * 2.2f;

    float gr = 1.0f;
    if (env[channel] > thresh)
    {
        const float over = env[channel] / thresh;
        const float compressed = std::pow (over, 1.0f / ratio);
        gr = (compressed * thresh) / juce::jmax (env[channel], 1.0e-6f);
    }

    // Slap: add a touch of upward transient "push" before the hit settles
    if (mode == Mode::Slap && punchAmt > 0.05f)
    {
        const float transient = juce::jmax (0.0f, absX - env[channel] * 0.85f);
        const float push = 1.0f + transient * punchAmt * 2.8f;
        x *= push;
    }

    // Makeup that scales with punch so "more punch" still feels louder/forward
    const float makeup = juce::Decibels::decibelsToGain ((mode == Mode::Slap ? 1.5f : 2.0f) * punchAmt);
    const float targetGain = gr * makeup;

    const float smooth = std::exp (-1.0f / (float) (sampleRate * 0.005));
    gainSmooth[channel] = smooth * gainSmooth[channel] + (1.0f - smooth) * targetGain;

    return x * gainSmooth[channel];
}

void BassEngine::process (juce::AudioBuffer<float>& buffer)
{
    cream.setTargetValue (creamTarget);
    punch.setTargetValue (punchTarget);
    tone.setTargetValue (toneTarget);
    outputGain.setTargetValue (juce::Decibels::decibelsToGain (outputDbTarget));
    mix.setTargetValue (mixTarget);

    // Refresh EQ every ~1024 samples or on mode/tone change
    if (++coeffUpdateCounter >= 1024
        || mode != lastCoeffMode
        || std::abs (tone.getCurrentValue() - lastToneForCoeffs) > 0.02f)
    {
        coeffUpdateCounter = 0;
        updateCoefficients();
    }

    const int channels = juce::jmin (buffer.getNumChannels(), numChannels);
    const int numSamples = buffer.getNumSamples();

    for (int i = 0; i < numSamples; ++i)
    {
        const float creamAmt = cream.getNextValue();
        const float punchAmt = punch.getNextValue();
        const float outG = outputGain.getNextValue();
        const float mixAmt = mix.getNextValue();
        tone.getNextValue(); // keep smoother in sync

        for (int ch = 0; ch < channels; ++ch)
        {
            float dry = buffer.getSample (ch, i);
            float x = dry;

            x = hpf[ch].processSample (x);
            x = mudCut[ch].processSample (x);
            x = bodyPeak[ch].processSample (x);
            x = presencePeak[ch].processSample (x);
            x = airShelf[ch].processSample (x);

            if (mode == Mode::Slap)
                x = saturateSlap (x, creamAmt);
            else
                x = saturateRock (x, creamAmt);

            x = processDynamics (x, ch, punchAmt);
            x *= outG;

            buffer.setSample (ch, i, dry * (1.0f - mixAmt) + x * mixAmt);
        }
    }
}
