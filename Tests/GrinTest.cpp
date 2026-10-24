// Renders a synthetic 128 BPM loop through the GRIN engine with the Smile knob
// automated over a 12-bar build-up, writes WAV files, and checks for NaNs,
// clipping and transparency at Smile = 0.

#include <juce_audio_formats/juce_audio_formats.h>
#include "GrinEngine.h"

using namespace juce;

static AudioBuffer<float> makeLoop (double sr, double bpm, int bars)
{
    const int beats = bars * 4;
    const int n = (int) (beats * 60.0 / bpm * sr);
    AudioBuffer<float> b (2, n);
    b.clear();
    Random rnd (7);

    const double spb = 60.0 / bpm * sr; // samples per beat
    // Am  F  C  G  (midi notes)
    const int chords[4][4] = { { 57, 60, 64, 69 }, { 53, 57, 60, 65 }, { 48, 52, 55, 60 }, { 55, 59, 62, 67 } };
    const int bassRoots[4] = { 45, 41, 48, 43 };
    std::vector<double> sawPh (12 * 2, 0.0);
    double bassPh = 0.0;
    float lpL = 0, lpR = 0, bassLp = 0;

    for (int i = 0; i < n; ++i)
    {
        const double beatPos = i / spb;
        const int beat = (int) beatPos;
        const double inBeat = beatPos - beat;
        const int bar = beat / 4;
        const auto& ch = chords[bar % 4];

        // kick
        const double tk = inBeat * spb / sr;
        const double kf = 45.0 + 120.0 * std::exp (-tk * 30.0);
        static double kph = 0.0;
        if (inBeat * spb < 1.0) kph = 0.0;
        kph += kf / sr;
        const float kick = (float) (std::sin (kph * MathConstants<double>::twoPi) * std::exp (-tk * 7.0)) * 0.8f;

        // offbeat hat
        const double hatPos = std::fmod (beatPos + 0.5, 1.0) * spb / sr;
        const float hat = (rnd.nextFloat() * 2.0f - 1.0f) * (float) std::exp (-hatPos * 60.0) * 0.12f;

        // supersaw chord, pumping
        const float pump = (float) (0.35 + 0.65 * std::min (1.0, inBeat * 3.0));
        float sl = 0, sr2 = 0;
        for (int v = 0; v < 4; ++v)
            for (int d = 0; d < 3; ++d)
            {
                const double f = 440.0 * std::pow (2.0, (ch[v] - 69) / 12.0) * (1.0 + (d - 1) * 0.006);
                for (int side = 0; side < 2; ++side)
                {
                    auto& ph = sawPh[(size_t) ((v * 3 + d) * 2 + side)];
                    ph += f * (1.0 + side * 0.002) / sr;
                    ph -= std::floor (ph);
                    (side == 0 ? sl : sr2) += (float) (2.0 * ph - 1.0);
                }
            }
        lpL += 0.18f * (sl - lpL);
        lpR += 0.18f * (sr2 - lpR);
        const float padL = lpL * 0.035f * pump, padR = lpR * 0.035f * pump;

        // offbeat bass
        const double bf = 440.0 * std::pow (2.0, (bassRoots[bar % 4] - 69) / 12.0);
        bassPh += bf / sr; bassPh -= std::floor (bassPh);
        const float bassEnv = inBeat > 0.5 ? (float) std::exp (-(inBeat - 0.5) * 6.0) : 0.0f;
        bassLp += 0.08f * ((float) (2.0 * bassPh - 1.0) - bassLp);
        const float bass = bassLp * bassEnv * 0.35f;

        b.setSample (0, i, kick + hat + padL + bass);
        b.setSample (1, i, kick + hat * 0.8f + padR + bass);
    }
    return b;
}

static void writeWav (const AudioBuffer<float>& b, double sr, const File& f)
{
    f.deleteFile();
    WavAudioFormat wav;
    std::unique_ptr<AudioFormatWriter> w (wav.createWriterFor (new FileOutputStream (f), sr, 2, 24, {}, 0));
    w->writeFromAudioSampleBuffer (b, 0, b.getNumSamples());
}

