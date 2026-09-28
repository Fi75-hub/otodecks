#include "PlaylistComponent.h"
#include <fstream>
#include <sstream>

// Styled button for deck loading
StyledPlaylistButton::StyledPlaylistButton(const String& name, Colour accent)
    : TextButton(name), accentColor(accent)
{
}

void StyledPlaylistButton::paintButton(Graphics& g, bool highlighted, bool down)
{
    auto bounds = getLocalBounds().toFloat().reduced(2);
    
    // Background
    Colour bgColor = down ? accentColor : (highlighted ? accentColor.darker(0.3f) : Colour(50, 50, 55));
    g.setColour(bgColor);
    g.fillRoundedRectangle(bounds, 4);
    
    // Border
    g.setColour(accentColor);
    g.drawRoundedRectangle(bounds, 4, 1);
    
    // Text
    g.setColour(down ? Colours::white : (highlighted ? Colours::white : accentColor));
    g.setFont(Font(11.0f, Font::bold));
    g.drawText(getButtonText(), bounds, Justification::centred);
}

PlaylistComponent::PlaylistComponent(DeckGUI* _deckGUI1, DeckGUI* _deckGUI2)
    : deckGUI1(_deckGUI1), deckGUI2(_deckGUI2)
{
    // Register audio formats for reading file metadata
    formatManager.registerBasicFormats();
    
    // Load saved library from file
    loadLibraryFromFile();
    
    // Initialise filtered indices to show all tracks
    updateFilteredTracks();

    // Add the table component
    addAndMakeVisible(tableComponent);
    
    // Style the table
    tableComponent.setColour(ListBox::backgroundColourId, Colour(25, 25, 30));
    tableComponent.setColour(ListBox::outlineColourId, Colour(50, 50, 55));
    tableComponent.setRowHeight(32);
    
    // Add control buttons
    addAndMakeVisible(addFilesButton);
    addAndMakeVisible(clearButton);
    addAndMakeVisible(searchBox);
    
    // Style buttons
    auto styleButton = [](TextButton& btn, Colour color) {
        btn.setColour(TextButton::buttonColourId, Colour(45, 45, 50));
        btn.setColour(TextButton::buttonOnColourId, color);
        btn.setColour(TextButton::textColourOffId, color);
        btn.setColour(TextButton::textColourOnId, Colours::white);
    };
    
    styleButton(addFilesButton, Colours::limegreen);

    styleButton(clearButton, Colours::red);
    
    // Configure search box with styled design
    searchBox.setTextToShowWhenEmpty("Search Tracks...", Colours::grey);
    searchBox.setColour(TextEditor::backgroundColourId, Colour(45, 45, 50));
    searchBox.setColour(TextEditor::outlineColourId, Colour(205, 133, 63));
    searchBox.setColour(TextEditor::focusedOutlineColourId, Colour(255, 165, 80));
    searchBox.setColour(TextEditor::textColourId, Colours::white);
    searchBox.setFont(Font(13.0f));
    searchBox.setJustification(Justification::centredLeft);
    searchBox.setIndents(8, -2);
    searchBox.addListener(this);
    
    // Configure button listeners
    addFilesButton.addListener(this);
    clearButton.addListener(this);

    // Add columns to the table
    auto& header = tableComponent.getHeader();
    header.setColour(TableHeaderComponent::backgroundColourId, Colour(35, 35, 40));
    header.setColour(TableHeaderComponent::textColourId, Colours::lightgrey);
    header.setColour(TableHeaderComponent::outlineColourId, Colour(50, 50, 55));
    
    // Apply custom look and feel for centered header text
    header.setLookAndFeel(&headerLookAndFeel);
    
    // Column 1: Track title
    header.addColumn("TRACK TITLE", 1, 280, 100, 1000, 
                     TableHeaderComponent::notSortable | TableHeaderComponent::visible);
    header.setStretchToFitActive(true);

    // Column 2: Duration
    header.addColumn("TIME", 2, 70, 50, 100, 
                     TableHeaderComponent::notSortable | TableHeaderComponent::visible);
    // Column 3: Load to Deck 1 button
    header.addColumn("", 3, 75, 75, 75, 
                     TableHeaderComponent::notSortable | TableHeaderComponent::notResizable);
    // Column 4: Load to Deck 2 button
    header.addColumn("", 4, 75, 75, 75, 
                     TableHeaderComponent::notSortable | TableHeaderComponent::notResizable);
    // Column 5: Delete button
    header.addColumn("", 5, 50, 50, 50, 
                     TableHeaderComponent::notSortable | TableHeaderComponent::notResizable);

    // Register this component as the model for the table
    tableComponent.setModel(this);
}

