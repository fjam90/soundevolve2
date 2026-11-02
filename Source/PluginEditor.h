#pragma once
#include "PluginProcessor.h"

// Estilo visual: perillas con arco degradado cian -> magenta y faders del EQ gráfico.
class AranguizLookAndFeel : public juce::LookAndFeel_V4
{
public:
    AranguizLookAndFeel();
    void drawRotarySlider (juce::Graphics&, int x, int y, int width, int height, float sliderPos,
                           float rotaryStartAngle, float rotaryEndAngle, juce::Slider&) override;
    void drawLinearSlider (juce::Graphics&, int x, int y, int width, int height, float sliderPos,
                           float minSliderPos, float maxSliderPos,
                           juce::Slider::SliderStyle, juce::Slider&) override;
};

class MasteringEditor : public juce::AudioProcessorEditor, private juce::Timer
{
public:
    explicit MasteringEditor (MasteringProcessor&);
    ~MasteringEditor() override;

    void paint (juce::Graphics&) override;
    void resized() override;

private:
    void timerCallback() override;

    MasteringProcessor& proc;
    AranguizLookAndFeel laf;
    juce::Image background;

    juce::ComboBox presetBox;
    juce::Label momentaryLabel, integratedLabel;
    juce::TextButton resetButton { "Reset LUFS" };

    struct Control
    {
        juce::Slider slider;
        juce::Label label;
        std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> attachment;
    };
    std::array<Control, mp::numParams> controls;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MasteringEditor)
};
