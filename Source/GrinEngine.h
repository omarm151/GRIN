#pragma once

#include <juce_dsp/juce_dsp.h>
#include <atomic>
#include <array>
#include <cmath>

//==============================================================================
// GRIN: a one-knob build-up engine.
//
// One "Smile" value (0..1) drives a stack of tension effects, each one with its
// own response curve, so turning a single knob takes a track from untouched
// to a full-on festival build-up. A Strength mode (Soft 1 .. Extreme) scales
// how hard every stage pushes; from Hard 1 up a tonal riser is added that is
// made from the incoming audio (tuned resonators whose pitch climbs with the
// knob), and Extreme layers a synth saw riser on top.
//
//   input -> drive -> 24 dB/oct highpass sweep -> flanger -> + noise riser
//         -> pitch lift -> + tonal riser -> tempo gate -> reverb
//         -> stereo widen -> output
//
// At Smile = 0 the engine is transparent in every mode.
//==============================================================================
class GrinEngine
{
public:
    enum Stage { Drive, Filter, Flange, Noise, Tone, Gate, Pitch, Space, NumStages };

    enum Mode { Soft1, Soft2, Medium1, Medium2, Hard1, Hard2, Extreme, NumModes };

    static const char* modeName (int m)
    {
        static const char* names[NumModes] = { "SOFT 1", "SOFT 2", "MED 1", "MED 2", "HARD 1", "HARD 2", "EXTREME" };
        return names[juce::jlimit (0, NumModes - 1, m)];
    }

    void prepare (double newSampleRate, int maxBlockSize)
    {
        sampleRate = newSampleRate;

        juce::dsp::ProcessSpec spec { sampleRate, (juce::uint32) juce::jmax (1, maxBlockSize), 2 };

        for (auto* f : { &highpass1, &highpass2 })
        {
            f->prepare (spec);
            f->setType (juce::dsp::StateVariableTPTFilterType::highpass);
        }

        noiseFilter.prepare (spec);
        noiseFilter.setType (juce::dsp::StateVariableTPTFilterType::bandpass);

        reverb.prepare (spec);

        for (auto& r : flangeLine) r.init ((int) (sampleRate * 0.02) + 4);          // 20 ms
        pitchWindow = (float) (sampleRate * 0.045);                                // 45 ms grains
        for (auto& r : pitchLine)  r.init ((int) pitchWindow + 8);
        for (auto& ch : combs)
            for (auto& c : ch) c.line.init ((int) (sampleRate / 50.0) + 8);        // down to 50 Hz

        smile.reset (sampleRate, 0.05);
        envCoefAtk = std::exp (-1.0f / (0.005f * (float) sampleRate));
        envCoefRel = std::exp (-1.0f / (0.300f * (float) sampleRate));
        gateCoef   = std::exp (-1.0f / (0.0015f * (float) sampleRate));

        reset();
    }

    void reset()
    {
        highpass1.reset();
        highpass2.reset();
        noiseFilter.reset();
        reverb.reset();
        for (auto& r : flangeLine) r.clear();
        for (auto& r : pitchLine)  r.clear();
        for (auto& ch : combs)
            for (auto& c : ch) { c.line.clear(); c.damp = 0.0f; }
        sawPhase.fill (0.0f);
        synthLp = { 0.0f, 0.0f };
        lfoPhase = 0.0f;
        pitchPhase = 0.0f;
        envelope = 0.0f;
        gateEnv = 1.0f;
        internalPpq = 0.0;
        smile.setCurrentAndTargetValue (smile.getTargetValue());
        modeCur = modeTable (currentMode);
        updateCoefficients (smile.getCurrentValue());
        lastCoefSmile = smile.getCurrentValue();
    }

