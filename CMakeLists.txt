cmake_minimum_required(VERSION 3.22)
project(OTODECKS VERSION 0.0.1)

if(MINGW)
    message(FATAL_ERROR "JUCE 8 does not support MinGW. On Windows, use Visual Studio 2022 with Desktop development with C++ installed.")
endif()

set(JUCE_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../JUCE" CACHE PATH "Path to the JUCE source tree")
if(NOT EXISTS "${JUCE_SOURCE_DIR}/CMakeLists.txt")
    message(FATAL_ERROR "JUCE was not found. Place it beside this repository or configure -DJUCE_SOURCE_DIR=/path/to/JUCE")
endif()
add_subdirectory("${JUCE_SOURCE_DIR}" "${CMAKE_CURRENT_BINARY_DIR}/JUCE")

juce_add_gui_app(OtoDecks PRODUCT_NAME "OtoDecks")
juce_generate_juce_header(OtoDecks)
target_compile_features(OtoDecks PRIVATE cxx_std_17)

target_sources(OtoDecks PRIVATE
    Source/Main.cpp
    Source/MainComponent.cpp
    Source/DeckGUI.cpp
    Source/DJAudioPlayer.cpp
    Source/WaveformDisplay.cpp
    Source/PlaylistComponent.cpp
    Source/MixerComponent.cpp)

target_compile_definitions(OtoDecks PRIVATE
    JUCE_WEB_BROWSER=0
    JUCE_USE_MP3AUDIOFORMAT=1
    JUCE_USE_CURL=0
    JUCE_APPLICATION_NAME_STRING="$<TARGET_PROPERTY:OtoDecks,JUCE_PRODUCT_NAME>"
    JUCE_APPLICATION_VERSION_STRING="$<TARGET_PROPERTY:OtoDecks,JUCE_VERSION>")

target_link_libraries(OtoDecks PRIVATE
    juce::juce_audio_utils
    juce::juce_dsp
    juce::juce_recommended_config_flags
    juce::juce_recommended_lto_flags
    juce::juce_recommended_warning_flags)

option(OTODECKS_BUILD_TESTS "Build the offline audio regression tests" OFF)
if(OTODECKS_BUILD_TESTS)
    enable_testing()
    juce_add_console_app(OtoDecksAudioTests PRODUCT_NAME "OtoDecks audio tests")
    juce_generate_juce_header(OtoDecksAudioTests)
    target_compile_features(OtoDecksAudioTests PRIVATE cxx_std_17)
    target_include_directories(OtoDecksAudioTests PRIVATE Source)
    target_sources(OtoDecksAudioTests PRIVATE tests/AudioPlayerTests.cpp Source/DJAudioPlayer.cpp)
    target_compile_definitions(OtoDecksAudioTests PRIVATE JUCE_WEB_BROWSER=0 JUCE_USE_CURL=0 JUCE_USE_MP3AUDIOFORMAT=1)
    target_link_libraries(OtoDecksAudioTests PRIVATE
        juce::juce_audio_devices juce::juce_audio_formats juce::juce_dsp
        juce::juce_recommended_config_flags)
    add_test(NAME audio-regressions COMMAND OtoDecksAudioTests)
endif()
