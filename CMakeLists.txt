cmake_minimum_required(VERSION 3.22)

project(VeenaPlugin VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Path to JUCE — adjust if your JUCE checkout is elsewhere
set(JUCE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/../JUCE" CACHE PATH "Path to JUCE framework")
add_subdirectory(${JUCE_PATH} ${CMAKE_CURRENT_BINARY_DIR}/JUCE)

# --- Engine library (pure C++, no JUCE dependency) ---
add_library(VeenaEngine STATIC
    engine/common/Filters.cpp
    engine/waveguide/DelayLine.cpp
    engine/waveguide/PluckedString.cpp
    engine/exciter/PluckExciter.cpp
    engine/expression/PitchBendEngine.cpp
    engine/expression/MidiMapper.cpp
    engine/resonator/ModalFilter.cpp
    engine/resonator/BodyResonator.cpp
    engine/sympathetic/SympatheticBank.cpp
)

target_include_directories(VeenaEngine PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/engine
    ${CMAKE_CURRENT_SOURCE_DIR}
)

# --- JUCE Plugin target ---
juce_add_plugin(VeenaPlugin
    PRODUCT_NAME "Veena"
    COMPANY_NAME "AudioJira"
    PLUGIN_MANUFACTURER_CODE Ajir
    PLUGIN_CODE Vna1
    FORMATS VST3 Standalone
    IS_SYNTH TRUE
    NEEDS_MIDI_INPUT TRUE
    NEEDS_MIDI_OUTPUT FALSE
    IS_MIDI_EFFECT FALSE
    COPY_PLUGIN_AFTER_BUILD FALSE
)

# Plugin source files
target_sources(VeenaPlugin PRIVATE
    plugin/PluginProcessor.cpp
    plugin/PluginProcessor.h
    plugin/PluginEditor.cpp
    plugin/PluginEditor.h
    instruments/veena/VeenaVoice.cpp
    instruments/veena/VeenaVoice.h
)

target_include_directories(VeenaPlugin PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
)

# Link JUCE modules and our engine
target_link_libraries(VeenaPlugin
    PRIVATE
        VeenaEngine
        juce::juce_audio_basics
        juce::juce_audio_devices
        juce::juce_audio_formats
        juce::juce_audio_plugin_client
        juce::juce_audio_processors
        juce::juce_audio_utils
        juce::juce_core
        juce::juce_graphics
        juce::juce_gui_basics
        juce::juce_gui_extra
    PUBLIC
        juce::juce_recommended_config_flags
        juce::juce_recommended_warning_flags
)

# JUCE requires this definition
target_compile_definitions(VeenaPlugin
    PUBLIC
        JUCE_WEB_BROWSER=0
        JUCE_USE_CURL=0
        JUCE_VST3_CAN_REPLACE_VST2=0
)
