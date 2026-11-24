#include "BassCreamLookAndFeel.h"

BassCreamLookAndFeel::BassCreamLookAndFeel()
{
    setColour (juce::ResizableWindow::backgroundColourId, bg());
    setColour (juce::Label::textColourId, text());
    setColour (juce::Slider::rotarySliderFillColourId, brass());
    setColour (juce::Slider::rotarySliderOutlineColourId, slate().withAlpha (0.45f));
    setColour (juce::Slider::thumbColourId, brassHot());
    setColour (juce::TextButton::buttonColourId, panel());
    setColour (juce::TextButton::buttonOnColourId, brass().withAlpha (0.28f));
    setColour (juce::TextButton::textColourOffId, muted());
    setColour (juce::TextButton::textColourOnId, brassHot());
}

juce::Font BassCreamLookAndFeel::getLabelFont (juce::Label&)
{
    return juce::Font (juce::FontOptions (13.0f).withStyle ("Regular"));
}

juce::Font BassCreamLookAndFeel::getTextButtonFont (juce::TextButton&, int)
{
    return juce::Font (juce::FontOptions (14.0f).withStyle ("Bold"));
}

void BassCreamLookAndFeel::drawRotarySlider (juce::Graphics& g, int x, int y, int width, int height,
                                             float sliderPos, float rotaryStartAngle,
                                             float rotaryEndAngle, juce::Slider& slider)
{
    const auto bounds = juce::Rectangle<float> ((float) x, (float) y, (float) width, (float) height).reduced (8.0f);
    const auto radius = juce::jmin (bounds.getWidth(), bounds.getHeight()) * 0.5f;
    const auto centre = bounds.getCentre();
    const auto toAngle = rotaryStartAngle + sliderPos * (rotaryEndAngle - rotaryStartAngle);
    const auto lineW = juce::jmax (2.0f, radius * 0.08f);

    juce::Path track;
    track.addCentredArc (centre.x, centre.y, radius - lineW * 0.5f, radius - lineW * 0.5f,
                         0.0f, rotaryStartAngle, rotaryEndAngle, true);
    g.setColour (ink());
    g.strokePath (track, juce::PathStrokeType (lineW * 2.2f, juce::PathStrokeType::curved,
                                               juce::PathStrokeType::rounded));
    g.setColour (slate().withAlpha (0.35f));
    g.strokePath (track, juce::PathStrokeType (lineW, juce::PathStrokeType::curved,
                                               juce::PathStrokeType::rounded));

    juce::Path valueArc;
    valueArc.addCentredArc (centre.x, centre.y, radius - lineW * 0.5f, radius - lineW * 0.5f,
                            0.0f, rotaryStartAngle, toAngle, true);
    g.setColour (slider.isEnabled() ? brass() : slate());
    g.strokePath (valueArc, juce::PathStrokeType (lineW, juce::PathStrokeType::curved,
                                                  juce::PathStrokeType::rounded));

    const float knobR = radius * 0.62f;
    g.setColour (panel());
    g.fillEllipse (centre.x - knobR, centre.y - knobR, knobR * 2.0f, knobR * 2.0f);
    g.setColour (ink().withAlpha (0.6f));
    g.drawEllipse (centre.x - knobR, centre.y - knobR, knobR * 2.0f, knobR * 2.0f, 1.0f);

    juce::Path pointer;
    const float pointerLen = knobR * 0.72f;
    pointer.addRoundedRectangle (-lineW * 0.5f, -pointerLen, lineW, pointerLen * 0.72f, lineW * 0.3f);
    g.setColour (brassHot());
    g.fillPath (pointer, juce::AffineTransform::rotation (toAngle).translated (centre.x, centre.y));
}

void BassCreamLookAndFeel::drawButtonBackground (juce::Graphics& g, juce::Button& button,
                                                 const juce::Colour&,
                                                 bool shouldDrawButtonAsHighlighted,
                                                 bool shouldDrawButtonAsDown)
{
    auto bounds = button.getLocalBounds().toFloat().reduced (1.0f);
    const bool on = button.getToggleState();

    g.setColour (on ? brass().withAlpha (0.22f) : panel());
    g.fillRoundedRectangle (bounds, 6.0f);

    g.setColour (on ? brass() : (shouldDrawButtonAsHighlighted || shouldDrawButtonAsDown
                                     ? slate().brighter (0.2f)
                                     : slate().withAlpha (0.5f)));
    g.drawRoundedRectangle (bounds, 6.0f, on ? 1.5f : 1.0f);
}
