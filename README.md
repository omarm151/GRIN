# GRIN — a one-knob build-up plugin

Put GRIN on a group or the master and automate the **Smile** knob from 0 to 100 % over a build.
At 0 % the plugin is fully transparent (bit-for-bit). As you turn the knob up, the effects come in one after another:

| Knob  | What comes in |
|-------|---------------|
| 0–25 %  | Saturation, the highpass starts climbing (20 Hz → ~1.6 kHz with rising resonance) |
| 15–75 % | Flanger gets faster, deeper and more metallic. A white-noise riser follows your track and its band sweeps up to ~12 kHz |
| 35 %+   | Tempo-synced gate locked to your DAW: 1/4 → 1/8 (60 %) → 1/16 (80 %) → 1/32 (93 %) |
| 55 %+   | Pitch lift that glides up to +1 octave |
| All the way | The reverb room grows, the stereo image widens, and makeup gain plus a soft clipper keep the level up |

Snap the knob back to 0 for the drop.

- **GATE** and **NOISE** buttons turn those stages off if you only want the filter and space part.
- Double-click the knob to reset it to 0.
- All three controls can be automated.

## Download a ready-made build (no compiling)

Every push to GitHub builds the Mac plugin automatically (see the **Actions** tab).
1. Go to **Actions → Build macOS plugin**, open the latest green run and download **GRIN-mac** at the bottom of the page.
2. Unzip it and double-click `install_mac.command`. If macOS blocks it, right-click it → *Open*.
3. Rescan plugins in your DAW.

## Build on your Mac yourself

1. Open Terminal in this folder (right-click the folder → *New Terminal at Folder*).
2. Run `chmod +x build_mac.sh && ./build_mac.sh`

The script installs Apple's command line tools and cmake if they're missing. It downloads JUCE, then builds a universal (Apple Silicon + Intel) AU, VST3 and standalone app. The AU and VST3 get copied into `~/Library/Audio/Plug-Ins`. The first build takes a few minutes.

Then rescan plugins in your DAW. In Logic it's under *Audio Units → Omar Martinez → GRIN*.

## Files

- `Source/GrinEngine.h`: all the DSP. The `updateCoefficients()` function maps the knob to every stage, so tweak the curves there.
- `Source/PluginProcessor.*`: parameters, host tempo and state.
- `Source/PluginEditor.*`: the UI.
- `Tests/`: an offline render and check (`-DGRIN_BUILD_TESTS=ON`). You don't need it to use the plugin.
