
cmake_minimum_required(VERSION 3.22)

project(OTODECKS VERSION 0.0.1)

set(JUCE_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../JUCE" CACHE PATH "Path to the JUCE source tree")
if(NOT EXISTS "${JUCE_SOURCE_DIR}/CMakeLists.txt")
    message(FATAL_ERROR "JUCE was not found. Place it beside this repository or configure -DJUCE_SOURCE_DIR=/path/to/JUCE")
endif()
add_subdirectory("${JUCE_SOURCE_DIR}" "${CMAKE_CURRENT_BINARY_DIR}/JUCE")

juce_add_gui_app(OtoDecks
    # VERSION ...                       # Set this if the app version is different to the project version
    # ICON_BIG ...                      # ICON_* arguments specify a path to an image file to use as an icon
    # ICON_SMALL ...
    # DOCUMENT_EXTENSIONS ...           # Specify file extensions that should be associated with this app
    # COMPANY_NAME ...                  # Specify the name of the app's author
    PRODUCT_NAME "OtoDecks")     # The name of the final executable, which can differ from the target name

juce_generate_juce_header(OtoDecks)
target_compile_features(OtoDecks PRIVATE cxx_std_17)

target_sources(OtoDecks
    PRIVATE
        Source/Main.cpp
        Source/MainComponent.cpp
        Source/DeckGUI.cpp
        Source/DJAudioPlayer.cpp
        Source/WaveformDisplay.cpp
        Source/PlaylistComponent.cpp
        Source/MixerComponent.cpp)

target_compile_definitions(OtoDecks
    PRIVATE
        # JUCE_WEB_BROWSER and JUCE_USE_CURL would be on by default, but you might not need them.
        JUCE_WEB_BROWSER=0  # If you remove this, add `NEEDS_WEB_BROWSER TRUE` to the `juce_add_gui_app` call
        JUCE_USE_MP3AUDIOFORMAT=1
        JUCE_USE_CURL=0     # If you remove this, add `NEEDS_CURL TRUE` to the `juce_add_gui_app` call
        JUCE_APPLICATION_NAME_STRING="$<TARGET_PROPERTY:OtoDecks,JUCE_PRODUCT_NAME>"
        JUCE_APPLICATION_VERSION_STRING="$<TARGET_PROPERTY:OtoDecks,JUCE_VERSION>")


# juce_add_binary_data(GuiAppData SOURCES ...)

target_link_libraries(OtoDecks
    PRIVATE
        # GuiAppData            # If we'd created a binary data target, we'd link to it here
        juce::juce_gui_extra
        juce::juce_audio_basics
        juce::juce_audio_devices
        juce::juce_audio_formats
        juce::juce_audio_processors
        juce::juce_audio_utils
        juce::juce_dsp
    PUBLIC
        juce::juce_recommended_config_flags
        juce::juce_recommended_lto_flags
        juce::juce_recommended_warning_flags)
