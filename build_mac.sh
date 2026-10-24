#!/bin/bash
# Builds GRIN as AU + VST3 (+ a standalone app) and installs the plugins into
# ~/Library/Audio/Plug-Ins. Run from this folder:   ./build_mac.sh
set -e
cd "$(dirname "$0")"

echo "== GRIN build =="

if ! xcode-select -p >/dev/null 2>&1; then
  echo "Apple's command line tools are missing. A dialog will open to install them."
  echo "Run ./build_mac.sh again once the install finishes."
  xcode-select --install || true
  exit 1
fi

if ! command -v cmake >/dev/null 2>&1; then
  if command -v brew >/dev/null 2>&1; then
    echo "Installing cmake with Homebrew..."
    brew install cmake
  else
    echo "cmake is needed. Install Homebrew from https://brew.sh then run: brew install cmake"
    echo "(or download CMake.app from https://cmake.org/download and run:"
    echo "  sudo \"/Applications/CMake.app/Contents/bin/cmake-gui\" --install )"
    exit 1
  fi
fi

# First run downloads JUCE (~100 MB) automatically.
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release -j "$(sysctl -n hw.ncpu)"

# Make Logic / GarageBand notice the new Audio Unit
killall -9 AudioComponentRegistrar >/dev/null 2>&1 || true

echo
echo "Done. Installed:"
ls -d ~/Library/Audio/Plug-Ins/Components/GRIN.component ~/Library/Audio/Plug-Ins/VST3/GRIN.vst3 2>/dev/null || true
echo "Standalone app: build/GRIN_artefacts/Release/Standalone/GRIN.app"
echo "Rescan plugins in your DAW and look for 'GRIN' by Omar Martinez."