int main (int argc, char** argv)
{
    const double sr = 44100.0, bpm = 128.0;
    const int bars = 18;
    const File outDir = argc > 1 ? File (argv[1]) : File::getCurrentWorkingDirectory();

    const auto dry = makeLoop (sr, bpm, bars);
    const int n = dry.getNumSamples();
    const double samplesPerBar = 4.0 * 60.0 / bpm * sr;

    auto render = [&] (std::function<float (double bar)> automation, bool gate, bool noise)
    {
        AudioBuffer<float> b (dry);
        GrinEngine e;
        e.setGateEnabled (gate);
        e.setNoiseEnabled (noise);
        e.setSmile (automation (0.0));
        e.prepare (sr, 512);
        for (int pos = 0; pos < n; pos += 512)
        {
            const int len = jmin (512, n - pos);
            AudioBuffer<float> view (b.getArrayOfWritePointers(), 2, pos, len);
            e.setSmile (automation (pos / samplesPerBar));
            e.process (view, bpm, pos / sr * bpm / 60.0, true);
        }
        return b;
    };

    // 2 bars clean, 12-bar build to 100 %, then the drop (knob snaps back to 0) for 4 bars
    auto build = render ([] (double bar)
    {
        if (bar < 2.0)  return 0.0f;
        if (bar < 14.0) return (float) std::pow ((bar - 2.0) / 12.0, 1.3);
        return 0.0f;
    }, true, true);

    auto zero = render ([] (double) { return 0.0f; }, true, true);
    auto full = render ([] (double) { return 1.0f; }, true, true);

    int nans = 0;
    float peakBuild = 0, peakFull = 0, maxDiff = 0;
    for (int c = 0; c < 2; ++c)
        for (int i = 0; i < n; ++i)
        {
            const float a = build.getSample (c, i), f = full.getSample (c, i);
            if (! std::isfinite (a) || ! std::isfinite (f)) ++nans;
            peakBuild = jmax (peakBuild, std::abs (a));
            peakFull  = jmax (peakFull, std::abs (f));
            if (i > (int) (0.1 * sr))
                maxDiff = jmax (maxDiff, std::abs (zero.getSample (c, i) - dry.getSample (c, i)));
        }

    auto rmsDb = [&] (const AudioBuffer<float>& b, double barFrom, double barTo)
    {
        const int s0 = (int) (barFrom * samplesPerBar), s1 = jmin (n, (int) (barTo * samplesPerBar));
        double acc = 0;
        for (int c = 0; c < 2; ++c)
            for (int i = s0; i < s1; ++i) acc += b.getSample (c, i) * b.getSample (c, i);
        return Decibels::gainToDecibels ((float) std::sqrt (acc / (2.0 * (s1 - s0))));
    };

    std::printf ("NaN/Inf samples:          %d\n", nans);
    std::printf ("peak (build):             %.3f\n", peakBuild);
    std::printf ("peak (100%% held):         %.3f\n", peakFull);
    std::printf ("max |out-in| at Smile=0:  %.2e  (%.1f dB)\n", maxDiff, Decibels::gainToDecibels (maxDiff, -200.0f));
    std::printf ("RMS dry     bars 0-2: %.1f dB\n", rmsDb (dry, 0, 2));
    for (double b0 = 2; b0 < 14; b0 += 3)
        std::printf ("RMS build   bars %2.0f-%2.0f: %.1f dB (dry %.1f)\n", b0, b0 + 3, rmsDb (build, b0, b0 + 3), rmsDb (dry, b0, b0 + 3));
    std::printf ("RMS drop    bars 14-18: %.1f dB (dry %.1f)\n", rmsDb (build, 14, 18), rmsDb (dry, 14, 18));

    writeWav (dry,   sr, outDir.getChildFile ("grin_dry.wav"));
    writeWav (build, sr, outDir.getChildFile ("grin_buildup.wav"));

    return (nans == 0 && peakBuild < 1.5f && maxDiff < 1.0e-3f) ? 0 : 1;
}
