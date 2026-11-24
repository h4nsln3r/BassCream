#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"
#include "ui/BassCreamLookAndFeel.h"

class BassCreamAudioProcessorEditor : public juce::AudioProcessorEditor,
                                      private juce::Timer
{
public:
    explicit BassCreamAudioProcessorEditor (BassCreamAudioProcessor&);
    ~BassCreamAudioProcessorEditor() override;

    void paint (juce::Graphics&) override;
    void resized() override;

private:
    void timerCallback() override;
    void setupKnob (juce::Slider& slider, juce::Label& label, const juce::String& text);
    void updateModeButtons();

    BassCreamAudioProcessor& processor;
    BassCreamLookAndFeel lnf;

    juce::TextButton slapButton { "SLAP" };
    juce::TextButton rockButton { "ROCK" };

    juce::Slider creamSlider, punchSlider, toneSlider, outputSlider, mixSlider;
    juce::Label creamLabel, punchLabel, toneLabel, outputLabel, mixLabel;
    juce::Label titleLabel, subtitleLabel, modeHintLabel;

    using Attachment = juce::AudioProcessorValueTreeState::SliderAttachment;
    using ButtonAttachment = juce::AudioProcessorValueTreeState::ButtonAttachment;

    std::unique_ptr<Attachment> creamAttachment, punchAttachment, toneAttachment,
                                outputAttachment, mixAttachment;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (BassCreamAudioProcessorEditor)
};