PlaylistComponent::~PlaylistComponent()
{
    // Clear custom look and feel before destruction
    tableComponent.getHeader().setLookAndFeel(nullptr);
    
    // Save library state when component is destroyed
    saveLibraryToFile();
}

// Get the library file path
File PlaylistComponent::getLibraryFile()
{
    // Store in user's application data directory for persistence
    File appDataDir = File::getSpecialLocation(File::userApplicationDataDirectory);
    File otoDecksDir = appDataDir.getChildFile("OtoDecks");
    
    // Create directory if it doesn't exist
    if (!otoDecksDir.exists())
    {
        otoDecksDir.createDirectory();
    }
    
    return otoDecksDir.getChildFile("library.csv");
}

// Save library to CSV file
void PlaylistComponent::saveLibraryToFile()
{
    File libraryFile = getLibraryFile();
    
    std::ofstream outFile(libraryFile.getFullPathName().toStdString());
    
    if (outFile.is_open())
    {
        outFile << "title,filepath,duration\n";
        
        for (const auto& track : trackLibrary)
        {
            outFile << "\"" << track.title.toStdString() << "\","
                    << "\"" << track.filePath.toStdString() << "\","
                    << track.duration << "\n";
        }
        
        outFile.close();
        DBG("Library saved: " << trackLibrary.size() << " tracks");
    }
}

// Load library from CSV file
void PlaylistComponent::loadLibraryFromFile()
{
    File libraryFile = getLibraryFile();
    
    if (!libraryFile.existsAsFile())
    {
        DBG("No library file found");
        return;
    }
    
    std::ifstream inFile(libraryFile.getFullPathName().toStdString());
    
    if (inFile.is_open())
    {
        std::string line;
        bool isHeader = true;
        
        while (std::getline(inFile, line))
        {
            if (isHeader)
            {
                isHeader = false;
                continue;
            }
            
            std::string title, filepath, durationStr;
            
            if (line[0] == '"')
            {
                size_t endQuote = line.find("\",", 1);
                if (endQuote != std::string::npos)
                {
                    title = line.substr(1, endQuote - 1);
                    
                    size_t pathStart = endQuote + 3;
                    size_t pathEnd = line.find("\",", pathStart);
                    if (pathEnd != std::string::npos)
                    {
                        filepath = line.substr(pathStart, pathEnd - pathStart);
                        durationStr = line.substr(pathEnd + 2);
                    }
                }
            }
            
            File audioFile(filepath);
            if (audioFile.existsAsFile())
            {
                TrackInfo track;
                track.title = String(title);
                track.filePath = String(filepath);
                track.duration = std::stod(durationStr);
                trackLibrary.push_back(track);
            }
        }
        
        inFile.close();
        DBG("Library loaded: " << trackLibrary.size() << " tracks");
    }
}

// Add files to library
void PlaylistComponent::addFilesToLibrary()
{
    fileChooser = std::make_unique<FileChooser>(
        "Select audio files to add...",
        File::getSpecialLocation(File::userMusicDirectory),
        "*.mp3;*.wav;*.aiff;*.flac;*.ogg",
        true
    );
    
    auto chooserFlags = FileBrowserComponent::openMode | 
                        FileBrowserComponent::canSelectFiles |
                        FileBrowserComponent::canSelectMultipleItems;
    
    fileChooser->launchAsync(chooserFlags, [this](const FileChooser& fc)
    {
        auto results = fc.getResults();
        
        for (const auto& file : results)
        {
            if (file.existsAsFile())
            {
                bool alreadyExists = false;
                for (const auto& track : trackLibrary)
                {
                    if (track.filePath == file.getFullPathName())
                    {
                        alreadyExists = true;
                        break;
                    }
                }
                
                if (!alreadyExists)
                {
                    TrackInfo track;
                    track.title = file.getFileNameWithoutExtension();
                    track.filePath = file.getFullPathName();
                    track.duration = getAudioFileDuration(file);
                    trackLibrary.push_back(track);
                    DBG("Added: " << track.title);
                }
            }
        }
        
        updateFilteredTracks();
        tableComponent.updateContent();
        saveLibraryToFile();
    });
}

// Get audio file duration
double PlaylistComponent::getAudioFileDuration(const File& file)
{
    std::unique_ptr<AudioFormatReader> reader(formatManager.createReaderFor(file));
    
    if (reader != nullptr)
    {
        return reader->lengthInSamples / reader->sampleRate;
    }
    
    return 0.0;
}

