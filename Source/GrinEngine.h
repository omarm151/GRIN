#pragma once

#include <juce_dsp/juce_dsp.h>
#include <atomic>
#include <array>
#include <cmath>

//==============================================================================
// GRIN — a one-knob build-up engine.
//
// One "Smile" value (0..1) drives a stack of tension effects, each one with its
// own response curve, so turning a single knob takes a track from untouched
// to a full-on festival build-up:
//
//   input -> drive -> highpass sweep -> flanger -> + noise riser
//         -> pitch lift -> tempo gate -> reverb -> stereo widen -> output
//
// At Smile = 0 the engine is transparent.
//==============================================================================
class GrinEngine
{
public:
    enum Stage { Drive, Filter, Flange, Noise, Gate, Pitch, Space, NumStages };

    void prepare (double newSampleRate, int maxBlockSize)
    {
        sampleRate = newSampleRate;

        juce::dsp::ProcessSpec spec { sampleRate, (juce::uint32) juce::jmax (1, maxBlockSize), 2 };

        highpass.prepare (spec);
        highpass.setType (juce::dsp::StateVariableTPTFilterType::highpass);

        noiseFilter.prepare (spec);
        noiseFilter.setType (juce::dsp::StateVariableTPTFilterType::bandpass);

        reverb.prepare (spec);

        for (auto& r : flangeLine) r.init ((int) (sampleRate * 0.02) + 4);          // 20 ms
        pitchWindow = (float) (sampleRate * 0.045);                                // 45 ms grains
        for (auto& r : pitchLine)  r.init ((int) pitchWindow + 8);

        smile.reset (sampleRate, 0.05);
        envCoefAtk = std::exp (-1.0f / (0.005f * (float) sampleRate));
        envCoefRel = std::exp (-1.0f / (0.300f * (float) sampleRate));
        gateCoef   = std::exp (-1.0f / (0.0015f * (float) sampleRate));

        reset();
    }

    void reset()
    {
        highpass.reset();
        noiseFilter.reset();
        reverb.reset();
        for (auto& r : flangeLine) r.clear();
        for (auto& r : pitchLine)  r.clear();
        flangeFb = { 0.0f, 0.0f };
        lfoPhase = 0.0f;
        pitchPhase = 0.0f;
        envelope = 0.0f;
        gateEnv = 1.0f;
        internalPpq = 0.0;
        smile.setCurrentAndTargetValue (smile.getTargetValue());
        updateCoefficients (smile.getCurrentValue());
        lastCoefSmile = smile.getCurrentValue();
    }

    void setSmile (float v)       { smile.setTargetValue (juce::jlimit (0.0f, 1.0f, v)); }
    void setGateEnabled (bool b)  { if (b != gateEnabled)  { gateEnabled = b;  togglesChanged = true; } }
    void setNoiseEnabled (bool b) { if (b != noiseEnabled) { noiseEnabled = b; togglesChanged = true; } }

