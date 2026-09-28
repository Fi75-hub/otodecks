#include "MainComponent.h"

// Constructor
MainComponent::MainComponent()
{
    formatManager.registerBasicFormats(); // Register basic formats
    
    deckGUI1 = std::make_unique<DeckGUI>(&player1, formatManager, thumbCache, 1); // Create deck 1
    deckGUI2 = std::make_unique<DeckGUI>(&player2, formatManager, thumbCache, 2); // Create deck 2
    
    addAndMakeVisible(*deckGUI1); // Add deck 1 to the component
    addAndMakeVisible(*deckGUI2); // Add deck 2 to the component
    
    mixerComponent = std::make_unique<MixerComponent>(&player1, &player2); // Create mixer
    addAndMakeVisible(*mixerComponent); // Add mixer to the component
    
    // Wire up sync buttons
    deckGUI1->onSyncPressed = [this]() {
        deckGUI1->syncToOtherDeck(deckGUI2.get());
    };
    deckGUI2->onSyncPressed = [this]() {
        deckGUI2->syncToOtherDeck(deckGUI1.get());
    };
    
    // Wire up EQ persistence
    deckGUI1->onFileLoaded = [this](String path) {
        mixerComponent->loadEQSettings(1, path);
    };
    deckGUI2->onFileLoaded = [this](String path) {
        mixerComponent->loadEQSettings(2, path);
    };
    
    // Create playlist component with references to both decks
    playlistComponent = std::make_unique<PlaylistComponent>(deckGUI1.get(), deckGUI2.get());
    addAndMakeVisible(*playlistComponent);
    
    // Configure title label
    titleLabel.setText("OtoDecks", dontSendNotification);
    titleLabel.setFont(Font(28.0f, Font::bold));
    titleLabel.setColour(Label::textColourId, Colours::white);
    titleLabel.setJustificationType(Justification::centred);
    addAndMakeVisible(titleLabel);
    
    setSize(1200, 800);

    mixerSource.addInputSource(&player1, false);
    mixerSource.addInputSource(&player2, false);

    // Request audio permissions
    if (RuntimePermissions::isRequired(RuntimePermissions::recordAudio)
        && !RuntimePermissions::isGranted(RuntimePermissions::recordAudio))
    {
        RuntimePermissions::request(RuntimePermissions::recordAudio,
                                   [&](bool granted) { if (granted) setAudioChannels(2, 2); });
    }  
    else
    {
        setAudioChannels(0, 2);
    }  
}

MainComponent::~MainComponent()
{
    shutdownAudio();
}

// Prepare to play
void MainComponent::prepareToPlay(int samplesPerBlockExpected, double sampleRate)
{
    mixerSource.prepareToPlay(samplesPerBlockExpected, sampleRate);
}

// Get next audio block
void MainComponent::getNextAudioBlock(const AudioSourceChannelInfo& bufferToFill)
{
    mixerSource.getNextAudioBlock(bufferToFill);
}

// Release resources
void MainComponent::releaseResources()
{
    mixerSource.releaseResources();
}

// Paint
void MainComponent::paint(Graphics& g)
{
    // Dark gradient background
    ColourGradient bgGradient(Colour(30, 30, 35), 0, 0,
                              Colour(15, 15, 20), 0, (float)getHeight(), false);
    g.setGradientFill(bgGradient);
    g.fillAll();
    
    // Top accent line
    ColourGradient topLine(Colour(0, 200, 255), 0, 0,
                           Colour(255, 100, 150), (float)getWidth(), 0, false);
    g.setGradientFill(topLine);
    g.fillRect(0, 50, getWidth(), 2);
    
    // Section labels
    g.setColour(Colour(80, 80, 85));
    g.setFont(Font(10.0f, Font::bold));
    g.drawText("MUSIC LIBRARY", 0, (int)(getHeight() * 0.58f + 52), getWidth(), 20, Justification::centred);
    
}

// Resized
void MainComponent::resized()
{
    auto area = getLocalBounds();
    
    // Title bar at top
    auto titleBar = area.removeFromTop(50);
    titleLabel.setBounds(titleBar);

    int deckHeight = (int)(area.getHeight() * 0.58);
    auto decksArea = area.removeFromTop(deckHeight);
    
    int totalWidth = decksArea.getWidth();
    int mixerWidth = 140; 
    int deckWidth = (totalWidth - mixerWidth) / 2;
    
    if (deckGUI1)
    {
        deckGUI1->setBounds(decksArea.removeFromLeft(deckWidth).reduced(4));
    }
    
    if (mixerComponent)
    {
        mixerComponent->setBounds(decksArea.removeFromLeft(mixerWidth));
    }
    
    if (deckGUI2)
    {
        deckGUI2->setBounds(decksArea.reduced(4));
    }
    
    // Playlist takes the bottom portion, full width
    auto bottomArea = area.reduced(5, 5);
    
    if (playlistComponent)
    {
        playlistComponent->setBounds(bottomArea);
    }
}