// Format duration
String PlaylistComponent::formatDuration(double seconds)
{
    int mins = static_cast<int>(seconds) / 60;
    int secs = static_cast<int>(seconds) % 60;
    return String::formatted("%d:%02d", mins, secs);
}

// Update filtered tracks
void PlaylistComponent::updateFilteredTracks()
{
    filteredIndices.clear();
    
    String searchText = searchBox.getText().toLowerCase();
    
    for (int i = 0; i < static_cast<int>(trackLibrary.size()); ++i)
    {
        if (searchText.isEmpty() || 
            trackLibrary[i].title.toLowerCase().contains(searchText))
        {
            filteredIndices.push_back(i);
        }
    }
}

void PlaylistComponent::textEditorTextChanged(TextEditor&)
{
    updateFilteredTracks();
    tableComponent.updateContent();
}

void PlaylistComponent::paint(Graphics& g)
{
    ColourGradient bgGradient(Colour(30, 30, 35), 0, 0,
                              Colour(20, 20, 25), 0, (float)getHeight(), false);
    g.setGradientFill(bgGradient);
    g.fillRoundedRectangle(getLocalBounds().toFloat(), 6);

    // Border
    g.setColour(Colour(50, 50, 55));
    g.drawRoundedRectangle(getLocalBounds().toFloat().reduced(1), 6, 1);

    // Empty state message
    if (trackLibrary.empty())
    {
        g.setColour(Colours::grey);
        g.setFont(Font(14.0f, Font::italic));
        g.drawText("Click '+ Add Files' to build your music library", 
                   getLocalBounds().reduced(0, 60),
                   Justification::centred, true);
    }
}

void PlaylistComponent::resized()
{
    auto area = getLocalBounds().reduced(8);
    
    // Control bar at top
    auto controlBar = area.removeFromTop(45);
    
    // Layout control bar components
    int buttonWidth = 100;
    int buttonHeight = 32;
    int spacing = 8;
    
    addFilesButton.setBounds(controlBar.removeFromLeft(buttonWidth).withHeight(buttonHeight).withY(controlBar.getY() + 6));
    controlBar.removeFromLeft(spacing);
    clearButton.setBounds(controlBar.removeFromLeft(buttonWidth - 20).withHeight(buttonHeight).withY(controlBar.getY() + 6));
    controlBar.removeFromLeft(spacing);
    
    // Search box takes remaining width
    searchBox.setBounds(controlBar.withHeight(buttonHeight).withY(controlBar.getY() + 6));
    
    // Table takes remaining space
    area.removeFromTop(4);
    tableComponent.setBounds(area);
}

int PlaylistComponent::getNumRows()
{
    return static_cast<int>(filteredIndices.size()); // Return number of rows
}

void PlaylistComponent::paintRowBackground(Graphics& g,
                                           int rowNumber,
                                           int width,
                                           int height,
                                           bool rowIsSelected)
{
    if (rowIsSelected)
    {
        // Selected row with gradient
        ColourGradient gradient(Colour(60, 60, 70), 0, 0,
                               Colour(50, 50, 60), 0, (float)height, false);
        g.setGradientFill(gradient);
        g.fillRect(0, 0, width, height);
        
        // Accent border
        g.setColour(deck1Color.withAlpha(0.5f));
        g.drawRect(0, 0, width, height, 1);
    }
    else
    {
        // Alternating row colors
        Colour rowColor = (rowNumber % 2 == 0) ? Colour(28, 28, 33) : Colour(32, 32, 37);
        g.setColour(rowColor);
        g.fillRect(0, 0, width, height);
    }
    
    // Bottom border
    g.setColour(Colour(40, 40, 45));
    g.drawLine(0, (float)height - 1, (float)width, (float)height - 1, 1.0f);
}

// Paint cell
void PlaylistComponent::paintCell(Graphics& g,
                                  int rowNumber,
                                  int columnId,
                                  int width,
                                  int height,
                                  bool rowIsSelected)
{
    if (rowNumber >= static_cast<int>(filteredIndices.size()))
        return;
        
    int libraryIndex = filteredIndices[rowNumber];
    const auto& track = trackLibrary[libraryIndex];
    
    g.setColour(rowIsSelected ? Colours::white : Colours::lightgrey);
    
    if (columnId == 1)
    {
        // Track title with icon
        g.setFont(Font(12.0f));
        g.drawText(String(CharPointer_UTF8("\xe2\x99\xab")) + " " + track.title,
                   8, 0,
                   width - 16, height,
                   Justification::centredLeft,
                   true);
    }
    else if (columnId == 2)
    {
        // Duration in lighter color
        g.setColour(Colours::grey);
        g.setFont(Font(11.0f));
        g.drawText(formatDuration(track.duration),
                   4, 0,
                   width - 8, height,
                   Justification::centred,
                   true);
    }
}