    void setSmile (float v)       { smile.setTargetValue (juce::jlimit (0.0f, 1.0f, v)); }
    void setGateEnabled (bool b)  { if (b != gateEnabled)  { gateEnabled = b;  dirty = true; } }
    void setNoiseEnabled (bool b) { if (b != noiseEnabled) { noiseEnabled = b; dirty = true; } }
    void setMode (int m)
    {
        m = juce::jlimit (0, NumModes - 1, m);
        if (m != currentMode) { currentMode = m; dirty = true; }
    }

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
        const float invSr = 1.0f / (float) sampleRate;

        for (int i = 0; i < numSamples; ++i)
        {
            const float sNow = smile.getNextValue();
            if ((i & 15) == 0 && (std::abs (sNow - lastCoefSmile) > 1.0e-6f || dirty || modeMoving))
            {
                updateCoefficients (sNow);
                lastCoefSmile = sNow;
                dirty = false;
            }

            float x[2] = { L[i], R != nullptr ? R[i] : L[i] };

            // envelope of the incoming signal (the added layers follow the track)
            const float inLevel = 0.5f * (std::abs (x[0]) + std::abs (x[1]));
            const float ec = inLevel > envelope ? envCoefAtk : envCoefRel;
            envelope = inLevel + ec * (envelope - inLevel);
            const float follow = juce::jmin (1.0f, envelope * 4.0f);

            lfoPhase += flangeRate * invSr;
            if (lfoPhase >= 1.0f) lfoPhase -= 1.0f;

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

            const float noiseScale = noiseLevel * follow;
            const float toneScale  = toneLevel * follow;
            const float synthScale = synthLevel * follow;

            // synth saw riser (Extreme): three detuned polyBLEP saws, shared by both channels
            float synth[2] = { 0.0f, 0.0f };
            if (synthScale > 0.00001f)
            {
                static constexpr float detune[3] = { 0.993f, 1.0f, 1.007f };
                for (int v = 0; v < 3; ++v)
                {
                    const float dt = toneFreq * detune[v] * invSr;
                    float& ph = sawPhase[(size_t) v];
                    ph += dt;
                    if (ph >= 1.0f) ph -= 1.0f;
                    const float saw = 2.0f * ph - 1.0f - polyBlep (ph, dt);
                    synth[0] += saw * (v == 2 ? 0.6f : 1.0f);
                    synth[1] += saw * (v == 0 ? 0.6f : 1.0f);
                }
            }

            for (int c = 0; c < 2; ++c)
            {
                float s = x[c];

                // 1. drive
                if (driveMix > 0.0f)
                {
                    const float d = std::tanh (s * driveAmount) * driveNorm;
                    s += driveMix * (d - s);
                }

                // 2. highpass sweep, 24 dB/oct
                if (hpMix > 0.0f)
                {
                    const float h = highpass2.processSample (c, highpass1.processSample (c, s));
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
                    s += noiseFilter.processSample (c, random.nextFloat() * 2.0f - 1.0f) * noiseScale;

                // 5. pitch lift (two crossfaded grains reading a delay line)
                pitchLine[(size_t) c].push (s);
                if (pitchMix > 0.0f)
                {
                    float p2 = pitchPhase + 0.5f;
                    if (p2 >= 1.0f) p2 -= 1.0f;
                    const float w1 = std::sin (juce::MathConstants<float>::pi * pitchPhase);
                    const float w2 = std::sin (juce::MathConstants<float>::pi * p2);
                    const float shifted = pitchLine[(size_t) c].read ((1.0f - pitchPhase) * pitchWindow) * w1 * w1
                                        + pitchLine[(size_t) c].read ((1.0f - p2) * pitchWindow) * w2 * w2;
                    s += pitchMix * (shifted - s);
                }

                // 6. tonal riser: the (already filtered) track excites tuned feedback combs
                //    on root / fifth / octave whose pitch climbs with the knob
                if (toneScale > 0.00001f)
                {
                    const float excite = s + 0.03f * (random.nextFloat() * 2.0f - 1.0f) * follow;
                    const float spread = c == 0 ? 1.0f : 1.003f;
                    float ring = 0.0f;
                    for (int k = 0; k < 3; ++k)
                    {
                        auto& cb = combs[(size_t) c][(size_t) k];
                        const float delay = (float) sampleRate / (toneFreq * toneRatios[k] * spread);
                        const float out = cb.line.read (delay - 1.0f);
                        cb.damp += toneDampCoef * (out - cb.damp);              // one-pole lowpass in the loop
                        cb.line.push (excite + toneFeedback * cb.damp);
                        ring += out * toneGains[k];
                    }
                    s += ring * toneNorm * toneScale;
                }

                if (synthScale > 0.00001f)
                {
                    synthLp[(size_t) c] += 0.35f * (synth[c] - synthLp[(size_t) c]);
                    s += synthLp[(size_t) c] * 0.12f * synthScale;
                }

                // 7. gate
                x[c] = s * gateGain;
            }

            L[i] = x[0];
            if (R != nullptr) R[i] = x[1];
        }

        internalPpq = ppq;

        // 8. reverb (block based, has its own internal parameter smoothing)
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

        // 9. stereo widen + 10. output gain / safety clip
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
        stageLevel[Drive]  = driveMix * juce::jmin (1.0f, sNow * 1.5f) * juce::jmin (1.0f, modeCur.drive);
        stageLevel[Filter] = hpMix * std::pow (sNow, 1.2f);
        stageLevel[Flange] = flangeMix / 0.8f;
        stageLevel[Noise]  = noiseLevel / 0.28f;
        stageLevel[Tone]   = juce::jmax (toneLevel, synthLevel);
        stageLevel[Gate]   = gateDepth;
        stageLevel[Pitch]  = pitchMix / 0.4f;
        stageLevel[Space]  = revWet / 0.24f;
        gateMeter = currentGateGain();
        gateDivMeter = gateDepth > 0.0f ? (float) gateDivision : 0.0f;
    }

