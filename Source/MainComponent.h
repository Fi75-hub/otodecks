#pragma once
#include <JuceHeader.h>
#include "DJAudioPlayer.h"
#include "DeckGUI.h"
#include "PlaylistComponent.h"
#include "MixerComponent.h"

// Main component class
class MainComponent : public AudioAppComponent
{
public:
    MainComponent(); // Constructor
    ~MainComponent(); // Destructor

    void prepareToPlay(int samplesPerBlockExpected, double sampleRate) override;
    void getNextAudioBlock(const AudioSourceChannelInfo& bufferToFill) override;
    void releaseResources() override;

    void paint(Graphics& g) override;
    void resized() override;

private:
    AudioFormatManager formatManager;
    AudioThumbnailCache thumbCache{100}; 

    DJAudioPlayer player1{formatManager};
    DJAudioPlayer player2{formatManager};
    
    std::unique_ptr<DeckGUI> deckGUI1;
    std::unique_ptr<DeckGUI> deckGUI2;
    std::unique_ptr<MixerComponent> mixerComponent;

    MixerAudioSource mixerSource; 

    std::unique_ptr<PlaylistComponent> playlistComponent;
    
    // Crossfader
    Slider crossfaderSlider;
    Label crossfaderLabel;
    
    // Application title label
    Label titleLabel;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MainComponent)
};