    // bpm <= 0 means unknown (falls back to 128). If !hostPlaying an internal clock runs.
    void process (juce::AudioBuffer<float>& buffer, double bpm, double ppqAtBlockStart, bool hostPlaying)
    {
        const int numSamples  = buffer.getNumSamples();
        const int numChannels = juce::jmin (2, buffer.getNumChannels());
        if (numSamples == 0 || numChannels == 0)
            return;

        float* L = buffer.getWritePointer (0);
        float* R = numChannels > 1 ? buffer.getWritePointer (1) : nullptr;

        const double tempo = bpm > 20.0 ? bpm : 128.0;
        const double ppqPerSample = tempo / (60.0 * sampleRate);
        double ppq = hostPlaying ? ppqAtBlockStart : internalPpq;

        for (int i = 0; i < numSamples; ++i)
        {
            const float sNow = smile.getNextValue();
            if ((i & 15) == 0 && (std::abs (sNow - lastCoefSmile) > 1.0e-6f || togglesChanged))
            {
                updateCoefficients (sNow);
                lastCoefSmile = sNow;
                togglesChanged = false;
            }

            float x[2] = { L[i], R != nullptr ? R[i] : L[i] };

            // envelope of the incoming signal (used to make the noise follow the track)
            const float inLevel = 0.5f * (std::abs (x[0]) + std::abs (x[1]));
            const float ec = inLevel > envelope ? envCoefAtk : envCoefRel;
            envelope = inLevel + ec * (envelope - inLevel);

            // LFO for the flanger
            lfoPhase += flangeRate / (float) sampleRate;
            if (lfoPhase >= 1.0f) lfoPhase -= 1.0f;

            // pitch-shifter phasor
            pitchPhase += pitchStep;
            if (pitchPhase >= 1.0f) pitchPhase -= 1.0f;
            if (pitchPhase < 0.0f)  pitchPhase += 1.0f;

            // tempo gate
            float gateTarget = 1.0f;
            if (gateDepth > 0.0f)
            {
                const double ph = std::fmod (ppq / gateDivision, 1.0);
                gateTarget = ph < 0.55 ? 1.0f : 0.0f;
            }
            gateEnv = gateTarget + gateCoef * (gateEnv - gateTarget);
            const float gateGain = 1.0f - gateDepth * (1.0f - gateEnv);
            ppq += ppqPerSample;

            const float noiseScale = noiseLevel * juce::jmin (1.0f, envelope * 4.0f);

            for (int c = 0; c < 2; ++c)
            {
                float s = x[c];

                // 1. drive
                if (driveMix > 0.0f)
                {
                    const float d = std::tanh (s * driveAmount) * driveNorm;
                    s += driveMix * (d - s);
                }

                // 2. highpass sweep
                if (hpMix > 0.0f)
                {
                    const float h = highpass.processSample (c, s);
                    s += hpMix * (h - s);
                }

                // 3. flanger
                {
                    float lp = lfoPhase + (c == 1 ? 0.25f : 0.0f);
                    if (lp >= 1.0f) lp -= 1.0f;
                    const float lfo = 0.5f + 0.5f * std::sin (juce::MathConstants<float>::twoPi * lp);
                    const float delayMs = 0.6f + flangeDepthMs * lfo;
                    const float tap = flangeLine[(size_t) c].read (delayMs * 0.001f * (float) sampleRate);
                    flangeLine[(size_t) c].push (s + flangeFeedback * tap);
                    if (flangeMix > 0.0f)
                        s += flangeMix * (0.5f * (s + tap) - s);
                }

                // 4. noise riser
                if (noiseScale > 0.00001f)
                {
                    const float n = noiseFilter.processSample (c, random.nextFloat() * 2.0f - 1.0f);
                    s += n * noiseScale;
                }

                // 5. pitch lift (two crossfaded grains reading a delay line)
                pitchLine[(size_t) c].push (s);
                if (pitchMix > 0.0f)
                {
                    float p2 = pitchPhase + 0.5f;
                    if (p2 >= 1.0f) p2 -= 1.0f;
                    const float d1 = (1.0f - pitchPhase) * pitchWindow;
                    const float d2 = (1.0f - p2) * pitchWindow;
                    const float w1 = std::sin (juce::MathConstants<float>::pi * pitchPhase);
                    const float w2 = std::sin (juce::MathConstants<float>::pi * p2);
                    const float shifted = pitchLine[(size_t) c].read (d1) * w1 * w1
                                        + pitchLine[(size_t) c].read (d2) * w2 * w2;
                    s += pitchMix * (shifted - s);
                }

                // 6. gate
                x[c] = s * gateGain;
            }

            L[i] = x[0];
            if (R != nullptr) R[i] = x[1];
        }

        internalPpq = ppq;

        // 7. reverb (block based, has its own internal parameter smoothing)
        juce::Reverb::Parameters rp;
        rp.roomSize   = revRoom;
        rp.damping    = revDamping;
        rp.wetLevel   = revWet;
        rp.dryLevel   = revDry;
        rp.width      = 1.0f;
        rp.freezeMode = 0.0f;
        reverb.setParameters (rp);

        auto block = juce::dsp::AudioBlock<float> (buffer).getSubsetChannelBlock (0, (size_t) numChannels);
        reverb.process (juce::dsp::ProcessContextReplacing<float> (block));

        // 8. stereo widen + 9. output gain / safety clip
        for (int i = 0; i < numSamples; ++i)
        {
            float l = L[i], r = R != nullptr ? R[i] : L[i];

            if (R != nullptr)
            {
                const float m = 0.5f * (l + r);
                const float sd = 0.5f * (l - r) * sideGain;
                l = m + sd;
                r = m - sd;
            }

            l *= outGain;
            r *= outGain;

            if (clipMix > 0.0f)
            {
                l += clipMix * (std::tanh (l) - l);
                r += clipMix * (std::tanh (r) - r);
            }

            L[i] = l;
            if (R != nullptr) R[i] = r;
        }

        // meters for the UI
        const float sNow = smile.getCurrentValue();
        stageLevel[Drive]  = driveMix * juce::jmin (1.0f, sNow * 1.5f);
        stageLevel[Filter] = hpMix * std::pow (sNow, 1.2f);
        stageLevel[Flange] = flangeMix / 0.8f;
        stageLevel[Noise]  = noiseLevel / 0.24f;
        stageLevel[Gate]   = gateDepth;
        stageLevel[Pitch]  = pitchMix / 0.35f;
        stageLevel[Space]  = revWet / 0.22f;
        gateMeter = currentGateGain();
    }

