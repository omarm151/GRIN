#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include "PluginProcessor.h"

class GrinLookAndFeel : public juce::LookAndFeel_V4
{
public:
    GrinLookAndFeel();

    static juce::Colour colourFor (float t);

    void drawRotarySlider (juce::Graphics&, int x, int y, int w, int h, float pos,
                           float startAngle, float endAngle, juce::Slider&) override;

    void drawToggleButton (juce::Graphics&, juce::ToggleButton&, bool highlighted, bool down) override;
};

class GrinAudioProcessorEditor : public juce::AudioProcessorEditor, private juce::Timer
{
public:
    explicit GrinAudioProcessorEditor (GrinAudioProcessor&);
    ~GrinAudioProcessorEditor() override;

    void paint (juce::Graphics&) override;
    void resized() override;

private:
    void timerCallback() override;

    GrinAudioProcessor& processor;
    GrinLookAndFeel lnf;

    juce::Slider smileKnob;
    juce::ToggleButton gateButton { "GATE" }, noiseButton { "NOISE" };

    using SliderAttachment = juce::AudioProcessorValueTreeState::SliderAttachment;
    using ButtonAttachment = juce::AudioProcessorValueTreeState::ButtonAttachment;
    std::unique_ptr<SliderAttachment> smileAttachment;
    std::unique_ptr<ButtonAttachment> gateAttachment, noiseAttachment;

    std::array<float, GrinEngine::NumStages> stageDisplay {};
    float gateDisplay = 1.0f, divisionBeats = 0.0f;

    juce::Rectangle<int> knobArea, stageArea, headerArea;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (GrinAudioProcessorEditor)
};
