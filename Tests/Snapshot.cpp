#include "../Source/PluginProcessor.h"
#include "../Source/PluginEditor.h"
int main (int, char** argv)
{
    juce::ScopedJuceInitialiser_GUI gui;
    GrinAudioProcessor p;
    p.prepareToPlay (44100, 512);
    for (float v : { 0.0f, 0.45f, 0.9f })
    {
        p.apvts.getParameter ("smile")->setValueNotifyingHost (v);
        p.engine.setSmile (v);
        juce::AudioBuffer<float> b (2, 512); juce::MidiBuffer m;
        for (int i = 0; i < 400; ++i) { for (int c=0;c<2;++c) for (int s=0;s<512;++s) b.setSample(c,s,0.3f*std::sin(s*0.05f)); p.processBlock (b, m); }
        std::unique_ptr<juce::AudioProcessorEditor> ed (p.createEditor());
        auto img = ed->createComponentSnapshot (ed->getLocalBounds(), true, 2.0f);
        juce::File f (juce::String (argv[1]) + "/ui_" + juce::String ((int) (v * 100)) + ".png");
        f.deleteFile(); juce::FileOutputStream os (f); juce::PNGImageFormat().writeImageToStream (img, os);
    }
    return 0;
}
