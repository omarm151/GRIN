cmake_minimum_required(VERSION 3.22)

# Universal binary on macOS (Apple Silicon + Intel); must be set before project()
if(CMAKE_HOST_APPLE AND NOT CMAKE_OSX_ARCHITECTURES)
    set(CMAKE_OSX_ARCHITECTURES "arm64;x86_64" CACHE STRING "")
endif()
set(CMAKE_OSX_DEPLOYMENT_TARGET "11.0" CACHE STRING "")

project(GRIN VERSION 1.0.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)


# Use a local JUCE checkout if given (-DJUCE_DIR=/path/to/JUCE), otherwise download it.
if(JUCE_DIR)
    add_subdirectory(${JUCE_DIR} JUCE)
else()
    include(FetchContent)
    FetchContent_Declare(JUCE
        GIT_REPOSITORY https://github.com/juce-framework/JUCE.git
        GIT_TAG 8.0.4
        GIT_SHALLOW TRUE)
    FetchContent_MakeAvailable(JUCE)
endif()

if(GRIN_MINIMAL_LINUX)
    set(GRIN_FORMATS VST3)
else()
    set(GRIN_FORMATS AU VST3 Standalone)
endif()

juce_add_plugin(GRIN
    COMPANY_NAME "Omar Martinez"
    BUNDLE_ID "com.omarmartinez.grin"
    PLUGIN_MANUFACTURER_CODE Omrz
    PLUGIN_CODE Grin
    FORMATS ${GRIN_FORMATS}
    PRODUCT_NAME "GRIN"
    IS_SYNTH FALSE
    NEEDS_MIDI_INPUT FALSE
    NEEDS_MIDI_OUTPUT FALSE
    IS_MIDI_EFFECT FALSE
    EDITOR_WANTS_KEYBOARD_FOCUS FALSE
    AU_MAIN_TYPE kAudioUnitType_Effect
    VST3_CATEGORIES Fx
    COPY_PLUGIN_AFTER_BUILD TRUE)

target_sources(GRIN PRIVATE
    Source/PluginProcessor.cpp
    Source/PluginEditor.cpp)

target_compile_definitions(GRIN PUBLIC
    JUCE_WEB_BROWSER=0
    JUCE_USE_CURL=0
    JUCE_VST3_CAN_REPLACE_VST2=0
    JUCE_DISPLAY_SPLASH_SCREEN=0)

# Only used for compile checks on minimal Linux machines
if(GRIN_MINIMAL_LINUX)
    target_compile_definitions(GRIN PUBLIC
        JUCE_USE_XRANDR=0 JUCE_USE_XINERAMA=0 JUCE_USE_XCURSOR=0 JUCE_USE_XRENDER=0 JUCE_ALSA=0 JUCE_JACK=0)
endif()

target_link_libraries(GRIN
    PRIVATE
        juce::juce_audio_utils
        juce::juce_dsp
    PUBLIC
        juce::juce_recommended_config_flags
        juce::juce_recommended_lto_flags
        juce::juce_recommended_warning_flags)

# Offline test: renders a synthetic build-up through the engine (not needed to use the plugin)
if(GRIN_BUILD_TESTS)
    juce_add_console_app(GrinTest PRODUCT_NAME "GrinTest")
    target_sources(GrinTest PRIVATE Tests/GrinTest.cpp)
    target_include_directories(GrinTest PRIVATE Source)
    target_compile_definitions(GrinTest PRIVATE JUCE_WEB_BROWSER=0 JUCE_USE_CURL=0)
    target_link_libraries(GrinTest PRIVATE juce::juce_dsp juce::juce_audio_formats
        juce::juce_recommended_config_flags)

    juce_add_console_app(GrinSnapshot PRODUCT_NAME "GrinSnapshot")
    target_sources(GrinSnapshot PRIVATE Tests/Snapshot.cpp Source/PluginProcessor.cpp Source/PluginEditor.cpp)
    target_compile_definitions(GrinSnapshot PRIVATE JUCE_WEB_BROWSER=0 JUCE_USE_CURL=0
        JucePlugin_Name="GRIN" JUCE_STANDALONE_APPLICATION=1)
    target_link_libraries(GrinSnapshot PRIVATE juce::juce_audio_processors juce::juce_dsp
        juce::juce_recommended_config_flags)
endif()
