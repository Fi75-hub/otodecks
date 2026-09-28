#include "MixerComponent.h"

MixerComponent::MixerComponent(DJAudioPlayer* _player1, DJAudioPlayer* _player2)
    : player1(_player1), player2(_player2)
{
    // Apply custom LookAndFeel
    volSlider1.setLookAndFeel(&faderLnF);
    volSlider2.setLookAndFeel(&faderLnF);
    crossfader.setLookAndFeel(&faderLnF);

    // Setup Deck 1
    Colour deck1Color(0, 200, 255);
    
    highEQ1.setAccentColor(deck1Color);
    highEQ1.setRange(0.0, 1.0);
    highEQ1.setValue(0.5);
    highEQ1.setBipolar(true);
    highEQ1.onValueChange = [this](double val) { 
        if (player1) player1->setHighEQ(val); 
        saveEQSettings(1);
    };

    addAndMakeVisible(highEQ1);
    
    midEQ1.setAccentColor(deck1Color);
    midEQ1.setRange(0.0, 1.0);
    midEQ1.setValue(0.5);
    midEQ1.setBipolar(true);
    
    midEQ1.onValueChange = [this](double val) { 
        if (player1) player1->setMidEQ(val); 
        saveEQSettings(1);
    };

    addAndMakeVisible(midEQ1);
    
    lowEQ1.setAccentColor(deck1Color);
    lowEQ1.setRange(0.0, 1.0);
    lowEQ1.setValue(0.5);
    lowEQ1.setBipolar(true);
    
    lowEQ1.onValueChange = [this](double val) { 
        if (player1) player1->setLowEQ(val); 
        saveEQSettings(1);
    };

    addAndMakeVisible(lowEQ1);
    
    volSlider1.setSliderStyle(Slider::LinearVertical);
    volSlider1.setRange(0.0, 1.0);
    volSlider1.setValue(0.75);
    volSlider1.setTextBoxStyle(Slider::NoTextBox, false, 0, 0);
    volSlider1.setColour(Slider::thumbColourId, deck1Color);
    volSlider1.onValueChange = [this] { if (player1) player1->setGain(volSlider1.getValue()); };
    addAndMakeVisible(volSlider1);
    
    
    // Setup Deck 2
    Colour deck2Color(255, 100, 150);
    
    highEQ2.setAccentColor(deck2Color);
    highEQ2.setRange(0.0, 1.0);
    highEQ2.setValue(0.5);
    highEQ2.setBipolar(true);
    highEQ2.onValueChange = [this](double val) { 
        if (player2) player2->setHighEQ(val); 
        saveEQSettings(2);
    };

    addAndMakeVisible(highEQ2);
    
    midEQ2.setAccentColor(deck2Color);
    midEQ2.setRange(0.0, 1.0);
    midEQ2.setValue(0.5);
    midEQ2.setBipolar(true);
    midEQ2.onValueChange = [this](double val) { 
        if (player2) player2->setMidEQ(val); 
        saveEQSettings(2);
    };

    addAndMakeVisible(midEQ2);
    
    lowEQ2.setAccentColor(deck2Color);
    lowEQ2.setRange(0.0, 1.0);
    lowEQ2.setValue(0.5);
    lowEQ2.setBipolar(true);
    lowEQ2.onValueChange = [this](double val) { 
        if (player2) player2->setLowEQ(val); 
        saveEQSettings(2);
    };

    addAndMakeVisible(lowEQ2);
    
    volSlider2.setSliderStyle(Slider::LinearVertical);
    volSlider2.setRange(0.0, 1.0);
    volSlider2.setValue(0.75);
    volSlider2.setTextBoxStyle(Slider::NoTextBox, false, 0, 0);
    volSlider2.setColour(Slider::thumbColourId, deck2Color);
    volSlider2.onValueChange = [this] { if (player2) player2->setGain(volSlider2.getValue()); };
    addAndMakeVisible(volSlider2);
    
    
    // Setup Crossfader
    crossfader.setSliderStyle(Slider::LinearHorizontal);
    crossfader.setRange(0.0, 1.0);
    crossfader.setValue(0.5);
    crossfader.setTextBoxStyle(Slider::NoTextBox, false, 0, 0);
    crossfader.setColour(Slider::thumbColourId, Colours::lightgrey);
    crossfader.onValueChange = [this] {
        double val = crossfader.getValue();
        if (player1) player1->setCrossfaderGain(1.0 - val);
        if (player2) player2->setCrossfaderGain(val);
    };
    addAndMakeVisible(crossfader);
    
    // Start timer for meters
    
    // Setup Labels
    auto setupEQLabel = [this](Label& label, const String& text) {
        label.setText(text, dontSendNotification);
        label.setFont(Font(10.0f, Font::bold));
        label.setColour(Label::textColourId, Colours::grey);
        label.setJustificationType(Justification::centred);
        addAndMakeVisible(label);
    };

    setupEQLabel(highLabel1, "HIGH");
    setupEQLabel(midLabel1, "MID");
    setupEQLabel(lowLabel1, "LOW");
    setupEQLabel(highLabel2, "HIGH");
    setupEQLabel(midLabel2, "MID");
    setupEQLabel(lowLabel2, "LOW");
}

