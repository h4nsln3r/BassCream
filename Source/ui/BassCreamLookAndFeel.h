#pragma once

#include <JuceHeader.h>

class BassCreamLookAndFeel : public juce::LookAndFeel_V4
{
public:
    BassCreamLookAndFeel();

    void drawRotarySlider (juce::Graphics& g, int x, int y, int width, int height,
                           float sliderPosProportional, float rotaryStartAngle,
                           float rotaryEndAngle, juce::Slider& slider) override;

    void drawButtonBackground (juce::Graphics& g, juce::Button& button,
                               const juce::Colour& backgroundColour,
                               bool shouldDrawButtonAsHighlighted,
                               bool shouldDrawButtonAsDown) override;

    juce::Font getLabelFont (juce::Label& label) override;
    juce::Font getTextButtonFont (juce::TextButton&, int buttonHeight) override;

    // Palette
    static juce::Colour bg()        { return juce::Colour (0xff12161c); }
    static juce::Colour panel()     { return juce::Colour (0xff1a212b); }
    static juce::Colour ink()       { return juce::Colour (0xff0c1016); }
    static juce::Colour brass()     { return juce::Colour (0xffc4a574); }
    static juce::Colour brassHot()  { return juce::Colour (0xffe0c392); }
    static juce::Colour slate()     { return juce::Colour (0xff6d7a8c); }
    static juce::Colour text()      { return juce::Colour (0xffe8e2d6); }
    static juce::Colour muted()     { return juce::Colour (0xff9aa3b0); }
};
