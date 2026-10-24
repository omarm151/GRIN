#include "PluginEditor.h"

namespace
{
    const juce::Colour bgDark   { 0xff0d0c12 };
    const juce::Colour panel    { 0xff17151f };
    const juce::Colour track    { 0xff24212e };
    const juce::Colour textDim  { 0xff8a8598 };
    const juce::Colour textHi   { 0xfff2eff8 };

    const char* stageNames[GrinEngine::NumStages] = { "DRIVE", "FILTER", "FLANGE", "NOISE", "GATE", "PITCH", "SPACE" };
}

//==============================================================================
GrinLookAndFeel::GrinLookAndFeel()
{
    setColour (juce::ResizableWindow::backgroundColourId, bgDark);
}

juce::Colour GrinLookAndFeel::colourFor (float t)
{
    // cool cyan -> hot magenta -> gold
    const juce::Colour a { 0xff3ee6ff }, b { 0xffff3ea5 }, c { 0xffffc23e };
    t = juce::jlimit (0.0f, 1.0f, t);
    return t < 0.5f ? a.interpolatedWith (b, t * 2.0f)
                    : b.interpolatedWith (c, (t - 0.5f) * 2.0f);
}

void GrinLookAndFeel::drawRotarySlider (juce::Graphics& g, int x, int y, int w, int h, float pos,
                                        float startAngle, float endAngle, juce::Slider&)
{
    using namespace juce;

    auto bounds = Rectangle<float> ((float) x, (float) y, (float) w, (float) h).reduced (14.0f);
    const float size   = jmin (bounds.getWidth(), bounds.getHeight());
    const auto  centre = bounds.getCentre();
    const float radius = size * 0.5f;
    const float ringR  = radius - 10.0f;
    const float ringW  = 12.0f;
    const float angle  = startAngle + pos * (endAngle - startAngle);

    // track
    {
        Path p;
        p.addCentredArc (centre.x, centre.y, ringR, ringR, 0.0f, startAngle, endAngle, true);
        g.setColour (track);
        g.strokePath (p, PathStrokeType (ringW, PathStrokeType::curved, PathStrokeType::rounded));
    }

    // value arc: drawn in short segments so the colour sweeps along it
    if (pos > 0.001f)
    {
        // soft glow: one continuous stroke so the overlap doesn't show seams
        {
            Path glowArc;
            glowArc.addCentredArc (centre.x, centre.y, ringR, ringR, 0.0f, startAngle, angle, true);
            g.setGradientFill (ColourGradient (colourFor (0.0f).withAlpha (0.2f), bounds.getX(), centre.y,
                                               colourFor (pos).withAlpha (0.2f), bounds.getRight(), centre.y, false));
            g.strokePath (glowArc, PathStrokeType (ringW + 14.0f, PathStrokeType::curved, PathStrokeType::rounded));
        }

        // value arc in short segments so the colour sweeps along it
        const int segments = jmax (2, (int) (pos * 90.0f));
        for (int i = 0; i < segments; ++i)
        {
            const float t0 = (float) i / (float) segments * pos;
            const float t1 = (float) (i + 1) / (float) segments * pos;
            Path seg;
            seg.addCentredArc (centre.x, centre.y, ringR, ringR, 0.0f,
                               startAngle + t0 * (endAngle - startAngle),
                               startAngle + t1 * (endAngle - startAngle) + 0.01f, true);
            g.setColour (colourFor (t1));
            g.strokePath (seg, PathStrokeType (ringW, PathStrokeType::curved,
                                               i == 0 || i == segments - 1 ? PathStrokeType::rounded
                                                                           : PathStrokeType::butt));
        }
    }

    // knob body
    const float bodyR = ringR - 26.0f;
    {
        ColourGradient grad (Colour (0xff2e2a3b), centre.x, centre.y - bodyR,
                             Colour (0xff131119), centre.x, centre.y + bodyR, false);
        g.setGradientFill (grad);
        g.fillEllipse (centre.x - bodyR, centre.y - bodyR, bodyR * 2.0f, bodyR * 2.0f);

        g.setColour (Colours::white.withAlpha (0.06f));
        g.drawEllipse (centre.x - bodyR, centre.y - bodyR, bodyR * 2.0f, bodyR * 2.0f, 1.5f);
    }

    // the grin: a flat line that bends into a wide smile as the knob turns
    {
        const float mw    = bodyR * (0.48f + 0.14f * pos);
        const float my    = centre.y + bodyR * 0.08f;
        const float curve = bodyR * 0.62f * pos;
        Path mouth;
        mouth.startNewSubPath (centre.x - mw, my - curve * 0.25f);
        mouth.quadraticTo (centre.x, my + curve, centre.x + mw, my - curve * 0.25f);

        const auto col = pos < 0.001f ? textDim : colourFor (pos);
        g.setColour (col.withAlpha (0.25f));
        g.strokePath (mouth, PathStrokeType (16.0f, PathStrokeType::curved, PathStrokeType::rounded));
        g.setColour (col);
        g.strokePath (mouth, PathStrokeType (6.0f, PathStrokeType::curved, PathStrokeType::rounded));
    }

    // pointer
    {
        const auto pt = centre.getPointOnCircumference (bodyR - 14.0f, angle);
        g.setColour (pos < 0.001f ? textDim : colourFor (pos));
        g.fillEllipse (pt.x - 5.0f, pt.y - 5.0f, 10.0f, 10.0f);
    }
}