// Destructor
MixerComponent::~MixerComponent()
{
    volSlider1.setLookAndFeel(nullptr);
    volSlider2.setLookAndFeel(nullptr);
    crossfader.setLookAndFeel(nullptr);
}

// Paint
void MixerComponent::paint(Graphics& g)
{
    // Background
    g.fillAll(Colour(20, 20, 25));
    
    // Vertical line separation
    g.setColour(Colour(40, 40, 45));
    g.drawVerticalLine(getWidth() / 2, 0, getHeight() - 50);
    
    // Container borders
    g.drawRect(getLocalBounds(), 1);
}

// Resized
void MixerComponent::resized()
{
    auto area = getLocalBounds().reduced(2);
    
    // Crossfader at bottom
    auto crossfaderArea = area.removeFromBottom(50);
    crossfader.setBounds(crossfaderArea.reduced(5, 10));
    
    // Vertical split
    int channelWidth = area.getWidth() / 2;
    auto leftChannel = area.removeFromLeft(channelWidth).reduced(2, 0);
    auto rightChannel = area.reduced(2, 0);
    
    int knobSize = 50;
    int spacing = 6;
    
    // Position helper
    auto layoutChannel = [&](Rectangle<int> bounds, DJKnob& high, DJKnob& mid, DJKnob& low, Slider& vol,
                             Label& hLab, Label& mLab, Label& lLab)
    {
        // Knobs at top (High, Mid, Low stacked)
        int knobY = bounds.getY() + 5;
        int knobX = bounds.getCentreX() - knobSize/2;
        int labelH = 12;
        
        hLab.setBounds(knobX, knobY, knobSize, labelH);
        high.setBounds(knobX, knobY + labelH, knobSize, knobSize);
        knobY += knobSize + labelH + spacing;
        
        mLab.setBounds(knobX, knobY, knobSize, labelH);
        mid.setBounds(knobX, knobY + labelH, knobSize, knobSize);
        knobY += knobSize + labelH + spacing;
        
        lLab.setBounds(knobX, knobY, knobSize, labelH);
        low.setBounds(knobX, knobY + labelH, knobSize, knobSize);
        knobY += knobSize + labelH + spacing + 5;
        
        // Fader section
        auto faderArea = bounds.withTop(knobY).withBottom(bounds.getBottom());
        
        int faderWidth = 40;
        
        // Center fader in remaining space
        vol.setBounds(faderArea.getCentreX() - faderWidth/2, faderArea.getY(), faderWidth, faderArea.getHeight());
    };
    
    layoutChannel(leftChannel, highEQ1, midEQ1, lowEQ1, volSlider1, highLabel1, midLabel1, lowLabel1);
    layoutChannel(rightChannel, highEQ2, midEQ2, lowEQ2, volSlider2, highLabel2, midLabel2, lowLabel2);
}