    float getStageLevel (int s) const  { return stageLevel[(size_t) s].load(); }
    float getGateMeter() const         { return gateMeter.load(); }
    float getGateDivisionBeats() const { return gateDivMeter.load(); }

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

    struct Comb { Ring line; float damp = 0.0f; };

    // How hard each stage pushes in each strength mode. 1.0 = the "Medium 2" reference.
    struct ModeParams
    {
        float drive, hpTopHz, hpRes, flange, noise, gate, pitch, space, tone, synth;
    };

    static ModeParams modeTable (int m)
    {
        //                      drive  hpTop   hpRes flange noise gate  pitch space tone  synth
        static const ModeParams t[NumModes] = {
            /* Soft 1  */ { 0.30f, 3000.f, 0.5f, 0.35f, 0.55f, 0.0f, 0.0f, 0.75f, 0.0f, 0.0f },
            /* Soft 2  */ { 0.45f, 3600.f, 0.6f, 0.50f, 0.70f, 0.5f, 0.0f, 0.85f, 0.0f, 0.0f },
            /* Med 1   */ { 0.65f, 4500.f, 0.8f, 0.70f, 0.85f, 0.7f, 0.5f, 0.90f, 0.0f, 0.0f },
            /* Med 2   */ { 0.80f, 5500.f, 1.0f, 0.85f, 1.00f, 0.85f, 0.8f, 1.00f, 0.0f, 0.0f },
            /* Hard 1  */ { 0.95f, 6500.f, 1.1f, 1.00f, 1.05f, 0.95f, 1.0f, 1.00f, 0.6f, 0.0f },
            /* Hard 2  */ { 1.10f, 7500.f, 1.2f, 1.10f, 1.10f, 1.0f, 1.0f, 1.05f, 0.85f, 0.0f },
            /* Extreme */ { 1.40f, 9000.f, 1.3f, 1.20f, 1.20f, 1.0f, 1.15f, 1.10f, 1.0f, 1.0f },
        };
        return t[juce::jlimit (0, NumModes - 1, m)];
    }

