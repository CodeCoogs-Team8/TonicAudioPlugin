cmake_minimum_required(VERSION 3.15)

project(Tonic VERSION 1.0.0)

# Set JUCE path and include it as a subdirectory
set(JUCE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/../../JUCE")

# Enable JUCE features
set(JUCE_ENABLE_MODULE_SOURCE_GROUPS ON CACHE BOOL "Enable Module Source Groups")
set(JUCE_BUILD_EXTRAS OFF CACHE BOOL "Build JUCE Extras")
set(JUCE_BUILD_EXAMPLES OFF CACHE BOOL "Build JUCE Examples")

# Add JUCE as a subdirectory
add_subdirectory(${JUCE_PATH} JUCE)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Create the plugin target
juce_add_plugin(Tonic
    VERSION 1.0.0
    COMPANY_NAME "Tonic Audio"
    BUNDLE_ID "com.tonicaudio.tonic"
    IS_SYNTH FALSE
    NEEDS_MIDI_INPUT FALSE
    NEEDS_MIDI_OUTPUT FALSE
    IS_MIDI_EFFECT FALSE
    EDITOR_WANTS_KEYBOARD_FOCUS FALSE
    COPY_PLUGIN_AFTER_BUILD TRUE
    PLUGIN_MANUFACTURER_CODE Tnca
    PLUGIN_CODE Tonc
    FORMATS VST3 AU Standalone
    PRODUCT_NAME "Tonic")

# Add source files
target_sources(Tonic
    PRIVATE
        Source/PluginProcessor.cpp
        Source/PluginProcessor.h
        Source/PluginEditor.cpp
        Source/PluginEditor.h
        Source/Components/EffectButton.cpp
        Source/Components/EffectButton.h
        Source/Components/EffectNodeComponent.cpp
        Source/Components/EffectNodeComponent.h
        Source/Components/EffectParameterComponent.cpp
        Source/Components/EffectParameterComponent.h
        Source/Components/EffectParametersContainer.cpp
        Source/Components/EffectParametersContainer.h
        Source/Components/HorizontalLevelMeter.cpp
        Source/Components/HorizontalLevelMeter.h
        Source/Components/KnobComponent.cpp
        Source/Components/KnobComponent.h
        Source/Components/LevelMeterComponent.cpp
        Source/Components/LevelMeterComponent.h
        Source/Components/LookandFeel.cpp
        Source/Components/LookandFeel.h
        Source/Components/PresetFinderComponent.cpp
        Source/Components/PresetFinderComponent.h
        Source/Components/ToolbarComponent.cpp
        Source/Components/ToolbarComponent.h
        Source/Components/TopBarComponent.cpp
        Source/Components/TopBarComponent.h
        Source/Components/WorkspaceComponent.cpp
        Source/Components/WorkspaceComponent.h
        Source/Effects/GainProcessor.cpp
        Source/Effects/GainProcessor.h
        Source/Effects/EffectRack.cpp
        Source/Effects/EffectRack.h
        Source/Effects/Chorus.cpp
        Source/Effects/Chorus.h
        Source/Effects/Distortion.cpp
        Source/Effects/Distortion.h
        Source/Effects/Delay.cpp
        Source/Effects/Delay.h
        Source/Effects/Reverb.cpp
        Source/Effects/Reverb.h
        Source/Effects/Equalizer.cpp
        Source/Effects/Equalizer.h
        Source/Graph/EffectGraphManager.cpp
        Source/Graph/EffectGraphManager.h)

# Add JUCE modules
target_link_libraries(Tonic
    PRIVATE
        juce::juce_audio_utils
        juce::juce_audio_processors
        juce::juce_gui_extra
        juce::juce_gui_basics
        juce::juce_graphics
        juce::juce_events
        juce::juce_core
        juce::juce_data_structures
        juce::juce_audio_basics
        juce::juce_audio_formats
        juce::juce_audio_devices
        juce::juce_opengl
        juce::juce_dsp
        juce::juce_recommended_config_flags
        juce::juce_recommended_lto_flags
        juce::juce_recommended_warning_flags)

# Set include directories
target_include_directories(Tonic
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/Source
        ${CMAKE_CURRENT_SOURCE_DIR}/Source/Components
        ${CMAKE_CURRENT_SOURCE_DIR}/Source/Effects
        ${CMAKE_CURRENT_SOURCE_DIR}/Source/Graph)

# Add binary resources
juce_add_binary_data(TonicBinaryData
    SOURCES
        Assets/Lato-Medium.ttf)

target_link_libraries(Tonic PRIVATE TonicBinaryData)

# Set output directories
set_target_properties(Tonic PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib"
    ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib") 