    float getStageLevel (int s) const { return stageLevel[(size_t) s].load(); }
    float getGateMeter() const        { return gateMeter.load(); }
    float getGateDivisionBeats() const { return gateDepth > 0.0f ? (float) gateDivision : 0.0f; }

    static float smoothstep (float e0, float e1, float x)
    {
        const float t = juce::jlimit (0.0f, 1.0f, (x - e0) / (e1 - e0));
        return t * t * (3.0f - 2.0f * t);
    }

private:
    //==========================================================================
    struct Ring
    {
        std::vector<float> data;
        int write = 0, size = 1;

        void init (int n) { size = juce::jmax (4, n); data.assign ((size_t) size, 0.0f); write = 0; }
        void clear()      { std::fill (data.begin(), data.end(), 0.0f); write = 0; }
        void push (float v) { data[(size_t) write] = v; if (++write >= size) write = 0; }

        // delay in samples, 0 = most recently pushed sample
        float read (float delay) const
        {
            delay = juce::jlimit (0.0f, (float) (size - 2), delay);
            float rp = (float) write - 1.0f - delay;
            while (rp < 0.0f) rp += (float) size;
            const int i0 = juce::jmin (size - 1, (int) rp);
            const int i1 = i0 + 1 >= size ? 0 : i0 + 1;
            const float f = rp - (float) i0;
            return data[(size_t) i0] + f * (data[(size_t) i1] - data[(size_t) i0]);
        }
    };

    float currentGateGain() const { return 1.0f - gateDepth * (1.0f - gateEnv); }

