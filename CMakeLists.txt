cmake_minimum_required(VERSION 3.22)

project(BassCream VERSION 1.0.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_subdirectory(libs/JUCE)

# AAX builds only when AAX_SDK_PATH points at a valid Avid AAX SDK.
set(BASS_CREAM_FORMATS VST3 Standalone)
if(DEFINED ENV{AAX_SDK_PATH} AND EXISTS "$ENV{AAX_SDK_PATH}")
    juce_set_aax_sdk_path("$ENV{AAX_SDK_PATH}")
    list(APPEND BASS_CREAM_FORMATS AAX)
    message(STATUS "AAX SDK found — AAX target enabled")
else()
    message(STATUS "AAX SDK not set — building VST3 + Standalone (set AAX_SDK_PATH for Pro Tools)")
endif()

juce_add_plugin(BassCream
    COMPANY_NAME "Hannes Audio"
    COMPANY_WEBSITE "https://example.com"
    COMPANY_EMAIL "hello@example.com"
    BUNDLE_ID "com.hannesaudio.basscream"
    PLUGIN_MANUFACTURER_CODE HnAu
    PLUGIN_CODE BsCr
    FORMATS ${BASS_CREAM_FORMATS}
    PRODUCT_NAME "BassCream"
    DESCRIPTION "Creamy bass thickener — Slap & Rock modes"
    IS_SYNTH FALSE
    NEEDS_MIDI_INPUT FALSE
    NEEDS_MIDI_OUTPUT FALSE
    IS_MIDI_EFFECT FALSE
    EDITOR_WANTS_KEYBOARD_FOCUS FALSE
    COPY_PLUGIN_AFTER_BUILD FALSE
    VST3_CATEGORIES Fx Dynamics Distortion EQ
    AAX_CATEGORY AAX_ePlugInCategory_Dynamics
)

target_sources(BassCream
    PRIVATE
        Source/PluginProcessor.cpp
        Source/PluginProcessor.h
        Source/PluginEditor.cpp
        Source/PluginEditor.h
        Source/dsp/BassEngine.cpp
        Source/dsp/BassEngine.h
        Source/ui/BassCreamLookAndFeel.cpp
        Source/ui/BassCreamLookAndFeel.h
)

target_compile_definitions(BassCream
    PUBLIC
        JUCE_WEB_BROWSER=0
        JUCE_USE_CURL=0
        JUCE_VST3_CAN_REPLACE_VST2=0
        JUCE_DISPLAY_SPLASH_SCREEN=0
)

target_link_libraries(BassCream
    PRIVATE
        juce::juce_audio_utils
        juce::juce_dsp
    PUBLIC
        juce::juce_recommended_config_flags
        juce::juce_recommended_lto_flags
        juce::juce_recommended_warning_flags
)

juce_generate_juce_header(BassCream)
