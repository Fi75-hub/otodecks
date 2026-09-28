# OtoDecks

A desktop DJ mixing application written in C++ with JUCE. Two audio decks, a central mixer and a searchable track library bring together object-oriented UI design, audio playback and signal processing.

Developed by Faizan Ilyas as the Object Oriented Programming midterm project. This application is separate from the trading simulator developed for the same module.

## Features

- Two decks with playback, speed, volume and waveform controls.
- Eight hot-cue slots per deck, with per-track cue persistence.
- Three-band EQ, a combined low/high-pass filter, reverb and echo.
- Crossfader and channel controls with audio level indicators.
- Energy-based BPM estimation and a speed-matching sync control.
- A searchable audio library with track durations and deck-loading buttons.
- Local CSV persistence for the library and cues.
- A resizable window with fullscreen controls.

## Dependencies

- A desktop C++ toolchain supported by the chosen JUCE release.
- [CMake](https://cmake.org/download/) 3.22 or later.
- The [JUCE source tree](https://github.com/juce-framework/JUCE), including its modules and build scripts.

JUCE is an external dependency and is not bundled. The original archive does not record a tested JUCE release. Its Projucer configuration includes `juce_audio_processors_headless`, so a recent JUCE 8 source tree is the starting point for that route; compatibility must be checked by building. CMake is the primary documented build route.

## Build with CMake

Place JUCE next to this repository:

```text
workspace/
  JUCE/
    CMakeLists.txt
    modules/
  otodecks/
    CMakeLists.txt
    Source/
```

From `otodecks`, configure and build:

```sh
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release --target OtoDecks
```

If JUCE is elsewhere, pass its source directory during configuration:

```sh
cmake -S . -B build -DJUCE_SOURCE_DIR="/path/to/JUCE" -DCMAKE_BUILD_TYPE=Release
```

On Windows, run these commands with a Visual Studio C++ toolchain installed; CMake can use an installed Visual Studio generator. Generated binaries are under `build/OtoDecks_artefacts/`, with configuration-specific subfolders depending on the generator.

CMake generates `JuceHeader.h` for the application. Source files include this generated header through the build system's include paths; no hand-written copy is required.

### Projucer alternative

`OtoDecks.jucer` is retained from the original project. Open it in a compatible Projucer version, confirm that the JUCE module paths point to your installation, select an exporter suitable for your system and save the project before opening the generated IDE project. The stored paths use `../JUCE/modules`. Generated `Builds/` and `JuceLibraryCode/` directories are intentionally excluded from version control.

## Use the application

1. Add your own audio files with **+ Add Files**, or use a deck's **LOAD** control. The library chooser lists MP3, WAV, AIFF, FLAC and OGG; actual decoding depends on the JUCE build.
2. Load tracks into Deck 1 and Deck 2, start playback and adjust the mixer controls.
3. Click an empty hot-cue slot to save the current position; click it again to jump there. Shift-click clears a slot and **CLR** clears the deck's cues.
4. Use **SYNC** to adjust a deck's playback rate using the BPM estimates. This does not guarantee beat-accurate alignment.
5. Press `F11` to toggle fullscreen and Escape to leave fullscreen.

The application stores `library.csv` and `hotcues.csv` inside an `OtoDecks` folder in the operating system's user application-data directory. Library entries contain local file paths; audio files are not copied into the application. No music files or personal library records are included in this repository.

## Code organisation

| Component | Responsibility |
| --- | --- |
| `DJAudioPlayer` | Audio transport, processing, playback rate and BPM estimation |
| `DeckGUI` | Deck controls, cue handling and interaction |
| `WaveformDisplay` | Waveform rendering and position feedback |
| `MixerComponent` | Mixer UI, gains and crossfader |
| `PlaylistComponent` | Track list, search and CSV persistence |
| `MainComponent` | Deck, mixer and library integration |
| `Main.cpp` | Application lifecycle and main window |

## Validation and limitations

Source references and project-file structure were checked during portfolio preparation. The generated-header includes and JUCE paths were made portable, and the CMake MP3 option was aligned with the existing Projucer setting. The application has **not been compiled or run** as part of this preparation; build compatibility and audio behaviour still need validation with a JUCE installation and desktop toolchain.

BPM analysis is a basic energy-based estimate rather than a professional beat grid. Sync changes playback speed, which can also affect pitch. CSV persistence assumes ordinary filenames and is not a general CSV database. This is a learning project, not a production DJ product.

## Acknowledgements

Built with the [JUCE framework](https://juce.com/), which is supplied separately under its own licensing terms. The build instructions follow JUCE's [CMake documentation](https://github.com/juce-framework/JUCE/blob/master/docs/CMake%20API.md). The project was developed as university coursework; this repository does not assign a new license to the framework or any course-provided material.
