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

- Visual Studio 2022 with the **Desktop development with C++** workload on Windows. Other platforms need a JUCE-supported compiler.
- [CMake](https://cmake.org/download/) 3.22 or later.
- The [JUCE 8.0.10 source tree](https://github.com/juce-framework/JUCE/tree/8.0.10), including its modules and build scripts.

JUCE is supplied separately. The Windows build workflow uses JUCE 8.0.10 and MSVC. MinGW is not supported by JUCE 8.

## Build with CMake

Download JUCE 8.0.10 or clone it next to this repository:

```sh
git clone --depth 1 --branch 8.0.10 https://github.com/juce-framework/JUCE.git ../JUCE
```

The folders should look like this:

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

### Projucer alternative

`OtoDecks.jucer` is retained from the original project. Open it in a compatible Projucer version, confirm that the JUCE module paths point to your installation, select an exporter suitable for your system and save the project before opening the generated IDE project. The stored paths use `../JUCE/modules`. Generated `Builds/` and `JuceLibraryCode/` directories are intentionally excluded from version control.

## Use the application

1. Add your own audio files with **+ Add Files**, or use a deck's **LOAD** control. The library chooser lists MP3, WAV, AIFF, FLAC and OGG; actual decoding depends on the JUCE build.
2. Load tracks into Deck 1 and Deck 2, start playback and adjust the mixer controls.
3. Click an empty hot-cue slot to save the current position; click it again to jump there. Shift-click clears a slot and **CLR** clears the deck's cues.
4. Use **SYNC** to adjust a deck's playback rate using the BPM estimates. This does not guarantee beat-accurate alignment.
5. Press `F11` to toggle fullscreen and Escape to leave fullscreen.

The application stores `library.csv`, `hotcues.csv` and `equalization.csv` inside an `OtoDecks` folder in the operating system's user application-data directory. Library entries contain local file paths; audio files are not copied into the application. No music files or personal library records are included in this repository.

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

## Tests

The [Windows build workflow](https://github.com/Fi75-hub/otodecks/actions/workflows/build.yml) compiles the application and runs offline audio checks. To run the same checks locally:

```sh
cmake -S . -B build -DOTODECKS_BUILD_TESTS=ON
cmake --build build --config Release
ctest --test-dir build -C Release --output-on-failure
```

The tests generate a short WAV file and check playback, seeking, input validation, track replacement, echo reset and crossfader muting. They do not require an audio device. Listening quality, device selection and live interaction still need a manual desktop check.

## Limitations

BPM analysis uses a basic energy estimate and falls back to 120 BPM when it cannot identify enough peaks. Sync changes playback speed, which can also affect pitch. Library and cue persistence use simple CSV files; filenames containing quotation marks are not supported reliably.

## Acknowledgements

Built with the [JUCE framework](https://juce.com/), which is supplied separately under its own licensing terms. The build instructions follow JUCE's [CMake documentation](https://github.com/juce-framework/JUCE/blob/master/docs/CMake%20API.md). Developed as university coursework; any course-provided material retains its original terms.