void GrinLookAndFeel::drawToggleButton (juce::Graphics& g, juce::ToggleButton& b, bool highlighted, bool)
{
    using namespace juce;
    auto r = b.getLocalBounds().toFloat().reduced (2.0f);
    const bool on = b.getToggleState();
    const auto accent = Colour (0xffff3ea5);

    g.setColour (on ? accent.withAlpha (0.18f) : panel);
    g.fillRoundedRectangle (r, r.getHeight() * 0.5f);
    g.setColour (on ? accent : (highlighted ? textDim : track.brighter (0.3f)));
    g.drawRoundedRectangle (r, r.getHeight() * 0.5f, 1.5f);

    g.setColour (on ? textHi : textDim);
    g.setFont (FontOptions (13.0f, Font::bold));
    g.drawText (b.getButtonText(), r, Justification::centred);
}

//==============================================================================
GrinAudioProcessorEditor::GrinAudioProcessorEditor (GrinAudioProcessor& p)
    : AudioProcessorEditor (&p), processor (p)
{
    setLookAndFeel (&lnf);

    smileKnob.setSliderStyle (juce::Slider::RotaryHorizontalVerticalDrag);
    smileKnob.setTextBoxStyle (juce::Slider::NoTextBox, false, 0, 0);
    smileKnob.setRotaryParameters (juce::degreesToRadians (-140.0f), juce::degreesToRadians (140.0f), true);
    smileKnob.setMouseDragSensitivity (320);
    smileKnob.setDoubleClickReturnValue (true, 0.0);
    smileKnob.setPopupDisplayEnabled (false, false, this);
    addAndMakeVisible (smileKnob);

    addAndMakeVisible (gateButton);
    addAndMakeVisible (noiseButton);
    gateButton.setTooltip ("Tempo-synced stutter gate in the upper half of the knob");
    noiseButton.setTooltip ("White-noise riser that follows the track");

    smileAttachment = std::make_unique<SliderAttachment> (p.apvts, "smile", smileKnob);
    gateAttachment  = std::make_unique<ButtonAttachment> (p.apvts, "gate", gateButton);
    noiseAttachment = std::make_unique<ButtonAttachment> (p.apvts, "noise", noiseButton);

    smileKnob.onValueChange = [this] { repaint(); };

    for (int i = 0; i < GrinEngine::NumStages; ++i)
        stageDisplay[(size_t) i] = juce::jlimit (0.0f, 1.0f, p.engine.getStageLevel (i));
    divisionBeats = p.engine.getGateDivisionBeats();

    setSize (440, 580);
    startTimerHz (30);
}

GrinAudioProcessorEditor::~GrinAudioProcessorEditor()
{
    stopTimer();
    setLookAndFeel (nullptr);
}

void GrinAudioProcessorEditor::timerCallback()
{
    bool changed = false;
    for (int i = 0; i < GrinEngine::NumStages; ++i)
    {
        const float target = juce::jlimit (0.0f, 1.0f, processor.engine.getStageLevel (i));
        const float next = stageDisplay[(size_t) i] + 0.35f * (target - stageDisplay[(size_t) i]);
        if (std::abs (next - stageDisplay[(size_t) i]) > 0.002f) changed = true;
        stageDisplay[(size_t) i] = next;
    }

    const float gate = processor.engine.getGateMeter();
    if (std::abs (gate - gateDisplay) > 0.01f) changed = true;
    gateDisplay = gate;

    const float div = processor.engine.getGateDivisionBeats();
    if (std::abs (div - divisionBeats) > 1.0e-4f) changed = true;
    divisionBeats = div;

    if (changed)
        repaint();
}

