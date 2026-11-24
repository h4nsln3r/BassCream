#include "PluginProcessor.h"
#include "PluginEditor.h"

BassCreamAudioProcessor::BassCreamAudioProcessor()
    : AudioProcessor (BusesProperties()
                          .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                          .withOutput ("Output", juce::AudioChannelSet::stereo(), true)),
      apvts (*this, nullptr, "PARAMS", createParameterLayout())
{
}

BassCreamAudioProcessor::~BassCreamAudioProcessor() = default;

juce::AudioProcessorValueTreeState::ParameterLayout BassCreamAudioProcessor::createParameterLayout()
{
    std::vector<std::unique_ptr<juce::RangedAudioParameter>> params;

    params.push_back (std::make_unique<juce::AudioParameterChoice> (
        juce::ParameterID { "mode", 1 },
        "Mode",
        juce::StringArray { "Slap", "Rock" },
        1));

    params.push_back (std::make_unique<juce::AudioParameterFloat> (
        juce::ParameterID { "cream", 1 },
        "Cream",
        juce::NormalisableRange<float> { 0.0f, 1.0f, 0.001f },
        0.45f,
        juce::AudioParameterFloatAttributes().withLabel ("%")
            .withStringFromValueFunction ([] (float v, int) {
                return juce::String (juce::roundToInt (v * 100.0f));
            })));

    params.push_back (std::make_unique<juce::AudioParameterFloat> (
        juce::ParameterID { "punch", 1 },
        "Punch",
        juce::NormalisableRange<float> { 0.0f, 1.0f, 0.001f },
        0.40f,
        juce::AudioParameterFloatAttributes().withLabel ("%")
            .withStringFromValueFunction ([] (float v, int) {
                return juce::String (juce::roundToInt (v * 100.0f));
            })));

    params.push_back (std::make_unique<juce::AudioParameterFloat> (
        juce::ParameterID { "tone", 1 },
        "Tone",
        juce::NormalisableRange<float> { 0.0f, 1.0f, 0.001f },
        0.50f,
        juce::AudioParameterFloatAttributes()
            .withStringFromValueFunction ([] (float v, int) {
                if (v < 0.45f) return juce::String ("Dark");
                if (v > 0.55f) return juce::String ("Bright");
                return juce::String ("Neutral");
            })));

    params.push_back (std::make_unique<juce::AudioParameterFloat> (
        juce::ParameterID { "output", 1 },
        "Output",
        juce::NormalisableRange<float> { -24.0f, 24.0f, 0.1f },
        0.0f,
        juce::AudioParameterFloatAttributes().withLabel ("dB")
            .withStringFromValueFunction ([] (float v, int) {
                return juce::String (v, 1) + " dB";
            })));

    params.push_back (std::make_unique<juce::AudioParameterFloat> (
        juce::ParameterID { "mix", 1 },
        "Mix",
        juce::NormalisableRange<float> { 0.0f, 1.0f, 0.001f },
        1.0f,
        juce::AudioParameterFloatAttributes().withLabel ("%")
            .withStringFromValueFunction ([] (float v, int) {
                return juce::String (juce::roundToInt (v * 100.0f));
            })));

    return { params.begin(), params.end() };
}

void BassCreamAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    juce::dsp::ProcessSpec spec;
    spec.sampleRate = sampleRate;
    spec.maximumBlockSize = (juce::uint32) samplesPerBlock;
    spec.numChannels = (juce::uint32) getTotalNumOutputChannels();
    engine.prepare (spec);
    syncEngineFromParams();
}

void BassCreamAudioProcessor::releaseResources()
{
    engine.reset();
}

bool BassCreamAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
    const auto& mainOut = layouts.getMainOutputChannelSet();
    const auto& mainIn  = layouts.getMainInputChannelSet();

    if (mainOut != mainIn)
        return false;

    return mainOut == juce::AudioChannelSet::mono()
        || mainOut == juce::AudioChannelSet::stereo();
}

void BassCreamAudioProcessor::syncEngineFromParams()
{
    const int modeIndex = (int) apvts.getRawParameterValue ("mode")->load();
    engine.setMode (modeIndex == 0 ? BassEngine::Mode::Slap : BassEngine::Mode::Rock);
    engine.setCream (apvts.getRawParameterValue ("cream")->load());
    engine.setPunch (apvts.getRawParameterValue ("punch")->load());
    engine.setTone (apvts.getRawParameterValue ("tone")->load());
    engine.setOutputDb (apvts.getRawParameterValue ("output")->load());
    engine.setMix (apvts.getRawParameterValue ("mix")->load());
}

void BassCreamAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer&)
{
    juce::ScopedNoDenormals noDenormals;

    for (int ch = getTotalNumInputChannels(); ch < getTotalNumOutputChannels(); ++ch)
        buffer.clear (ch, 0, buffer.getNumSamples());

    syncEngineFromParams();
    engine.process (buffer);
}

void BassCreamAudioProcessor::setCurrentProgram (int index)
{
    currentProgram = juce::jlimit (0, 2, index);

    auto set = [this] (const char* id, float value)
    {
        if (auto* p = apvts.getParameter (id))
        {
            p->beginChangeGesture();
            p->setValueNotifyingHost (p->convertTo0to1 (value));
            p->endChangeGesture();
        }
    };

    auto setChoice = [this] (const char* id, int choiceIndex)
    {
        if (auto* p = dynamic_cast<juce::AudioParameterChoice*> (apvts.getParameter (id)))
        {
            p->beginChangeGesture();
            p->setValueNotifyingHost (p->convertTo0to1 ((float) choiceIndex));
            p->endChangeGesture();
        }
    };

    switch (currentProgram)
    {
        case 0: // Slap Pop Ready
            setChoice ("mode", 0);
            set ("cream", 0.35f);
            set ("punch", 0.65f);
            set ("tone", 0.72f);
            set ("output", 0.0f);
            set ("mix", 1.0f);
            break;

        case 1: // Rock Cream
            setChoice ("mode", 1);
            set ("cream", 0.55f);
            set ("punch", 0.40f);
            set ("tone", 0.35f);
            set ("output", 0.0f);
            set ("mix", 1.0f);
            break;

        case 2: // Subtle Glue
            setChoice ("mode", 1);
            set ("cream", 0.25f);
            set ("punch", 0.28f);
            set ("tone", 0.45f);
            set ("output", 0.0f);
            set ("mix", 0.7f);
            break;

        default:
            break;
    }
}

const juce::String BassCreamAudioProcessor::getProgramName (int index)
{
    switch (index)
    {
        case 0:  return "Slap Pop Ready";
        case 1:  return "Rock Cream";
        case 2:  return "Subtle Glue";
        default: return {};
    }
}

void BassCreamAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    if (auto xml = apvts.copyState().createXml())
        copyXmlToBinary (*xml, destData);
}

void BassCreamAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    if (auto xml = getXmlFromBinary (data, sizeInBytes))
        if (xml->hasTagName (apvts.state.getType()))
            apvts.replaceState (juce::ValueTree::fromXml (*xml));
}

juce::AudioProcessorEditor* BassCreamAudioProcessor::createEditor()
{
    return new BassCreamAudioProcessorEditor (*this);
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new BassCreamAudioProcessor();
}