    // Maps the single Smile value onto every stage. Each curve is shaped so the
    // effects arrive in sequence instead of all at once.
    void updateCoefficients (float s)
    {
        // drive: arrives early, gets heavy at the top
        driveMix    = juce::jlimit (0.0f, 1.0f, s * 4.0f);
        driveAmount = 1.0f + 7.0f * s * s;
        driveNorm   = 1.0f / std::sqrt (driveAmount);

        // highpass: 20 Hz -> ~1.6 kHz, resonance rises
        hpMix = juce::jlimit (0.0f, 1.0f, s * 8.0f);
        highpass.setCutoffFrequency (20.0f * std::pow (80.0f, std::pow (s, 1.4f)));
        highpass.setResonance (0.707f + 1.6f * s * s);

        // flanger: fades in from ~15 %, gets faster and more metallic
        flangeMix      = 0.8f * smoothstep (0.15f, 0.75f, s);
        flangeRate     = 0.12f + 2.8f * s * s;
        flangeDepthMs  = 0.8f + 3.2f * s;
        flangeFeedback = 0.78f * s;

        // noise riser: white noise through a rising bandpass
        const float nT = smoothstep (0.1f, 1.0f, s);
        noiseLevel = noiseEnabled ? 0.24f * std::pow (nT, 1.5f) : 0.0f;
        noiseFilter.setCutoffFrequency (juce::jmin ((float) sampleRate * 0.45f, 300.0f * std::pow (40.0f, s)));
        noiseFilter.setResonance (1.1f + 1.5f * s);

        // tempo gate: 1/4 -> 1/8 -> 1/16 -> 1/32
        if (gateEnabled && s >= 0.35f)
        {
            const float t = (s - 0.35f) / 0.65f;
            gateDepth = juce::jlimit (0.0f, 1.0f, (s - 0.35f) / 0.05f) * (0.3f + 0.55f * t);
            gateDivision = s < 0.6f ? 1.0 : s < 0.8f ? 0.5 : s < 0.93f ? 0.25 : 0.125;
        }
        else
        {
            gateDepth = 0.0f;
        }

        // pitch lift: last stretch of the knob, glides up to +12 semitones
        const float pT = smoothstep (0.55f, 1.0f, s);
        pitchMix = 0.35f * pT;
        const float ratio = std::pow (2.0f, pT);                  // up to one octave
        pitchStep = (ratio - 1.0f) / pitchWindow;

        // reverb: room grows and gets brighter
        revWet     = 0.22f * std::pow (s, 1.3f);
        revDry     = 0.5f * (1.0f - 0.3f * s);                     // juce::Reverb dry scale = 2
        revRoom    = 0.45f + 0.53f * s;
        revDamping = 0.55f - 0.4f * s;

        // stereo + output
        sideGain = 1.0f + 0.7f * s;
        outGain  = juce::Decibels::decibelsToGain (11.0f * std::pow (s, 1.5f));   // makeup for the highpass + gate
        clipMix  = juce::jlimit (0.0f, 1.0f, s * 5.0f);
    }

    double sampleRate = 44100.0;
    juce::SmoothedValue<float> smile { 0.0f };
    bool gateEnabled = true, noiseEnabled = true, togglesChanged = true;
    float lastCoefSmile = -1.0f;

    juce::dsp::StateVariableTPTFilter<float> highpass, noiseFilter;
    juce::dsp::Reverb reverb;
    juce::Random random;

    std::array<Ring, 2> flangeLine, pitchLine;
    std::array<float, 2> flangeFb { 0.0f, 0.0f };

    float lfoPhase = 0.0f, pitchPhase = 0.0f, pitchWindow = 2000.0f, pitchStep = 0.0f;
    float envelope = 0.0f, envCoefAtk = 0.0f, envCoefRel = 0.0f;
    float gateEnv = 1.0f, gateCoef = 0.0f;
    double internalPpq = 0.0, gateDivision = 1.0;

    float driveMix = 0, driveAmount = 1, driveNorm = 1, hpMix = 0;
    float flangeMix = 0, flangeRate = 0.1f, flangeDepthMs = 1, flangeFeedback = 0;
    float noiseLevel = 0, gateDepth = 0, pitchMix = 0;
    float revWet = 0, revDry = 0.5f, revRoom = 0.5f, revDamping = 0.5f;
    float sideGain = 1, outGain = 1, clipMix = 0;

    std::array<std::atomic<float>, NumStages> stageLevel {};
    std::atomic<float> gateMeter { 1.0f };
};
