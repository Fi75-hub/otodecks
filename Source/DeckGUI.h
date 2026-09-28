#pragma once

#include <JuceHeader.h>
#include "DJAudioPlayer.h"
#include "WaveformDisplay.h"

// Jog wheel component
class JogWheel : public Component,
                 public Timer
{
public:
    JogWheel(); // Constructor
    ~JogWheel() override; // Destructor
    
    void paint(Graphics&) override;
    void resized() override;
    
    // Mouse listeners for jog wheel interaction
    void mouseDown(const MouseEvent& event) override;
    void mouseDrag(const MouseEvent& event) override;
    void mouseUp(const MouseEvent& event) override;
    
    void timerCallback() override;
    
    // Set the rotation based on playback position (0.0 to 1.0)
    void setPositionRelative(double pos);
    
    // Set whether the track is playing
    void setPlaying(bool isPlaying);
    
    // Callback for when user scratches the jog wheel
    std::function<void(double)> onScratch;
    
    // Set the deck color accent
    void setAccentColor(Colour color);
    
    // Update audio level for visual pulsing (0.0 to 1.0)
    void setAudioLevel(float level);


private:
    // Private members
    double rotation = 0.0;
    double lastMouseAngle = 0.0;
    double dragStartRotation = 0.0;
    bool isDragging = false;
    bool playing = false;
    float audioLevel = 0.0f;
    Colour accentColor = Colours::cyan;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(JogWheel)
};


// Custom styled button
class DJButton : public TextButton
{
public:
    DJButton(const String& buttonName);
    void paintButton(Graphics&, bool shouldDrawButtonAsHighlighted, bool shouldDrawButtonAsDown) override;
    void setAccentColor(Colour color);
    
private:
    Colour accentColor = Colours::cyan;
};

// Custom rotary knob
class DJKnob : public Component
{
public:
    DJKnob();
    
    void paint(Graphics&) override;
    void mouseDown(const MouseEvent& event) override;
    void mouseDrag(const MouseEvent& event) override;
    void mouseUp(const MouseEvent& event) override;
    
    void setAccentColor(Colour color);
    void setRange(double minVal, double maxVal);
    void setValue(double newValue, NotificationType notification = sendNotificationAsync);
    double getValue() const;
    void setBipolar(bool bipolar);
    
    // Listener support
    std::function<void(double)> onValueChange;
    
private:
    double value = 0.5;
    double minValue = 0.0;
    double maxValue = 1.0;
    double lastMouseAngle = 0.0;
    bool isDragging = false;
    bool isBipolar = false;
    Colour accentColor = Colours::cyan;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(DJKnob)
};

// Deck GUI component
class DeckGUI : public Component,
                public Button::Listener, 
                public FileDragAndDropTarget,
                public DragAndDropTarget,
                public Timer
{
public:
    DeckGUI(DJAudioPlayer* player, 
           AudioFormatManager& formatManagerToUse,
           AudioThumbnailCache& cacheToUse,
           int deckNumber);
    ~DeckGUI();

    void paint(Graphics&) override;
    void resized() override;

    // Button::Listener
    void buttonClicked(Button*) override;

    // FileDragAndDropTarget
    bool isInterestedInFileDrag(const StringArray& files) override;
    void filesDropped(const StringArray& files, int x, int y) override; 
    
    // DragAndDropTarget
    bool isInterestedInDragSource(const SourceDetails& dragSourceDetails) override;
    void itemDropped(const SourceDetails& dragSourceDetails) override; 

    // Timer
    void timerCallback() override; 
    
    // Load a file into this deck and update waveform display
    void loadFile(URL audioURL);
    
    // Get the deck name for display
    String getDeckName() const;

    // Sync functionality
    void syncToOtherDeck(DeckGUI* otherDeck);
    double getBPM() const;
    double getSpeed() const;
    void setSpeed(double speed);
    
    // Callback for sync button
    std::function<void()> onSyncPressed;

    // Callback for when a file is loaded
    std::function<void(String)> onFileLoaded;

    // Hot Cue methods
    void updateHotCueVisuals();
    void saveHotCues();
    void loadHotCues();
    File getHotCuesFile();

private:
    DJAudioPlayer* player;
    int deckNumber;
    Colour accentColor;
    
    // File chooser
    juce::FileChooser fChooser{"Select a file..."};
    
    // Waveform display
    WaveformDisplay waveformDisplay;
    
    // Custom components
    JogWheel jogWheel;
    
    // Styled buttons
    DJButton playStopButton{"PLAY"};
    DJButton loopButton{"LOOP"};
    DJButton loadButton{"LOAD"};
    DJButton syncButton{"SYNC"};  
    
    // Hot Cue buttons
    std::vector<std::unique_ptr<DJButton>> hotCueButtons;
    DJButton clearCuesButton{"CLEAR CUES"};
    
    // Labels
    Label bpmLabel;
    
    double detectedBPM = 0.0;
    
    // Styled knobs
    DJKnob speedKnob;
    DJKnob reverbKnob;
    DJKnob echoKnob;
    DJKnob filterKnob;
    DJKnob feedbackKnob;
    
    // Labels
    Label deckLabel;
    Label speedLabel{"", "SPEED"};
    Label reverbLabel{"", "REVERB"};
    Label echoLabel{"", "ECHO"};
    Label filterLabel{"", "FILTER"};
    Label feedbackLabel{"", "FDBK"};
    Label trackNameLabel;
    String currentFilePath;
    
    // Hot Cue data
    std::vector<double> hotCues; // Stores relative positions (-1.0 if unset)
    
    // Cue point position
    double cuePoint = 0.0;
    bool isLooping = false;  // When true, song restarts from beginning when it ends
    bool wasPlayingBeforeCue = false;  // Track if we were playing before CUE was pressed
    bool isSyncing = false; // Flag for sync lock status

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(DeckGUI)
};