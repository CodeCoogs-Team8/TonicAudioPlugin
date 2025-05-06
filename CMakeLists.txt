cmake_minimum_required(VERSION 3.15)

# Set JUCE path
set(JUCE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/../../JUCE")
list(APPEND CMAKE_PREFIX_PATH ${JUCE_PATH})

project(Delay VERSION 1.0.0)

# Find JUCE package
find_package(JUCE REQUIRED)

# Create the plugin target
juce_add_plugin(Delay
    VERSION 1.0.0
    COMPANY_NAME "Evan Fraustro"
    IS_SYNTH FALSE
    NEEDS_MIDI_INPUT FALSE
    NEEDS_MIDI_OUTPUT FALSE
    IS_MIDI_EFFECT FALSE
    EDITOR_WANTS_KEYBOARD_FOCUS FALSE
    COPY_PLUGIN_AFTER_BUILD TRUE
    PLUGIN_MANUFACTURER_CODE Efrs
    PLUGIN_CODE Dlly
    FORMATS VST3 AU
    PRODUCT_NAME "Delay")

# Add source files
target_sources(Delay
    PRIVATE
        Source/PluginProcessor.cpp
        Source/PluginProcessor.h
        Source/PluginEditor.cpp
        Source/PluginEditor.h
        Source/ComboBoxComponent.cpp
        Source/ComboBoxComponent.h
        Source/ConnectionComponent.cpp
        Source/ConnectionComponent.h
        Source/EffectButton.cpp
        Source/EffectButton.h
        Source/EffectGraphManager.cpp
        Source/EffectGraphManager.h
        Source/EffectNodeComponent.cpp
        Source/EffectNodeComponent.h
        Source/EffectParameterComponent.cpp
        Source/EffectParameterComponent.h
        Source/EffectParametersContainer.cpp
        Source/EffectParametersContainer.h
        Source/HorizontalLevelMeter.cpp
        Source/HorizontalLevelMeter.h
        Source/KnobComponent.cpp
        Source/KnobComponent.h
        Source/LevelMeterComponent.cpp
        Source/LevelMeterComponent.h
        Source/LookandFeel.cpp
        Source/LookandFeel.h
        Source/PresetFinderComponent.cpp
        Source/PresetFinderComponent.h
        Source/ToolbarComponent.cpp
        Source/ToolbarComponent.h
        Source/TopBarComponent.cpp
        Source/TopBarComponent.h
        Source/WorkspaceComponent.cpp
        Source/WorkspaceComponent.h
)

# Add JUCE modules
target_link_libraries(Delay
    PRIVATE
        juce::juce_audio_utils
        juce::juce_audio_processors
        juce::juce_core
        juce::juce_data_structures
        juce::juce_events
        juce::juce_graphics
        juce::juce_gui_basics
        juce::juce_gui_extra
        juce::juce_opengl
        juce::juce_recommended_config_flags
        juce::juce_recommended_lto_flags
        juce::juce_recommended_warning_flags
)

# Set C++ standard
target_compile_features(Delay PRIVATE cxx_std_17)

# Set include directories
target_include_directories(Delay
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/Source
        ${CMAKE_CURRENT_SOURCE_DIR}/Source/Components
        ${CMAKE_CURRENT_SOURCE_DIR}/Source/Effects
        ${CMAKE_CURRENT_SOURCE_DIR}/Source/Graph
        ${CMAKE_CURRENT_SOURCE_DIR}/JuceLibraryCode
) 