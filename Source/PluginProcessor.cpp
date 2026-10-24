#include "PluginProcessor.h"
#include "PluginEditor.h"

GrinAudioProcessor::GrinAudioProcessor()
    : AudioProcessor (BusesProperties()
                          .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                          .withOutput ("Output", juce::AudioChannelSet::stereo(), true)),
      apvts (*this, nullptr, "GRIN", createLayout())
{
    smileParam = apvts.getRawParameterValue ("smile");
    gateParam  = apvts.getRawParameterValue ("gate");
    noiseParam = apvts.getRawParameterValue ("noise");
    modeParam  = apvts.getRawParameterValue ("mode");
}

juce::AudioProcessorValueTreeState::ParameterLayout GrinAudioProcessor::createLayout()
{
    using namespace juce;
    AudioProcessorValueTreeState::ParameterLayout layout;

    layout.add (std::make_unique<AudioParameterFloat> (
        ParameterID { "smile", 1 }, "Smile",
        NormalisableRange<float> (0.0f, 1.0f, 0.0f), 0.0f,
        AudioParameterFloatAttributes()
            .withStringFromValueFunction ([] (float v, int) { return String (roundToInt (v * 100.0f)) + " %"; })
            .withValueFromStringFunction ([] (const String& s) { return s.getFloatValue() / 100.0f; })));

    const StringArray modes { "Soft 1", "Soft 2", "Medium 1", "Medium 2", "Hard 1", "Hard 2", "Extreme" };
    layout.add (std::make_unique<AudioParameterChoice> (ParameterID { "mode", 1 }, "Strength", modes, GrinEngine::Medium1));

    layout.add (std::make_unique<AudioParameterBool> (ParameterID { "gate", 1 },  "Gate",  true));
    layout.add (std::make_unique<AudioParameterBool> (ParameterID { "noise", 1 }, "Noise", true));

    return layout;
}

bool GrinAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
    const auto out = layouts.getMainOutputChannelSet();
    if (out != juce::AudioChannelSet::stereo() && out != juce::AudioChannelSet::mono())
        return false;
    return layouts.getMainInputChannelSet() == out;
}

void GrinAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    engine.setSmile (smileParam->load());
    engine.setMode ((int) modeParam->load());
    engine.prepare (sampleRate, samplesPerBlock);
}

void GrinAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer&)
{
    juce::ScopedNoDenormals noDenormals;

    for (int ch = getTotalNumInputChannels(); ch < getTotalNumOutputChannels(); ++ch)
        buffer.clear (ch, 0, buffer.getNumSamples());

    double bpm = 0.0, ppq = 0.0;
    bool playing = false;

    if (auto* ph = getPlayHead())
    {
        if (auto pos = ph->getPosition())
        {
            if (auto b = pos->getBpm())         bpm = *b;
            if (auto p = pos->getPpqPosition()) ppq = *p;
            playing = pos->getIsPlaying() && pos->getPpqPosition().hasValue();
        }
    }

    engine.setSmile (smileParam->load());
    engine.setGateEnabled (gateParam->load() > 0.5f);
    engine.setNoiseEnabled (noiseParam->load() > 0.5f);
    engine.setMode ((int) modeParam->load());
    engine.process (buffer, bpm, ppq, playing);
}

juce::AudioProcessorEditor* GrinAudioProcessor::createEditor()
{
    return new GrinAudioProcessorEditor (*this);
}

void GrinAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    if (auto xml = apvts.copyState().createXml())
        copyXmlToBinary (*xml, destData);
}

void GrinAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    if (auto xml = getXmlFromBinary (data, sizeInBytes))
        if (xml->hasTagName (apvts.state.getType()))
            apvts.replaceState (juce::ValueTree::fromXml (*xml));
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new GrinAudioProcessor();
}