void GrinAudioProcessorEditor::paint (juce::Graphics& g)
{
    using namespace juce;
    const float s = (float) smileKnob.getValue();
    const auto accent = GrinLookAndFeel::colourFor (s);

    g.fillAll (bgDark);

    // glow behind the knob, pulsing with the gate
    {
        const auto c = knobArea.toFloat().getCentre();
        const float pulse = 0.6f + 0.4f * gateDisplay;
        ColourGradient glow (accent.withAlpha ((0.04f + 0.30f * s) * pulse), c.x, c.y,
                             bgDark.withAlpha (0.0f), c.x, c.y + (float) knobArea.getWidth() * 0.75f, true);
        g.setGradientFill (glow);
        g.fillRect (getLocalBounds());
    }

    // header
    {
        auto h = headerArea.toFloat();
        g.setColour (textHi);
        g.setFont (FontOptions (34.0f, Font::bold));
        g.drawText ("GRIN", h.removeFromLeft (130.0f), Justification::centredLeft);

        g.setColour (textDim);
        g.setFont (FontOptions (11.0f));
        g.drawText ("ONE-KNOB BUILD-UP", headerArea.toFloat().withTrimmedLeft (106.0f).withTrimmedTop (10.0f),
                    Justification::centredLeft);

        g.setColour (s < 0.001f ? textDim : accent);
        g.setFont (FontOptions (30.0f, Font::bold));
        g.drawText (String (roundToInt (s * 100.0f)) + "%", headerArea.toFloat(), Justification::centredRight);
    }

    // stage meters
    {
        auto area = stageArea.toFloat();
        g.setColour (panel);
        g.fillRoundedRectangle (area, 12.0f);

        auto inner = area.reduced (12.0f, 10.0f);
        const float colW = inner.getWidth() / (float) GrinEngine::NumStages;

        for (int i = 0; i < GrinEngine::NumStages; ++i)
        {
            auto col = inner.withX (inner.getX() + colW * (float) i).withWidth (colW);
            auto label = col.removeFromBottom (16.0f);
            auto bar = col.withSizeKeepingCentre (8.0f, col.getHeight() - 6.0f);

            g.setColour (track);
            g.fillRoundedRectangle (bar, 4.0f);

            const float v = stageDisplay[(size_t) i];
            if (v > 0.005f)
            {
                auto fill = bar.withTrimmedTop (bar.getHeight() * (1.0f - v));
                const auto c = GrinLookAndFeel::colourFor ((float) i / (float) (GrinEngine::NumStages - 1));
                g.setColour (c.withAlpha (0.25f));
                g.fillRoundedRectangle (fill.expanded (4.0f, 2.0f), 6.0f);
                g.setColour (c);
                g.fillRoundedRectangle (fill, 4.0f);
            }

            g.setColour (v > 0.05f ? textHi : textDim);
            g.setFont (FontOptions (10.0f, Font::bold));
            g.drawText (stageNames[i], label, Justification::centred);
        }
    }

    // gate division readout next to the toggles
    {
        String div = "--";
        if (divisionBeats > 0.0f)
            div = "1/" + String (roundToInt (4.0f / divisionBeats));
        g.setColour (divisionBeats > 0.0f ? textHi : textDim);
        g.setFont (FontOptions (13.0f, Font::bold));
        g.drawText ("GATE " + div, getLocalBounds().removeFromBottom (58).reduced (24, 14),
                    Justification::centredRight);
    }
}

void GrinAudioProcessorEditor::resized()
{
    auto r = getLocalBounds().reduced (24, 18);
    headerArea = r.removeFromTop (48);
    r.removeFromTop (6);

    auto bottom = r.removeFromBottom (40);
    r.removeFromBottom (14);
    stageArea = r.removeFromBottom (92);
    r.removeFromBottom (8);

    knobArea = r;
    smileKnob.setBounds (knobArea.withSizeKeepingCentre (juce::jmin (knobArea.getWidth(), knobArea.getHeight()),
                                                         juce::jmin (knobArea.getWidth(), knobArea.getHeight())));

    gateButton.setBounds (bottom.removeFromLeft (96));
    bottom.removeFromLeft (10);
    noiseButton.setBounds (bottom.removeFromLeft (96));
}
