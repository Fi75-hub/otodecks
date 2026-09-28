#pragma once

#include <JuceHeader.h>
#include "DeckGUI.h"
#include <vector>
#include <string>

class StyledPlaylistButton : public TextButton
{
public:
    StyledPlaylistButton(const String& name, Colour accent);
    void paintButton(Graphics&, bool, bool) override;
private:
    Colour accentColor;
};

// Custom LookAndFeel for centered table headers
class CenteredHeaderLookAndFeel : public LookAndFeel_V4
{
public:
    void drawTableHeaderColumn(Graphics& g, TableHeaderComponent& header,
                               const String& columnName, int columnId,
                               int width, int height, bool isMouseOver,
                               bool isMouseDown, int columnFlags) override
    {
        // Draw background
        auto bgColour = header.findColour(TableHeaderComponent::backgroundColourId);
        g.setColour(isMouseDown ? bgColour.darker() : (isMouseOver ? bgColour.brighter(0.1f) : bgColour));
        g.fillRect(0, 0, width, height);
        
        // Draw right border
        g.setColour(header.findColour(TableHeaderComponent::outlineColourId));
        g.drawLine((float)width - 1, 0, (float)width - 1, (float)height, 1.0f);
        
        // Draw bottom border
        g.drawLine(0, (float)height - 1, (float)width, (float)height - 1, 1.0f);
        
        // Draw centered text
        g.setColour(header.findColour(TableHeaderComponent::textColourId));
        g.setFont(Font(12.0f, Font::bold));
        g.drawText(columnName, 4, 0, width - 8, height, Justification::centred, true);
    }
};

class PlaylistComponent : public Component,
                          public TableListBoxModel,
                          public Button::Listener,
                          public TextEditor::Listener,
                          public DragAndDropContainer
{
public:
    PlaylistComponent(DeckGUI* deckGUI1, DeckGUI* deckGUI2);
    ~PlaylistComponent() override;

    // Component overrides
    void paint(Graphics&) override;
    void resized() override;

    // TableListBoxModel overrides
    int getNumRows() override;
    void paintRowBackground(Graphics&,
                            int rowNumber,
                            int width,
                            int height,
                            bool rowIsSelected) override;
                            
    void paintCell(Graphics&,
                   int rowNumber,
                   int columnId,
                   int width,
                   int height,
                   bool rowIsSelected) override;
    Component* refreshComponentForCell(int rowNumber,
                                       int columnId,
                                       bool isRowSelected,
                                       Component* existingComponentToUpdate) override;
    void cellClicked(int rowNumber, int columnId, const MouseEvent&) override;

    // Drag and Drop source (Drag file from playlist)
    var getDragSourceDescription(const SparseSet<int>& rowsToDescribe) override;

    // Button::Listener override
    void buttonClicked(Button* button) override;
    
    // TextEditor::Listener override for search functionality
    void textEditorTextChanged(TextEditor&) override;

private:
    // Track information structure
    struct TrackInfo
    {
        String title;
        String filePath;
        double duration; // Duration in seconds
    };
    
    // Library persistence methods
    void saveLibraryToFile();
    void loadLibraryFromFile();
    File getLibraryFile();
    
    // File management methods
    void addFilesToLibrary();  // Opens file chooser to add files
    double getAudioFileDuration(const File& file); // Get duration of audio file
    String formatDuration(double seconds); // Format seconds as MM:SS
    
    // Search and filter
    void updateFilteredTracks(); // Update displayed tracks based on search
    
    // UI Components
    TableListBox tableComponent;
    TextButton addFilesButton{"+ Add Files"};
    TextButton clearButton{"Clear All"};
    TextEditor searchBox;
    
    // Data storage
    std::vector<TrackInfo> trackLibrary; // Complete library
    std::vector<int> filteredIndices; // Indices of tracks matching search
    
    // Audio format manager for getting file info
    AudioFormatManager formatManager;
    
    // File chooser for adding files
    std::unique_ptr<FileChooser> fileChooser;

    // References to the deck components
    DeckGUI* deckGUI1;
    DeckGUI* deckGUI2;
    
    // Theme colors
    Colour deck1Color = Colour(0, 200, 255);   
    Colour deck2Color = Colour(255, 100, 150); 
    
    // Custom look and feel for centered headers
    CenteredHeaderLookAndFeel headerLookAndFeel;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PlaylistComponent)
};