void MixerComponent::timerCallback()
{
    
}

// Get EQ Settings File
File MixerComponent::getEQSettingsFile()
{
    File appDataDir = File::getSpecialLocation(File::userApplicationDataDirectory);
    File otoDecksDir = appDataDir.getChildFile("OtoDecks");
    if (!otoDecksDir.exists()) otoDecksDir.createDirectory();
    return otoDecksDir.getChildFile("equalization.csv");
}

// Save EQ Settings
void MixerComponent::saveEQSettings(int deck)
{
    String path = (deck == 1) ? currentFilePath1 : currentFilePath2;
    if (path.isEmpty()) return;
    
    File file = getEQSettingsFile();
    StringArray lines;
    if (file.existsAsFile())
        file.readLines(lines);
        
    double high = (deck == 1) ? highEQ1.getValue() : highEQ2.getValue();
    double mid = (deck == 1) ? midEQ1.getValue() : midEQ2.getValue();
    double low = (deck == 1) ? lowEQ1.getValue() : lowEQ2.getValue();
    
    bool found = false;
    for (int i = 0; i < lines.size(); ++i)
    {
        if (lines[i].startsWith("\"" + path + "\""))
        {
            StringArray tokens;
            tokens.addTokens(lines[i], ",", "\"");
            
            // Format: "path", origH, origM, origL, lastH, lastM, lastL
            String origH = (tokens.size() >= 4) ? tokens[1] : "0.500";
            String origM = (tokens.size() >= 4) ? tokens[2] : "0.500";
            String origL = (tokens.size() >= 4) ? tokens[3] : "0.500";
            
            String updatedLine = "\"" + path + "\"," + origH + "," + origM + "," + origL + "," + 
                                 String(high, 3) + "," + String(mid, 3) + "," + String(low, 3);
            lines.set(i, updatedLine);
            found = true;
            break;
        }
    }
    
    if (!found)
    {
        // First time saving (Original is neutral (0.5), Last is also current)
        String myLine = "\"" + path + "\",0.500,0.500,0.500," + 
                         String(high, 3) + "," + String(mid, 3) + "," + String(low, 3);
        lines.add(myLine);
    }
        
    file.replaceWithText(lines.joinIntoString("\n"));
}

void MixerComponent::loadEQSettings(int deck, const String& path)
{
    if (deck == 1) currentFilePath1 = path;
    else currentFilePath2 = path;
    
    // Default to neutral (0.5)
    double high = 0.5, mid = 0.5, low = 0.5;
    
    File file = getEQSettingsFile();
    if (file.existsAsFile() && !path.isEmpty())
    {
        StringArray lines;
        file.readLines(lines);
        
        for (const auto& line : lines)
        {
            if (line.startsWith("\"" + path + "\""))
            {
                StringArray tokens;
                tokens.addTokens(line, ",", "\"");
                
                // Last adjusted values
                if (tokens.size() >= 7)
                {
                    high = tokens[4].getDoubleValue();
                    mid = tokens[5].getDoubleValue();
                    low = tokens[6].getDoubleValue();
                }
                else if (tokens.size() >= 4) // Legacy format
                {
                    high = tokens[1].getDoubleValue();
                    mid = tokens[2].getDoubleValue();
                    low = tokens[3].getDoubleValue();
                }
                break;
            }
        }
    }
    
    // Apply to knobs and players
    if (deck == 1)
    {
        highEQ1.setValue(high, dontSendNotification);
        midEQ1.setValue(mid, dontSendNotification);
        lowEQ1.setValue(low, dontSendNotification);
        if (player1)
        {
            player1->setHighEQ(high);
            player1->setMidEQ(mid);
            player1->setLowEQ(low);
        }
    }
    else
    {
        highEQ2.setValue(high, dontSendNotification);
        midEQ2.setValue(mid, dontSendNotification);
        lowEQ2.setValue(low, dontSendNotification);
        if (player2)
        {
            player2->setHighEQ(high);
            player2->setMidEQ(mid);
            player2->setLowEQ(low);
        }
    }
}