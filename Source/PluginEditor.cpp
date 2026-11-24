#include "PluginEditor.h"

BassCreamAudioProcessorEditor::BassCreamAudioProcessorEditor (BassCreamAudioProcessor& p)
    : AudioProcessorEditor (&p), processor (p)
{
    setLookAndFeel (&lnf);
    setSize (560, 360);
    setResizable (false, false);

    titleLabel.setText ("BASSCREAM", juce::dontSendNotification);
    titleLabel.setJustificationType (juce::Justification::centredLeft);
    titleLabel.setFont (juce::Font (juce::FontOptions (28.0f).withStyle ("Bold")));
    titleLabel.setColour (juce::Label::textColourId, BassCreamLookAndFeel::brassHot());
    addAndMakeVisible (titleLabel);

    subtitleLabel.setText ("bass thickener  ·  slap & rock", juce::dontSendNotification);
    subtitleLabel.setJustificationType (juce::Justification::centredLeft);
    subtitleLabel.setFont (juce::Font (juce::FontOptions (13.0f)));
    subtitleLabel.setColour (juce::Label::textColourId, BassCreamLookAndFeel::muted());
    addAndMakeVisible (subtitleLabel);

    modeHintLabel.setJustificationType (juce::Justification::centred);
    modeHintLabel.setFont (juce::Font (juce::FontOptions (12.0f)));
    modeHintLabel.setColour (juce::Label::textColourId, BassCreamLookAndFeel::slate());
    addAndMakeVisible (modeHintLabel);

    auto styleModeButton = [this] (juce::TextButton& b)
    {
        b.setClickingTogglesState (true);
        b.setRadioGroupId (1001);
        addAndMakeVisible (b);
    };

    styleModeButton (slapButton);
    styleModeButton (rockButton);

    slapButton.onClick = [this]
    {
        if (auto* param = processor.getAPVTS().getParameter ("mode"))
        {
            param->beginChangeGesture();
            param->setValueNotifyingHost (param->convertTo0to1 (0.0f));
            param->endChangeGesture();
        }
        updateModeButtons();
    };

    rockButton.onClick = [this]
    {
        if (auto* param = processor.getAPVTS().getParameter ("mode"))
        {
            param->beginChangeGesture();
            param->setValueNotifyingHost (param->convertTo0to1 (1.0f));
            param->endChangeGesture();
        }
        updateModeButtons();
    };

    setupKnob (creamSlider,  creamLabel,  "CREAM");
    setupKnob (punchSlider,  punchLabel,  "PUNCH");
    setupKnob (toneSlider,   toneLabel,   "TONE");
    setupKnob (outputSlider, outputLabel, "OUTPUT");
    setupKnob (mixSlider,    mixLabel,    "MIX");

    auto& apvts = processor.getAPVTS();
    creamAttachment  = std::make_unique<Attachment> (apvts, "cream",  creamSlider);
    punchAttachment  = std::make_unique<Attachment> (apvts, "punch",  punchSlider);
    toneAttachment   = std::make_unique<Attachment> (apvts, "tone",   toneSlider);
    outputAttachment = std::make_unique<Attachment> (apvts, "output", outputSlider);
    mixAttachment    = std::make_unique<Attachment> (apvts, "mix",    mixSlider);

    updateModeButtons();
    startTimerHz (15);
}

BassCreamAudioProcessorEditor::~BassCreamAudioProcessorEditor()
{
    setLookAndFeel (nullptr);
}

void BassCreamAudioProcessorEditor::setupKnob (juce::Slider& slider, juce::Label& label, const juce::String& text)
{
    slider.setSliderStyle (juce::Slider::RotaryHorizontalVerticalDrag);
    slider.setTextBoxStyle (juce::Slider::TextBoxBelow, false, 64, 18);
    slider.setColour (juce::Slider::textBoxTextColourId, BassCreamLookAndFeel::text());
    slider.setColour (juce::Slider::textBoxOutlineColourId, juce::Colours::transparentBlack);
    addAndMakeVisible (slider);

    label.setText (text, juce::dontSendNotification);
    label.setJustificationType (juce::Justification::centred);
    label.setColour (juce::Label::textColourId, BassCreamLookAndFeel::muted());
    label.attachToComponent (&slider, false);
    addAndMakeVisible (label);
}

void BassCreamAudioProcessorEditor::updateModeButtons()
{
    const int mode = (int) processor.getAPVTS().getRawParameterValue ("mode")->load();
    slapButton.setToggleState (mode == 0, juce::dontSendNotification);
    rockButton.setToggleState (mode == 1, juce::dontSendNotification);

    modeHintLabel.setText (mode == 0
                               ? "transient punch  ·  string clarity  ·  controlled sub"
                               : "warm harmonics  ·  low-mid body  ·  optical glue",
                           juce::dontSendNotification);
}

void BassCreamAudioProcessorEditor::timerCallback()
{
    updateModeButtons();
}

void BassCreamAudioProcessorEditor::paint (juce::Graphics& g)
{
    auto bounds = getLocalBounds().toFloat();

    juce::ColourGradient wash (BassCreamLookAndFeel::bg(),
                               bounds.getTopLeft(),
                               BassCreamLookAndFeel::ink(),
                               bounds.getBottomRight(),
                               false);
    g.setGradientFill (wash);
    g.fillAll();

    // Subtle vertical grain / vignette plane
    g.setColour (BassCreamLookAndFeel::brass().withAlpha (0.035f));
    g.fillRect (0, 0, getWidth(), 72);

    g.setColour (BassCreamLookAndFeel::slate().withAlpha (0.25f));
    g.drawLine (24.0f, 78.0f, (float) getWidth() - 24.0f, 78.0f, 1.0f);

    // Mode panel backdrop
    auto modeArea = juce::Rectangle<float> (24.0f, 96.0f, (float) getWidth() - 48.0f, 56.0f);
    g.setColour (BassCreamLookAndFeel::panel().withAlpha (0.85f));
    g.fillRoundedRectangle (modeArea, 8.0f);
    g.setColour (BassCreamLookAndFeel::slate().withAlpha (0.35f));
    g.drawRoundedRectangle (modeArea, 8.0f, 1.0f);
}

void BassCreamAudioProcessorEditor::resized()
{
    auto area = getLocalBounds().reduced (24);

    auto header = area.removeFromTop (52);
    titleLabel.setBounds (header.removeFromTop (32));
    subtitleLabel.setBounds (header);

    area.removeFromTop (18);
    auto modeRow = area.removeFromTop (56);
    auto buttons = modeRow.removeFromTop (34).reduced (12, 8);
    const int btnW = 100;
    slapButton.setBounds (buttons.removeFromLeft (btnW));
    buttons.removeFromLeft (10);
    rockButton.setBounds (buttons.removeFromLeft (btnW));

    modeHintLabel.setBounds (area.removeFromTop (28));

    area.removeFromTop (18);
    auto knobs = area.removeFromTop (160);
    const int knobW = knobs.getWidth() / 5;

    creamSlider.setBounds  (knobs.removeFromLeft (knobW).reduced (6));
    punchSlider.setBounds  (knobs.removeFromLeft (knobW).reduced (6));
    toneSlider.setBounds   (knobs.removeFromLeft (knobW).reduced (6));
    outputSlider.setBounds (knobs.removeFromLeft (knobW).reduced (6));
    mixSlider.setBounds    (knobs.reduced (6));
}
