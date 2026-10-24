#!/bin/bash
# Double-click to install GRIN. Copies the plugins into your user plug-in folders
# and clears the macOS download quarantine so your DAW will load them.
cd "$(dirname "$0")"
mkdir -p ~/Library/Audio/Plug-Ins/Components ~/Library/Audio/Plug-Ins/VST3
rm -rf ~/Library/Audio/Plug-Ins/Components/GRIN.component ~/Library/Audio/Plug-Ins/VST3/GRIN.vst3
cp -R GRIN.component ~/Library/Audio/Plug-Ins/Components/
cp -R GRIN.vst3 ~/Library/Audio/Plug-Ins/VST3/
xattr -dr com.apple.quarantine ~/Library/Audio/Plug-Ins/Components/GRIN.component ~/Library/Audio/Plug-Ins/VST3/GRIN.vst3 2>/dev/null
xattr -dr com.apple.quarantine GRIN.app 2>/dev/null
killall -9 AudioComponentRegistrar >/dev/null 2>&1
echo "GRIN installed. Rescan plugins in your DAW."
