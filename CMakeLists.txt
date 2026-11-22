cmake_minimum_required(VERSION 3.22)

project(Ladderine VERSION 0.1.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(FetchContent)

# JUCE is fetched at configure time. Bump GIT_TAG to update the framework.
FetchContent_Declare(JUCE
    GIT_REPOSITORY https://github.com/juce-framework/JUCE.git
    GIT_TAG        8.0.9
    GIT_SHALLOW    ON)
FetchContent_MakeAvailable(JUCE)

juce_add_plugin(Ladderine
    COMPANY_NAME "duddex"
    COMPANY_WEBSITE "https://github.com/duddex"
    PLUGIN_MANUFACTURER_CODE Ddex
    PLUGIN_CODE Lddr
    FORMATS VST3 Standalone
    PRODUCT_NAME "Ladderine"
    DESCRIPTION "Monophonic virtual-analogue synth, inspired by classic 1970s mono synthesizers"
    IS_SYNTH TRUE
    NEEDS_MIDI_INPUT TRUE
    NEEDS_MIDI_OUTPUT FALSE
    IS_MIDI_EFFECT FALSE
    EDITOR_WANTS_KEYBOARD_FOCUS FALSE
    COPY_PLUGIN_AFTER_BUILD FALSE
    VST3_CATEGORIES Instrument Synth
    NEEDS_WEB_BROWSER FALSE
    NEEDS_CURL FALSE)

target_sources(Ladderine
    PRIVATE
        src/Parameters.cpp
        src/PluginProcessor.cpp
        src/PluginEditor.cpp
        src/engine/SynthEngine.cpp
        src/presets/FactoryPresets.cpp
        src/ui/LookAndFeel.cpp
        src/ui/Scope.cpp
        src/ui/SteppedKnob.cpp
        src/ui/Wheel.cpp)

target_include_directories(Ladderine
    PRIVATE
        src)

target_compile_definitions(Ladderine
    PUBLIC
        JUCE_WEB_BROWSER=0
        JUCE_USE_CURL=0
        JUCE_VST3_CAN_REPLACE_VST2=0
        JUCE_DISPLAY_SPLASH_SCREEN=0)

target_link_libraries(Ladderine
    PRIVATE
        juce::juce_audio_utils
    PUBLIC
        juce::juce_recommended_config_flags
        juce::juce_recommended_lto_flags
        juce::juce_recommended_warning_flags)

enable_testing()

file(MAKE_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/tests/out)

add_executable(LadderineDspTests
    tests/DspTests.cpp
    src/engine/SynthEngine.cpp)

target_include_directories(LadderineDspTests PRIVATE src)
add_test(NAME LadderineDspTests
         COMMAND LadderineDspTests ${CMAKE_CURRENT_SOURCE_DIR}/tests/out)

