cmake_minimum_required(VERSION 3.22)
# macOS: binario universal (Apple Silicon + Intel). Debe ir ANTES de project().
set(CMAKE_OSX_ARCHITECTURES "arm64;x86_64" CACHE STRING "" FORCE)
set(CMAKE_OSX_DEPLOYMENT_TARGET "10.13" CACHE STRING "" FORCE)

project(MasterPreset VERSION 0.1.0)

include(FetchContent)
FetchContent_Declare(JUCE
    GIT_REPOSITORY https://github.com/juce-framework/JUCE.git
    GIT_TAG 8.0.4
    GIT_SHALLOW TRUE)
FetchContent_MakeAvailable(JUCE)

option(COPY_AFTER_BUILD "Copiar el plugin a las carpetas del sistema tras compilar" ON)

juce_add_plugin(MasterPreset
    COMPANY_NAME "Aranguiz"
    PLUGIN_MANUFACTURER_CODE Aran
    PLUGIN_CODE Asev
    AU_MAIN_TYPE kAudioUnitType_Effect
    FORMATS VST3 AU Standalone
    PRODUCT_NAME "Aranguiz Soundevolve"
    IS_SYNTH FALSE
    NEEDS_MIDI_INPUT FALSE
    NEEDS_MIDI_OUTPUT FALSE
    COPY_PLUGIN_AFTER_BUILD ${COPY_AFTER_BUILD})

juce_add_binary_data(MasterPresetAssets SOURCES Assets/background.png)

target_sources(MasterPreset PRIVATE
    Source/PluginProcessor.cpp
    Source/PluginEditor.cpp)

target_compile_features(MasterPreset PRIVATE cxx_std_17)

target_compile_definitions(MasterPreset PUBLIC
    JUCE_WEB_BROWSER=0
    JUCE_USE_CURL=0
    JUCE_VST3_CAN_REPLACE_VST2=0)

target_link_libraries(MasterPreset
    PRIVATE MasterPresetAssets juce::juce_audio_utils juce::juce_dsp
    PUBLIC juce::juce_recommended_config_flags
           juce::juce_recommended_lto_flags
           juce::juce_recommended_warning_flags)