    static float polyBlep (float t, float dt)
    {
        if (t < dt)        { t /= dt;               return t + t - t * t - 1.0f; }
        if (t > 1.0f - dt) { t = (t - 1.0f) / dt;   return t * t + t + t + 1.0f; }
        return 0.0f;
    }

    float currentGateGain() const { return 1.0f - gateDepth * (1.0f - gateEnv); }

    // Maps the single Smile value (and the strength mode) onto every stage.
    // Each curve is shaped so the effects arrive in sequence instead of all at once.
    void updateCoefficients (float s)
    {
        // glide between modes so switching mid-build doesn't click
        const auto target = modeTable (currentMode);
        auto glide = [] (float& cur, float tgt) { cur += 0.04f * (tgt - cur); return std::abs (tgt - cur) > 1.0e-4f; };
        bool moving = false;
        moving |= glide (modeCur.drive,   target.drive);
        moving |= glide (modeCur.hpTopHz, target.hpTopHz) && std::abs (target.hpTopHz - modeCur.hpTopHz) > 1.0f;
        moving |= glide (modeCur.hpRes,   target.hpRes);
        moving |= glide (modeCur.flange,  target.flange);
        moving |= glide (modeCur.noise,   target.noise);
        moving |= glide (modeCur.gate,    target.gate);
        moving |= glide (modeCur.pitch,   target.pitch);
        moving |= glide (modeCur.space,   target.space);
        moving |= glide (modeCur.tone,    target.tone);
        moving |= glide (modeCur.synth,   target.synth);
        modeMoving = moving;
        const auto& m = modeCur;

        // drive: arrives early, gets heavy at the top
        driveMix    = juce::jlimit (0.0f, 1.0f, s * 4.0f);
        driveAmount = 1.0f + 7.0f * m.drive * s * s;
        driveNorm   = 1.0f / std::sqrt (driveAmount);

        // highpass: 20 Hz -> mode ceiling (3-9 kHz). Slow start, then it races up
        // to leave a razor-thin top end. Resonance rises on the second stage.
        hpMix = juce::jlimit (0.0f, 1.0f, s * 8.0f);
        const float hpCut = juce::jmin ((float) sampleRate * 0.45f,
                                        20.0f * std::pow (m.hpTopHz / 20.0f, std::pow (s, 1.5f)));
        highpass1.setCutoffFrequency (hpCut);
        highpass2.setCutoffFrequency (hpCut);
        highpass1.setResonance (0.54f);
        highpass2.setResonance (1.3f + 1.4f * m.hpRes * s * s);

        // flanger
        flangeMix      = juce::jmin (0.8f, 0.8f * m.flange * smoothstep (0.15f, 0.75f, s));
        flangeRate     = 0.12f + 2.8f * s * s;
        flangeDepthMs  = 0.8f + 3.2f * s;
        flangeFeedback = juce::jmin (0.85f, 0.75f * s * m.flange);

        // noise riser: white noise through a rising bandpass
        const float nT = smoothstep (0.1f, 1.0f, s);
        noiseLevel = noiseEnabled ? 0.24f * m.noise * std::pow (nT, 1.5f) : 0.0f;
        noiseFilter.setCutoffFrequency (juce::jmin ((float) sampleRate * 0.45f, 300.0f * std::pow (40.0f, s)));
        noiseFilter.setResonance (1.1f + 1.5f * s);

        // tonal riser (Hard / Extreme): pitch climbs 3 octaves, C3 -> C6
        const float tT = smoothstep (0.2f, 1.0f, s);
        toneLevel    = m.tone * std::pow (tT, 1.2f);
        synthLevel   = m.synth * std::pow (smoothstep (0.35f, 1.0f, s), 1.5f);
        toneFreq     = 130.81f * std::pow (2.0f, 3.0f * tT);
        toneFeedback = 0.92f + 0.06f * tT;
        toneDampCoef = 0.55f;
        toneNorm     = 3.2f * (1.0f - toneFeedback) / (1.0f - toneFeedback * 0.9f);

        // tempo gate: 1/4 -> 1/8 -> 1/16 -> 1/32
        if (gateEnabled && m.gate > 0.01f && s >= 0.35f)
        {
            const float t = (s - 0.35f) / 0.65f;
            gateDepth = juce::jlimit (0.0f, 1.0f, (s - 0.35f) / 0.05f) * (0.3f + 0.55f * t) * m.gate;
            gateDivision = s < 0.6f ? 1.0 : s < 0.8f ? 0.5 : s < 0.93f ? 0.25 : 0.125;
        }
        else
        {
            gateDepth = 0.0f;
        }

        // pitch lift: last stretch of the knob, glides up to +12 semitones
        const float pT = smoothstep (0.55f, 1.0f, s);
        pitchMix = 0.35f * pT * m.pitch;
        pitchStep = (std::pow (2.0f, pT) - 1.0f) / pitchWindow;

        // reverb: room grows and gets brighter
        revWet     = 0.22f * m.space * std::pow (s, 1.3f);
        revDry     = 0.5f * (1.0f - 0.3f * s);                     // juce::Reverb dry scale = 2
        revRoom    = juce::jmin (0.98f, 0.45f + 0.53f * s * m.space);
        revDamping = 0.55f - 0.4f * s;

        // stereo + output: some makeup, but the thinning is meant to be heard
        sideGain = 1.0f + 0.7f * s;
        outGain  = juce::Decibels::decibelsToGain (9.0f * std::pow (s, 1.5f));
        clipMix  = juce::jlimit (0.0f, 1.0f, s * 5.0f);
    }

