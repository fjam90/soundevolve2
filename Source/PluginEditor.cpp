#include "PluginEditor.h"
#include <BinaryData.h>

namespace
{
const juce::Colour cyan    (0xff2de2ff);
const juce::Colour magenta (0xffff3d8b);

// Debe coincidir con PANELS en Assets/make_background.py (ventana 1040 x 600 pt).
struct Panel { juce::Rectangle<int> r; std::vector<int> params; bool faders; };
const std::array<Panel, 5> panels {{
    { { 24, 116, 460, 230 },  { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9 }, true },   // Graphic EQ
    { { 496, 116, 520, 230 }, { 10, 11, 12, 13, 14 },            false },  // Dynamics
    { { 24, 358, 220, 220 },  { 19, 20 },                        false },  // Input / Output
    { { 256, 358, 220, 220 }, { 15, 16 },                        false },  // Color (drive, width)
    { { 488, 358, 220, 220 }, { 17, 18 },                        false } } }; // Limiter
const juce::Rectangle<float> barArea { 736.0f, 500.0f, 264.0f, 10.0f };
constexpr int faderW = 42, faderLeft = 26, faderTop = 40, faderH = 160;

juce::String lufsText (float v) { return v < -69.f ? juce::String ("-inf") : juce::String (v, 1); }
} // namespace

//==============================================================================
AranguizLookAndFeel::AranguizLookAndFeel()
{
    const auto dark = juce::Colour (0xff10131c);
    setColour (juce::ComboBox::backgroundColourId, dark);
    setColour (juce::ComboBox::outlineColourId, juce::Colours::white.withAlpha (0.18f));
    setColour (juce::ComboBox::textColourId, juce::Colours::white);
    setColour (juce::ComboBox::arrowColourId, cyan);
    setColour (juce::PopupMenu::backgroundColourId, dark);
    setColour (juce::PopupMenu::textColourId, juce::Colours::white);
    setColour (juce::PopupMenu::highlightedBackgroundColourId, cyan.withAlpha (0.25f));
    setColour (juce::PopupMenu::highlightedTextColourId, juce::Colours::white);
    setColour (juce::TextButton::buttonColourId, dark);
    setColour (juce::TextButton::textColourOffId, juce::Colours::white);
    setColour (juce::Slider::textBoxTextColourId, juce::Colour (0xffe8eef8));
    setColour (juce::Slider::textBoxOutlineColourId, juce::Colours::transparentBlack);
    setColour (juce::Slider::textBoxBackgroundColourId, juce::Colours::transparentBlack);
    setColour (juce::Label::textColourId, juce::Colour (0xffb8c4d8));
}