Component* PlaylistComponent::refreshComponentForCell(int rowNumber,
                                                      int columnId,
                                                      bool isRowSelected,
                                                      Component* existingComponentToUpdate)
{
    // Create styled buttons for deck loading
    if (columnId == 3)
    {
        auto* btn = dynamic_cast<StyledPlaylistButton*>(existingComponentToUpdate);
        if (btn == nullptr)
        {
            btn = new StyledPlaylistButton("DECK 1", deck1Color);
            btn->addListener(this);
        }
        btn->setComponentID(String(rowNumber) + "_1");
        return btn;
    }
    else if (columnId == 4)
    {
        auto* btn = dynamic_cast<StyledPlaylistButton*>(existingComponentToUpdate);
        if (btn == nullptr)
        {
            btn = new StyledPlaylistButton("DECK 2", deck2Color);
            btn->addListener(this);
        }
        btn->setComponentID(String(rowNumber) + "_2");
        return btn;
    }
    else if (columnId == 5)
    {
        auto* btn = dynamic_cast<StyledPlaylistButton*>(existingComponentToUpdate);
        if (btn == nullptr)
        {
            btn = new StyledPlaylistButton("DEL", Colours::red);
            btn->addListener(this);
        }
        btn->setComponentID(String(rowNumber) + "_DEL");
        return btn;
    }
    
    return nullptr;
}

// Cell clicked
void PlaylistComponent::cellClicked(int rowNumber, int columnId, const MouseEvent&)
{
    DBG("Cell clicked - Row: " << rowNumber << ", Column: " << columnId);
}

var PlaylistComponent::getDragSourceDescription(const SparseSet<int>& rowsToDescribe)
{
    StringArray files;
    
    for (int i = 0; i < rowsToDescribe.size(); ++i)
    {
        int row = rowsToDescribe[i];
        if (row < static_cast<int>(filteredIndices.size()))
        {
            int libraryIndex = filteredIndices[row];
            files.add(trackLibrary[libraryIndex].filePath);
        }
    }
    
    return files.joinIntoString("\n"); // Return paths separated by newline
}

void PlaylistComponent::buttonClicked(Button* button)
{
    if (button == &addFilesButton)
    {
        addFilesToLibrary();
        return;
    }
    
    if (button == &clearButton)
    {
        trackLibrary.clear();
        updateFilteredTracks();
        tableComponent.updateContent();
        saveLibraryToFile();
        return;
    }
    
    // Handle deck load buttons
    String componentId = button->getComponentID();
    
    if (componentId.contains("_"))
    {
        int underscorePos = componentId.indexOf("_");
        int rowNumber = componentId.substring(0, underscorePos).getIntValue();
        int deckNumber = componentId.substring(underscorePos + 1).getIntValue();
        
        if (rowNumber < static_cast<int>(filteredIndices.size()))
        {
            int libraryIndex = filteredIndices[rowNumber];
            const auto& track = trackLibrary[libraryIndex];
            
            if (componentId.endsWith("_DEL"))
            {
                // Delete track from library
                trackLibrary.erase(trackLibrary.begin() + libraryIndex);
                updateFilteredTracks();
                tableComponent.updateContent();
                saveLibraryToFile();
                DBG("Removed track: " << track.title);
                return;
            }

            File audioFile(track.filePath);
            
            if (audioFile.existsAsFile())
            {
                if (deckNumber == 1 && deckGUI1 != nullptr)
                {
                    deckGUI1->loadFile(URL{audioFile});
                    DBG("Loaded '" << track.title << "' into Deck 1");
                }
                else if (deckNumber == 2 && deckGUI2 != nullptr)
                {
                    deckGUI2->loadFile(URL{audioFile});
                    DBG("Loaded '" << track.title << "' into Deck 2");
                }
            }
            else
            {
                DBG("File not found: " << track.filePath);
                // Remove missing file from library
                trackLibrary.erase(trackLibrary.begin() + libraryIndex);
                updateFilteredTracks();
                tableComponent.updateContent();
                saveLibraryToFile();
            }
        }
    }
}