    double sampleRate = 44100.0;
    juce::SmoothedValue<float> smile { 0.0f };
    bool gateEnabled = true, noiseEnabled = true, dirty = true, modeMoving = false;
    int currentMode = Medium1;
    ModeParams modeCur = modeTable (Medium1);
    float lastCoefSmile = -1.0f;

    juce::dsp::StateVariableTPTFilter<float> highpass1, highpass2, noiseFilter;
    juce::dsp::Reverb reverb;
    juce::Random random;

    std::array<Ring, 2> flangeLine, pitchLine;
    std::array<std::array<Comb, 3>, 2> combs;
    static constexpr float toneRatios[3] = { 1.0f, 1.5f, 2.0f };
    static constexpr float toneGains[3]  = { 1.0f, 0.7f, 0.5f };
    std::array<float, 3> sawPhase { 0.0f, 0.33f, 0.66f };
    std::array<float, 2> synthLp { 0.0f, 0.0f };

    float lfoPhase = 0.0f, pitchPhase = 0.0f, pitchWindow = 2000.0f, pitchStep = 0.0f;
    float envelope = 0.0f, envCoefAtk = 0.0f, envCoefRel = 0.0f;
    float gateEnv = 1.0f, gateCoef = 0.0f;
    double internalPpq = 0.0, gateDivision = 1.0;

    float driveMix = 0, driveAmount = 1, driveNorm = 1, hpMix = 0;
    float flangeMix = 0, flangeRate = 0.1f, flangeDepthMs = 1, flangeFeedback = 0;
    float noiseLevel = 0, gateDepth = 0, pitchMix = 0;
    float toneLevel = 0, synthLevel = 0, toneFreq = 130.81f, toneFeedback = 0.92f, toneDampCoef = 0.55f, toneNorm = 0.2f;
    float revWet = 0, revDry = 0.5f, revRoom = 0.5f, revDamping = 0.5f;
    float sideGain = 1, outGain = 1, clipMix = 0;

    std::array<std::atomic<float>, NumStages> stageLevel {};
    std::atomic<float> gateMeter { 1.0f }, gateDivMeter { 0.0f };
};
