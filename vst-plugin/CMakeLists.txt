cmake_minimum_required(VERSION 3.22)

project(MetronomeVST VERSION 1.0.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

option(METRONOME_WINDOWS_CROSS "Cross-compile Windows VST3 (MinGW)" OFF)

if(METRONOME_WINDOWS_CROSS)
    set(plugin_formats VST3)
    set(copy_after_build FALSE)
else()
    set(plugin_formats VST3 AU Standalone)
    set(copy_after_build TRUE)
endif()

include(FetchContent)
FetchContent_Declare(
    JUCE
    GIT_REPOSITORY https://github.com/juce-framework/JUCE.git
    GIT_TAG        8.0.6
    GIT_SHALLOW    TRUE
)
FetchContent_MakeAvailable(JUCE)

juce_add_binary_data(MetronomeData
    HEADER_NAME BinaryData.h
    NAMESPACE BinaryData
    SOURCES
        Resources/audio/kick.wav
        Resources/audio/snare.wav
        Resources/audio/hihat_closed.wav
        Resources/audio/hihat_open.wav
        Resources/audio/crash.wav
        Resources/audio/tom.wav
)

juce_add_plugin(MetronomeVST
    COMPANY_NAME "Metronome"
    PLUGIN_MANUFACTURER_CODE Metr
    PLUGIN_CODE Mtrn
    FORMATS ${plugin_formats}
    PRODUCT_NAME "Metronome"
    IS_SYNTH FALSE
    NEEDS_MIDI_INPUT FALSE
    NEEDS_MIDI_OUTPUT FALSE
    IS_MIDI_EFFECT FALSE
    EDITOR_WANTS_KEYBOARD_FOCUS TRUE
    COPY_PLUGIN_AFTER_BUILD ${copy_after_build}
    VST3_CATEGORIES "Tools"
)

juce_generate_juce_header(MetronomeVST)

target_sources(MetronomeVST
    PRIVATE
        Source/PluginProcessor.cpp
        Source/PluginEditor.cpp
        Source/MetronomeEngine.cpp
        Source/SampleBank.cpp
        Source/BeatVisualizer.h
)

target_compile_definitions(MetronomeVST
    PUBLIC
        JUCE_WEB_BROWSER=0
        JUCE_USE_CURL=0
        JUCE_VST3_CAN_REPLACE_VST2=0
)

target_link_libraries(MetronomeVST
    PRIVATE
        MetronomeData
        juce::juce_audio_utils
        juce::juce_dsp
    PUBLIC
        juce::juce_recommended_config_flags
        juce::juce_recommended_lto_flags
        juce::juce_recommended_warning_flags
)