void AranguizLookAndFeel::drawRotarySlider (juce::Graphics& g, int x, int y, int width, int height,
                                            float pos, float startAngle, float endAngle, juce::Slider&)
{
    auto bounds = juce::Rectangle<float> ((float) x, (float) y, (float) width, (float) height).reduced (10.0f);
    const float radius = juce::jmin (bounds.getWidth(), bounds.getHeight()) * 0.5f;
    const auto centre = bounds.getCentre();
    const float lineW = 6.0f;
    const float arcR = radius - lineW * 0.5f;
    const float bodyR = arcR - lineW - 3.0f;
    const float angle = startAngle + pos * (endAngle - startAngle);

    g.setGradientFill (juce::ColourGradient (juce::Colour (0xff2a2f3d), centre.x, centre.y - bodyR,
                                             juce::Colour (0xff0d0f16), centre.x, centre.y + bodyR, false));
    g.fillEllipse (centre.x - bodyR, centre.y - bodyR, bodyR * 2.0f, bodyR * 2.0f);
    g.setColour (juce::Colours::white.withAlpha (0.10f));
    g.drawEllipse (centre.x - bodyR, centre.y - bodyR, bodyR * 2.0f, bodyR * 2.0f, 1.0f);

    juce::Path track;
    track.addCentredArc (centre.x, centre.y, arcR, arcR, 0.0f, startAngle, endAngle, true);
    g.setColour (juce::Colour (0xff232836));
    g.strokePath (track, juce::PathStrokeType (lineW, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));

    if (pos > 0.001f)
    {
        juce::Path value;
        value.addCentredArc (centre.x, centre.y, arcR, arcR, 0.0f, startAngle, angle, true);
        juce::ColourGradient cg (cyan, centre.x - arcR, centre.y, magenta, centre.x + arcR, centre.y, false);
        g.setGradientFill (cg);
        g.setOpacity (0.28f);
        g.strokePath (value, juce::PathStrokeType (lineW + 6.0f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
        g.setGradientFill (cg);
        g.strokePath (value, juce::PathStrokeType (lineW, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
    }

    juce::Path pointer;
    pointer.addRoundedRectangle (-1.5f, -bodyR + 3.0f, 3.0f, bodyR * 0.55f, 1.5f);
    g.setColour (juce::Colours::white);
    g.fillPath (pointer, juce::AffineTransform::rotation (angle).translated (centre));
}

void AranguizLookAndFeel::drawLinearSlider (juce::Graphics& g, int x, int y, int width, int height,
                                            float sliderPos, float minSliderPos, float maxSliderPos,
                                            juce::Slider::SliderStyle style, juce::Slider& s)
{
    if (style != juce::Slider::LinearVertical)
    {
        LookAndFeel_V4::drawLinearSlider (g, x, y, width, height, sliderPos, minSliderPos, maxSliderPos, style, s);
        return;
    }

    const float cx = (float) x + (float) width * 0.5f;
    const float top = (float) s.getPositionOfValue (s.getMaximum());
    const float bottom = (float) s.getPositionOfValue (s.getMinimum());
    const float mid = (float) s.getPositionOfValue (0.0);

    // líneas de rejilla (+-6, +-12 dB) y línea de 0 dB
    g.setColour (juce::Colours::white.withAlpha (0.07f));
    for (double v : { -12.0, -6.0, 6.0, 12.0 })
        g.fillRect (juce::Rectangle<float> ((float) x, (float) s.getPositionOfValue (v) - 0.5f, (float) width, 1.0f));
    g.setColour (juce::Colours::white.withAlpha (0.20f));
    g.fillRect (juce::Rectangle<float> ((float) x, mid - 0.5f, (float) width, 1.0f));

    // ranura
    g.setColour (juce::Colour (0xff232836));
    g.fillRoundedRectangle (cx - 2.5f, top - 4.0f, 5.0f, bottom - top + 8.0f, 2.5f);

    // relleno desde 0 dB hasta el valor actual
    g.setGradientFill (juce::ColourGradient (magenta, cx, top, cyan, cx, bottom, false));
    g.fillRoundedRectangle (cx - 2.5f, juce::jmin (mid, sliderPos), 5.0f, std::abs (mid - sliderPos), 2.5f);

    // tirador
    juce::Rectangle<float> thumb (cx - 9.0f, sliderPos - 5.0f, 18.0f, 10.0f);
    g.setGradientFill (juce::ColourGradient (juce::Colours::white, 0.0f, thumb.getY(),
                                             juce::Colour (0xffaab6cc), 0.0f, thumb.getBottom(), false));
    g.fillRoundedRectangle (thumb, 3.0f);
    g.setColour (juce::Colours::black.withAlpha (0.45f));
    g.fillRect (juce::Rectangle<float> (thumb.getX() + 3.0f, sliderPos - 0.5f, 12.0f, 1.0f));
}

//==============================================================================
MasteringEditor::MasteringEditor (MasteringProcessor& p)
    : AudioProcessorEditor (&p), proc (p)
{
    setLookAndFeel (&laf);
    background = juce::ImageCache::getFromMemory (BinaryData::background_png, BinaryData::background_pngSize);

    // Selector de presets con dos submenús
    juce::PopupMenu mixMenu, instMenu;
    for (int i = 0; i < (int) mp::presets.size(); ++i)
        (mp::presets[(size_t) i].category == mp::catMix ? mixMenu : instMenu)
            .addItem (i + 1, mp::presets[(size_t) i].name);
    auto* root = presetBox.getRootMenu();
    root->addSubMenu ("Mix / Master", mixMenu);
    root->addSubMenu ("Instruments", instMenu);
    presetBox.setSelectedId (proc.getCurrentProgram() + 1, juce::dontSendNotification);
    presetBox.onChange = [this] { proc.setCurrentProgram (presetBox.getSelectedId() - 1); };
    addAndMakeVisible (presetBox);

    for (auto* l : { &momentaryLabel, &integratedLabel })
    {
        l->setJustificationType (juce::Justification::centred);
        l->setFont (juce::Font (juce::FontOptions (34.0f, juce::Font::bold)));
        addAndMakeVisible (l);
    }
    momentaryLabel.setColour (juce::Label::textColourId, juce::Colours::white);
    integratedLabel.setColour (juce::Label::textColourId, cyan);

    resetButton.onClick = [this] { proc.meter.requestReset(); };
    addAndMakeVisible (resetButton);

    for (size_t i = 0; i < controls.size(); ++i)
    {
        auto& c = controls[i];
        const bool isFader = (int) i < mp::numEq;

        if (isFader)
        {
            c.slider.setSliderStyle (juce::Slider::LinearVertical);
            c.slider.setTextBoxStyle (juce::Slider::NoTextBox, true, 0, 0);
            c.slider.setPopupDisplayEnabled (true, true, this);
        }
        else
        {
            c.slider.setSliderStyle (juce::Slider::RotaryHorizontalVerticalDrag);
            c.slider.setTextBoxStyle (juce::Slider::TextBoxBelow, false, 72, 20);
        }
        addAndMakeVisible (c.slider);

        c.label.setText (mp::paramNames[i], juce::dontSendNotification);
        c.label.setJustificationType (juce::Justification::centred);
        c.label.setFont (juce::Font (juce::FontOptions (isFader ? 11.0f : 12.0f)));
        addAndMakeVisible (c.label);

        c.attachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (
            proc.apvts, mp::paramIds[i], c.slider);
    }

    setSize (1040, 600);
    startTimerHz (10);
    timerCallback();
}

MasteringEditor::~MasteringEditor() { setLookAndFeel (nullptr); }

void MasteringEditor::paint (juce::Graphics& g)
{
    g.fillAll (juce::Colour (0xff07080d));
    g.setImageResamplingQuality (juce::Graphics::highResamplingQuality);
    g.drawImage (background, getLocalBounds().toFloat(), juce::RectanglePlacement::stretchToFit);

    // escala de dB del EQ gráfico
    {
        auto& s0 = controls[0].slider;
        g.setColour (juce::Colour (0xff8c9ebe));
        g.setFont (juce::Font (juce::FontOptions (9.0f)));
        for (int v : { 12, 6, 0, -6, -12 })
        {
            const float ty = (float) s0.getY() + (float) s0.getPositionOfValue ((double) v);
            g.drawText ((v > 0 ? "+" : "") + juce::String (v),
                        juce::Rectangle<float> ((float) panels[0].r.getX() + 2.0f, ty - 6.0f, 22.0f, 12.0f),
                        juce::Justification::centredRight);
        }
    }

    // textos del panel Loudness
    g.setColour (juce::Colour (0xff8c9ebe));
    g.setFont (juce::Font (juce::FontOptions (10.0f, juce::Font::bold)));
    g.drawText ("MOMENTARY",  736, 384, 128, 16, juce::Justification::centred);
    g.drawText ("INTEGRATED", 872, 384, 128, 16, juce::Justification::centred);
    g.setFont (juce::Font (juce::FontOptions (11.0f)));
    g.drawText ("LUFS", 736, 468, 128, 16, juce::Justification::centred);
    g.drawText ("LUFS", 872, 468, 128, 16, juce::Justification::centred);

    // barra de loudness momentáneo (-40 .. 0 LUFS)
    g.setColour (juce::Colour (0xff1e222e));
    g.fillRoundedRectangle (barArea, 5.0f);
    const float frac = juce::jlimit (0.0f, 1.0f, (proc.meter.getMomentary() + 40.0f) / 40.0f);
    if (frac > 0.0f)
    {
        g.setGradientFill (juce::ColourGradient (cyan, barArea.getX(), 0.0f, magenta, barArea.getRight(), 0.0f, false));
        g.fillRoundedRectangle (barArea.withWidth (barArea.getWidth() * frac), 5.0f);
    }

    struct Tick { float lufs; const char* text; int row; };
    const Tick ticks[] { { -23.0f, "-23 EBU", 0 }, { -16.0f, "-16 Apple", 1 }, { -14.0f, "-14 Spotify", 0 } };
    g.setFont (juce::Font (juce::FontOptions (10.0f)));
    for (auto& t : ticks)
    {
        const float tx = barArea.getX() + barArea.getWidth() * (t.lufs + 40.0f) / 40.0f;
        g.setColour (juce::Colours::white.withAlpha (0.8f));
        g.drawLine (tx, barArea.getY() - 3.0f, tx, barArea.getBottom() + 3.0f, 1.0f);
        g.setColour (juce::Colour (0xffa0b0cc));
        g.drawText (t.text, juce::Rectangle<float> (tx - 40.0f, barArea.getBottom() + 6.0f + 12.0f * (float) t.row, 80.0f, 12.0f),
                    juce::Justification::centred);
    }
}

void MasteringEditor::resized()
{
    presetBox.setBounds (760, 44, 256, 32);

    for (auto& panel : panels)
        for (size_t i = 0; i < panel.params.size(); ++i)
        {
            auto& c = controls[(size_t) panel.params[i]];
            if (panel.faders)
            {
                const int fx = panel.r.getX() + faderLeft + (int) i * faderW;
                const int fy = panel.r.getY() + faderTop;
                c.slider.setBounds (fx, fy, faderW, faderH);
                c.label.setBounds (fx, fy + faderH + 2, faderW, 18);
            }
            else
            {
                auto cell = juce::Rectangle<int> (panel.r.getX() + 10 + (int) i * 100, panel.r.getY() + 34,
                                                  100, panel.r.getHeight() - 44);
                c.label.setBounds (cell.removeFromTop (18));
                c.slider.setBounds (cell);
            }
        }

    momentaryLabel.setBounds (736, 412, 128, 56);
    integratedLabel.setBounds (872, 412, 128, 56);
    resetButton.setBounds (736, 544, 110, 26);
}

void MasteringEditor::timerCallback()
{
    momentaryLabel.setText (lufsText (proc.meter.getMomentary()), juce::dontSendNotification);
    integratedLabel.setText (lufsText (proc.meter.getIntegrated()), juce::dontSendNotification);
    presetBox.setSelectedId (proc.getCurrentProgram() + 1, juce::dontSendNotification);
    repaint (730, 496, 276, 40);
}
